#include "stdafx.h"
#include "DownScaleDX11.h"

#include "CommonLib/FileUtil.h"

#include "UtilDX11.h"
#include "DeviceDX11.h"
#include "RenderTargetDX11.h"

namespace eastengine
{
	namespace graphics
	{
		namespace dx11
		{
			namespace shader
			{
				struct DownScaleContents
				{
					math::Vector2 f2SourceDimensions;
					math::Vector2 padding;
				};

				enum CBSlot
				{
					eCB_DownScaleContents = 0,
				};

				enum SamplerSlot
				{
					eSampler_Point = 0,
					eSampler_Linear = 1,
				};

				enum SRVSlot
				{
					eSRV_Color = 0,
					eSRV_Depth,
				};

				enum PSType
				{
					ePS_Downscale4 = 0,
					ePS_Downscale4Luminance,
					ePS_DownscaleHW,

					ePS_Count,
				};

				const char* GetDownScalePSTypeString(PSType emPSType)
				{
					switch (emPSType)
					{
					case ePS_Downscale4:
						return "DownscalePS";
					case ePS_Downscale4Luminance:
						return "DownscaleLuminancePS";
					case ePS_DownscaleHW:
						return "HWScalePS";
					default:
						throw_line("unknown ps type");
						break;
					}
				}

				void SetDownScaleContents(ID3D11DeviceContext* pDeviceContext, ConstantBuffer<DownScaleContents>* pCB_DownScaleContents, const math::Vector2& f2SourceDimensions)
				{
					DownScaleContents* pDownScaleContents = pCB_DownScaleContents->Map(pDeviceContext);

					pDownScaleContents->f2SourceDimensions = f2SourceDimensions;
					pDownScaleContents->padding = {};

					pCB_DownScaleContents->Unmap(pDeviceContext);
				}
			}

			class DownScale::Impl
			{
			public:
				Impl();
				~Impl();

			public:
				void Apply4SW(const RenderTarget* pSource, RenderTarget* pResult, bool isLuminance = false);
				void Apply16SW(const RenderTarget* pSource, RenderTarget* pResult);

				void ApplyHW(const RenderTarget* pSource, RenderTarget* pResult);
				void Apply16HW(const RenderTarget* pSource, RenderTarget* pResult);

			private:
				void SetRenderState(Device* pDeviceInstance, ID3D11DeviceContext* pDeviceContext);
				void ApplyDownScale(ID3D11DeviceContext* pDeviceContext, ID3D11PixelShader* pPixelShader, const RenderTarget* pSource, RenderTarget* pResult);

			private:
				ID3D11VertexShader* m_pVertexShader{ nullptr };
				std::array<ID3D11PixelShader*, shader::ePS_Count> m_pPixelShaders;

				ConstantBuffer<shader::DownScaleContents> m_downScaleContents;
			};

			DownScale::Impl::Impl()
			{
				ID3D11Device* pDevice = Device::GetInstance()->GetInterface();

				std::string strShaderPath = file::GetPath(file::eFx);
				strShaderPath.append("PostProcessing\\DownScale\\DownScale.hlsl");

				ID3DBlob* pShaderBlob{ nullptr };
				if (FAILED(D3DReadFileToBlob(string::MultiToWide(strShaderPath).c_str(), &pShaderBlob)))
				{
					throw_line("failed to read shader file : DownScale.hlsl");
				}

				{
					const D3D_SHADER_MACRO macros[] =
					{
						{ "DX11", "1" },
						{ nullptr, nullptr },
					};

					if (util::CreateVertexShader(pDevice, pShaderBlob, macros, strShaderPath.c_str(), "VS", shader::VS_CompileVersion, &m_pVertexShader, "DownScaleVS") == false)
					{
						throw_line("failed to create DownScaleVS");
					}

					if (util::CreatePixelShader(pDevice, pShaderBlob, macros, strShaderPath.c_str(), GetDownScalePSTypeString(shader::ePS_Downscale4), shader::PS_CompileVersion, &m_pPixelShaders[shader::ePS_Downscale4], GetDownScalePSTypeString(shader::ePS_Downscale4)) == false)
					{
						throw_line("failed to create DownscalePS");
					}

					if (util::CreatePixelShader(pDevice, pShaderBlob, macros, strShaderPath.c_str(), GetDownScalePSTypeString(shader::ePS_DownscaleHW), shader::PS_CompileVersion, &m_pPixelShaders[shader::ePS_DownscaleHW], GetDownScalePSTypeString(shader::ePS_DownscaleHW)) == false)
					{
						throw_line("failed to create HWScalePS");
					}

					if (util::CreatePixelShader(pDevice, pShaderBlob, macros, strShaderPath.c_str(), GetDownScalePSTypeString(shader::ePS_Downscale4Luminance), shader::PS_CompileVersion, &m_pPixelShaders[shader::ePS_Downscale4Luminance], GetDownScalePSTypeString(shader::ePS_Downscale4Luminance)) == false)
					{
						throw_line("failed to create DownscalePS_Luminance");
					}
				}

				SafeRelease(pShaderBlob);

				m_downScaleContents.Create(pDevice, "DownScaleContents");
			}

			DownScale::Impl::~Impl()
			{
				m_downScaleContents.Destroy();

				SafeRelease(m_pVertexShader);
				for (auto& pPixelShader : m_pPixelShaders)
				{
					SafeRelease(pPixelShader);
				}
			}

			void DownScale::Impl::Apply4SW(const RenderTarget* pSource, RenderTarget* pResult, bool isLuminance)
			{
				if (pSource == nullptr || pResult == nullptr)
					return;

				DX_PROFILING(DownScale4SW);

				Device* pDeviceInstance = Device::GetInstance();
				ID3D11DeviceContext* pDeviceContext = pDeviceInstance->GetImmediateContext();

				SetRenderState(pDeviceInstance, pDeviceContext);

				ID3D11PixelShader* pPixelShader = nullptr;
				if (isLuminance == true)
				{
					pPixelShader = m_pPixelShaders[shader::ePS_Downscale4Luminance];
				}
				else
				{
					pPixelShader = m_pPixelShaders[shader::ePS_Downscale4];
				}

				ApplyDownScale(pDeviceContext, pPixelShader, pSource, pResult);
			}

			void DownScale::Impl::Apply16SW(const RenderTarget* pSource, RenderTarget* pResult)
			{
				if (pSource == nullptr || pResult == nullptr)
					return;

				DX_PROFILING(DownScale16SW);

				Device* pDeviceInstance = Device::GetInstance();
				ID3D11DeviceContext* pDeviceContext = pDeviceInstance->GetImmediateContext();

				SetRenderState(pDeviceInstance, pDeviceContext);

				D3D11_TEXTURE2D_DESC desc{};
				pSource->GetDesc2D(&desc);

				desc.Width /= 4;
				desc.Height /= 4;

				RenderTarget* pDownscale = pDeviceInstance->GetRenderTarget(&desc, false);

				ApplyDownScale(pDeviceContext, m_pPixelShaders[shader::ePS_Downscale4], pSource, pDownscale);
				ApplyDownScale(pDeviceContext, m_pPixelShaders[shader::ePS_Downscale4], pDownscale, pResult);

				pDeviceInstance->ReleaseRenderTargets(&pDownscale);
			}

			void DownScale::Impl::ApplyHW(const RenderTarget* pSource, RenderTarget* pResult)
			{
				if (pSource == nullptr || pResult == nullptr)
					return;

				DX_PROFILING(DownScaleHW);

				Device* pDeviceInstance = Device::GetInstance();
				ID3D11DeviceContext* pDeviceContext = pDeviceInstance->GetImmediateContext();

				SetRenderState(pDeviceInstance, pDeviceContext);

				ApplyDownScale(pDeviceContext, m_pPixelShaders[shader::ePS_DownscaleHW], pSource, pResult);
			}

			void DownScale::Impl::Apply16HW(const RenderTarget* pSource, RenderTarget* pResult)
			{
				if (pSource == nullptr || pResult == nullptr)
					return;

				DX_PROFILING(DownScale16HW);

				Device* pDeviceInstance = Device::GetInstance();
				ID3D11DeviceContext* pDeviceContext = pDeviceInstance->GetImmediateContext();

				SetRenderState(pDeviceInstance, pDeviceContext);

				D3D11_TEXTURE2D_DESC desc{};
				pSource->GetDesc2D(&desc);

				// 2
				desc.Width /= 2;
				desc.Height /= 2;

				RenderTarget* pDownscale1 = pDeviceInstance->GetRenderTarget(&desc, false);
				ApplyDownScale(pDeviceContext, m_pPixelShaders[shader::ePS_DownscaleHW], pSource, pDownscale1);

				// 4
				desc.Width /= 2;
				desc.Height /= 2;

				RenderTarget* pDownscale2 = pDeviceInstance->GetRenderTarget(&desc, false);
				ApplyDownScale(pDeviceContext, m_pPixelShaders[shader::ePS_DownscaleHW], pDownscale1, pDownscale2);
				pDeviceInstance->ReleaseRenderTargets(&pDownscale1);

				// 8
				desc.Width /= 2;
				desc.Height /= 2;

				RenderTarget* pDownscale3 = pDeviceInstance->GetRenderTarget(&desc, false);
				ApplyDownScale(pDeviceContext, m_pPixelShaders[shader::ePS_DownscaleHW], pDownscale2, pDownscale3);
				pDeviceInstance->ReleaseRenderTargets(&pDownscale2);

				// 16
				ApplyDownScale(pDeviceContext, m_pPixelShaders[shader::ePS_DownscaleHW], pDownscale3, pResult);
				pDeviceInstance->ReleaseRenderTargets(&pDownscale3);
			}

			void DownScale::Impl::SetRenderState(Device* pDeviceInstance, ID3D11DeviceContext* pDeviceContext)
			{
				pDeviceContext->ClearState();

				pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

				ID3D11RasterizerState* pRasterizerState = pDeviceInstance->GetRasterizerState(EmRasterizerState::eSolidCCW);
				pDeviceContext->RSSetState(pRasterizerState);

				ID3D11BlendState* pBlendState = pDeviceInstance->GetBlendState(EmBlendState::eOff);
				pDeviceContext->OMSetBlendState(pBlendState, &math::Vector4::Zero.x, 0xffffffff);

				ID3D11DepthStencilState* pDepthStencilState = pDeviceInstance->GetDepthStencilState(EmDepthStencilState::eRead_Write_Off);
				pDeviceContext->OMSetDepthStencilState(pDepthStencilState, 0);

				ID3D11SamplerState* pSamplerPoint = pDeviceInstance->GetSamplerState(EmSamplerState::eMinMagMipPointWrap);
				pDeviceContext->PSSetSamplers(shader::eSampler_Point, 1, &pSamplerPoint);

				ID3D11SamplerState* pSamplerLinear = pDeviceInstance->GetSamplerState(EmSamplerState::eMinMagMipLinearWrap);
				pDeviceContext->PSSetSamplers(shader::eSampler_Linear, 1, &pSamplerLinear);
				
				pDeviceContext->VSSetShader(m_pVertexShader, nullptr, 0);
			}

			void DownScale::Impl::ApplyDownScale(ID3D11DeviceContext* pDeviceContext, ID3D11PixelShader* pPixelShader, const RenderTarget* pSource, RenderTarget* pResult)
			{
				pDeviceContext->PSSetShader(pPixelShader, nullptr, 0);

				ID3D11RenderTargetView* pRTV[] = { pResult->GetRenderTargetView() };
				pDeviceContext->OMSetRenderTargets(_countof(pRTV), pRTV, nullptr);

				D3D11_TEXTURE2D_DESC desc_source{};
				pSource->GetDesc2D(&desc_source);

				shader::SetDownScaleContents(pDeviceContext, &m_downScaleContents, math::Vector2(static_cast<float>(desc_source.Width), static_cast<float>(desc_source.Height)));
				pDeviceContext->PSSetConstantBuffers(shader::eCB_DownScaleContents, 1, &m_downScaleContents.pBuffer);

				D3D11_TEXTURE2D_DESC desc{};
				pResult->GetDesc2D(&desc);

				D3D11_VIEWPORT viewport{};
				viewport.Width = static_cast<float>(desc.Width);
				viewport.Height = static_cast<float>(desc.Height);
				viewport.MinDepth = 0.f;
				viewport.MaxDepth = 1.f;
				pDeviceContext->RSSetViewports(1, &viewport);

				ID3D11ShaderResourceView* pSRV = pSource->GetShaderResourceView();
				pDeviceContext->PSSetShaderResources(shader::eSRV_Color, 1, &pSRV);

				pDeviceContext->Draw(4, 0);
			}

			DownScale::DownScale()
				: m_pImpl{ std::make_unique<Impl>() }
			{
			}

			DownScale::~DownScale()
			{
			}

			void DownScale::Apply4SW(const RenderTarget* pSource, RenderTarget* pResult, bool isLuminance)
			{
				m_pImpl->Apply4SW(pSource, pResult, isLuminance);
			}

			void DownScale::Apply16SW(const RenderTarget* pSource, RenderTarget* pResult)
			{
				m_pImpl->Apply16SW(pSource, pResult);
			}

			void DownScale::ApplyHW(const RenderTarget* pSource, RenderTarget* pResult)
			{
				m_pImpl->ApplyHW(pSource, pResult);
			}

			void DownScale::Apply16HW(const RenderTarget* pSource, RenderTarget* pResult)
			{
				m_pImpl->Apply16HW(pSource, pResult);
			}
		}
	}
}