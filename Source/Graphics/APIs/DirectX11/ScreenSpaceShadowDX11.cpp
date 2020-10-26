#include "stdafx.h"
#include "ScreenSpaceShadowDX11.h"

#include "CommonLib/FileUtil.h"

#include "UtilDX11.h"
#include "DeviceDX11.h"
#include "RenderTargetDX11.h"
#include "DepthStencilDX11.h"

namespace est
{
	namespace graphics
	{
		namespace dx11
		{
			namespace shader
			{
				struct ScreenSpaceShadowContents
				{
					float ScreenSpaceShadowWidth{ 1.f };

					math::float3 padding;
				};

				enum CBSlot
				{
					eCB_ScreenSpaceShadowContents = 0,
				};

				enum SamplerSlot
				{
					eSampler_LinearPointClamp = 0,
					eSampler_PointClamp = 1,
				};

				enum SRVSlot
				{
					eSRV_Source = 0,
					eSRV_Depth = 1,
				};

				enum PSType
				{
					ePS_Count,
				};

				const char* GetScreenSpaceShadowPSTypeToString(PSType emPSType)
				{
					switch (emPSType)
					{
					default:
						throw_line("unknown ps type");
						break;
					}
				}

				void SetScreenSpaceShadowContents(ID3D11DeviceContext* pDeviceContext, ConstantBuffer<ScreenSpaceShadowContents>* pCB_ScreenSpaceShadowContents, float ScreenSpaceShadowWidth)
				{
					ScreenSpaceShadowContents* pScreenSpaceShadowContents = pCB_ScreenSpaceShadowContents->Map(pDeviceContext);
					pScreenSpaceShadowContents->ScreenSpaceShadowWidth = ScreenSpaceShadowWidth;

					pScreenSpaceShadowContents->padding = {};

					pCB_ScreenSpaceShadowContents->Unmap(pDeviceContext);
				}
			}

			class ScreenSpaceShadow::Impl
			{
			public:
				Impl();
				~Impl();

			public:
				void Apply(const RenderTarget* pSource, const DepthStencil* pDepth, RenderTarget* pResult);

			private:
				ID3D11VertexShader* m_pVertexShader{ nullptr };
				std::array<ID3D11PixelShader*, shader::ePS_Count> m_pPixelShaders{ nullptr };

				ConstantBuffer<shader::ScreenSpaceShadowContents> m_screenSpaceShadowContents;
			};

			ScreenSpaceShadow::Impl::Impl()
			{
				ID3D11Device* pDevice = Device::GetInstance()->GetInterface();

				std::wstring shaderPath = file::GetEngineDataPath();
				shaderPath.append(L"Fx\\PostProcessing\\ScreenSpaceShadow\\ScreenSpaceShadow.hlsl");

				ID3DBlob* pShaderBlob{ nullptr };
				if (FAILED(D3DReadFileToBlob(shaderPath.c_str(), &pShaderBlob)))
				{
					throw_line("failed to read shader file : ScreenSpaceShadow.hlsl");
				}

				const D3D_SHADER_MACRO macros[] =
				{
					{ "DX11", "1" },
					{ nullptr, nullptr },
				};

				if (util::CreateVertexShader(pDevice, pShaderBlob, macros, shaderPath.c_str(), "ScreenSpaceShadowSBlurVS", shader::VS_CompileVersion, &m_pVertexShader, "ScreenSpaceShadow_VS") == false)
				{
					throw_line("failed to create ScreenSpaceShadow_VS");
				}

				for (int i = 0; i < shader::ePS_Count; ++i)
				{
					shader::PSType emPSType = static_cast<shader::PSType>(i);

					const char* psName = shader::GetScreenSpaceShadowPSTypeToString(emPSType);
					if (util::CreatePixelShader(pDevice, pShaderBlob, macros, shaderPath.c_str(), psName, shader::PS_CompileVersion, &m_pPixelShaders[i], psName) == false)
					{
						std::string dump = string::Format("failed to create %s", psName);
						throw_line(dump.c_str());
					}
				}

				SafeRelease(pShaderBlob);

				m_screenSpaceShadowContents.Create(pDevice, "ScreenSpaceShadowContents");
			}

			ScreenSpaceShadow::Impl::~Impl()
			{
				m_screenSpaceShadowContents.Destroy();

				SafeRelease(m_pVertexShader);

				for (int i = 0; i < shader::ePS_Count; ++i)
				{
					SafeRelease(m_pPixelShaders[i]);
				}
			}

			void ScreenSpaceShadow::Impl::Apply(const RenderTarget* pSource, const DepthStencil* pDepth, RenderTarget* pResult)
			{
				if (pSource == nullptr || pResult == nullptr)
					return;

				DX_PROFILING(ScreenSpaceShadow);

				Device* pDeviceInstance = Device::GetInstance();
				ID3D11DeviceContext* pDeviceContext = pDeviceInstance->GetImmediateContext();

				pDeviceContext->ClearState();

				pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

				ID3D11RasterizerState* pRasterizerState = pDeviceInstance->GetRasterizerState(EmRasterizerState::eSolidCCW);
				pDeviceContext->RSSetState(pRasterizerState);

				ID3D11BlendState* pBlendState = pDeviceInstance->GetBlendState(EmBlendState::eOff);
				pDeviceContext->OMSetBlendState(pBlendState, &math::float4::Zero.x, 0xffffffff);

				ID3D11DepthStencilState* pDepthStencilState = pDeviceInstance->GetDepthStencilState(EmDepthStencilState::eRead_Write_Off);
				pDeviceContext->OMSetDepthStencilState(pDepthStencilState, 0);

				ID3D11SamplerState* pSamplerLinearPointClamp = pDeviceInstance->GetSamplerState(EmSamplerState::eMinMagLinearMipPointClamp);
				pDeviceContext->PSSetSamplers(shader::eSampler_LinearPointClamp, 1, &pSamplerLinearPointClamp);

				ID3D11SamplerState* pSamplerPointClamp = pDeviceInstance->GetSamplerState(EmSamplerState::eMinMagMipPointClamp);
				pDeviceContext->PSSetSamplers(shader::eSampler_PointClamp, 1, &pSamplerPointClamp);

				pDeviceContext->VSSetShader(m_pVertexShader, nullptr, 0);

				ID3D11RenderTargetView* pRTV[] = { pResult->GetRenderTargetView() };
				pDeviceContext->OMSetRenderTargets(_countof(pRTV), pRTV, nullptr);

				D3D11_TEXTURE2D_DESC desc{};
				pResult->GetDesc2D(&desc);

				D3D11_VIEWPORT viewport{};
				viewport.Width = static_cast<float>(desc.Width);
				viewport.Height = static_cast<float>(desc.Height);
				viewport.MinDepth = 0.f;
				viewport.MaxDepth = 1.f;
				pDeviceContext->RSSetViewports(1, &viewport);

				ID3D11ShaderResourceView* pSourceSRV = pSource->GetShaderResourceView();
				pDeviceContext->PSSetShaderResources(shader::eSRV_Source, 1, &pSourceSRV);

				ID3D11ShaderResourceView* pDepthSRV = pDepth->GetShaderResourceView();
				pDeviceContext->PSSetShaderResources(shader::eSRV_Depth, 1, &pDepthSRV);

				//const Options& options = GetOptions();
				//const Options::ScreenSpaceShadowConfig& config = options.ScreenSpaceShadowConfig;
				//
				//shader::SetScreenSpaceShadowContents(pDeviceContext, &m_screenSpaceShadowContents, config.width);
				//pDeviceContext->PSSetConstantBuffers(shader::eCB_ScreenSpaceShadowContents, 1, &m_screenSpaceShadowContents.pBuffer);
				//
				//pDeviceContext->PSSetShader(m_pPixelShaders[shader::eHorizontal], nullptr, 0);
				//pDeviceContext->Draw(4, 0);
				//
				//pDeviceContext->PSSetShader(m_pPixelShaders[shader::eVertical], nullptr, 0);
				//pDeviceContext->Draw(4, 0);
			}

			ScreenSpaceShadow::ScreenSpaceShadow()
				: m_pImpl{ std::make_unique<Impl>() }
			{
			}

			ScreenSpaceShadow::~ScreenSpaceShadow()
			{
			}

			void ScreenSpaceShadow::Apply(const RenderTarget* pSource, const DepthStencil* pDepth, RenderTarget* pResult)
			{
				m_pImpl->Apply(pSource, pDepth, pResult);
			}
		}
	}
}