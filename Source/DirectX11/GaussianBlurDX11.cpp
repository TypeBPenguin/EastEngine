#include "stdafx.h"
#include "GaussianBlurDX11.h"

#include "CommonLib/FileUtil.h"

#include "UtilDX11.h"
#include "DeviceDX11.h"
#include "RenderTargetDX11.h"
#include "DepthStencilDX11.h"

namespace eastengine
{
	namespace graphics
	{
		namespace dx11
		{
			namespace shader
			{
				struct GaussianBlurContents
				{
					float fSigma{ 0.5f };
					math::Vector2 f2SourceDimensions;
					float padding{ 0.f };
				};

				enum CBSlot
				{
					eCB_GaussianBlurContents = 0,
				};

				enum SamplerSlot
				{
					eSampler_Point = 0,
				};

				enum SRVSlot
				{
					eSRV_Color = 0,
					eSRV_Depth,
				};

				enum PSType
				{
					ePS_GaussianBlurH = 0,
					ePS_GaussianBlurV,
					ePS_GaussianDepthBlurH,
					ePS_GaussianDepthBlurV,

					ePS_Count,
				};

				void SetGaussianBlurContents(ID3D11DeviceContext* pDeviceContext, ConstantBuffer<GaussianBlurContents>* pCB_GaussianBlurContents, float fSigma, const math::Vector2& f2SourceDimensions)
				{
					GaussianBlurContents* pGaussianBlurContents = pCB_GaussianBlurContents->Map(pDeviceContext);

					pGaussianBlurContents->fSigma = fSigma;
					pGaussianBlurContents->f2SourceDimensions = f2SourceDimensions;
					pGaussianBlurContents->padding = 0.f;

					pCB_GaussianBlurContents->Unmap(pDeviceContext);
				}
			}

			class GaussianBlur::Impl
			{
			public:
				Impl();
				~Impl();

			public:
				void Apply(const RenderTarget* pSource, RenderTarget* pResult, float fSigma);
				void Apply(const RenderTarget* pSource, const DepthStencil* pDepth, RenderTarget* pResult, float fSigma);

			private:
				void SetRenderState(Device* pDeviceInstance, ID3D11DeviceContext* pDeviceContext);
				void ApplyBlur(ID3D11DeviceContext* pDeviceContext, ID3D11PixelShader* pPixelShader, float fSigma, const RenderTarget* pSource, RenderTarget* pResult);

			private:
				ID3D11VertexShader* m_pVertexShader{ nullptr };
				std::array<ID3D11PixelShader*, shader::ePS_Count> m_pPixelShaders;

				ConstantBuffer<shader::GaussianBlurContents> m_gaussianBlurContents;
			};

			GaussianBlur::Impl::Impl()
			{
				ID3D11Device* pDevice = Device::GetInstance()->GetInterface();

				std::string strShaderPath = file::GetPath(file::eFx);
				strShaderPath.append("PostProcessing\\GaussianBlur\\GaussianBlur.hlsl");

				ID3DBlob* pShaderBlob{ nullptr };
				if (FAILED(D3DReadFileToBlob(String::MultiToWide(strShaderPath).c_str(), &pShaderBlob)))
				{
					throw_line("failed to read shader file : GaussianBlur.hlsl");
				}

				const D3D_SHADER_MACRO macros[] =
				{
					{ "DX11", "1" },
					{ nullptr, nullptr },
				};

				if (util::CreateVertexShader(pDevice, pShaderBlob, macros, strShaderPath.c_str(), "VS", "vs_5_0", &m_pVertexShader, "GaussianBlur_VS") == false)
				{
					throw_line("failed to create GaussianBlur_VS");
				}

				if (util::CreatePixelShader(pDevice, pShaderBlob, macros, strShaderPath.c_str(), "GaussianBlurH_PS", "ps_5_0", &m_pPixelShaders[shader::ePS_GaussianBlurH], "GaussianBlurH_PS") == false)
				{
					throw_line("failed to create GaussianBlurH_PS");
				}

				if (util::CreatePixelShader(pDevice, pShaderBlob, macros, strShaderPath.c_str(), "GaussianBlurV_PS", "ps_5_0", &m_pPixelShaders[shader::ePS_GaussianBlurV], "GaussianBlurV_PS") == false)
				{
					throw_line("failed to create GaussianBlurV_PS");
				}

				if (util::CreatePixelShader(pDevice, pShaderBlob, macros, strShaderPath.c_str(), "GaussianDepthBlurH_PS", "ps_5_0", &m_pPixelShaders[shader::ePS_GaussianDepthBlurH], "GaussianDepthBlurH_PS") == false)
				{
					throw_line("failed to create GaussianDepthBlurH_PS");
				}

				if (util::CreatePixelShader(pDevice, pShaderBlob, macros, strShaderPath.c_str(), "GaussianDepthBlurV_PS", "ps_5_0", &m_pPixelShaders[shader::ePS_GaussianDepthBlurV], "GaussianDepthBlurV_PS") == false)
				{
					throw_line("failed to create GaussianDepthBlurV_PS");
				}

				SafeRelease(pShaderBlob);

				m_gaussianBlurContents.Create(pDevice, "GaussianBlurContents");
			}

			GaussianBlur::Impl::~Impl()
			{
				m_gaussianBlurContents.Destroy();

				SafeRelease(m_pVertexShader);
				for (auto& pPixelShader : m_pPixelShaders)
				{
					SafeRelease(pPixelShader);
				}
			}

			void GaussianBlur::Impl::Apply(const RenderTarget* pSource, RenderTarget* pResult, float fSigma)
			{
				if (pSource == nullptr || pResult == nullptr)
					return;

				DX_PROFILING(GaussianBlur);

				Device* pDeviceInstance = Device::GetInstance();
				ID3D11DeviceContext* pDeviceContext = pDeviceInstance->GetImmediateContext();

				SetRenderState(pDeviceInstance, pDeviceContext);

				D3D11_TEXTURE2D_DESC desc{};
				pSource->GetDesc2D(&desc);

				RenderTarget* pGaussianBlur = pDeviceInstance->GetRenderTarget(&desc, false);
				ApplyBlur(pDeviceContext, m_pPixelShaders[shader::ePS_GaussianBlurH], fSigma, pSource, pGaussianBlur);
				ApplyBlur(pDeviceContext, m_pPixelShaders[shader::ePS_GaussianBlurV], fSigma, pGaussianBlur, pResult);

				pDeviceInstance->ReleaseRenderTargets(&pGaussianBlur);
			}

			void GaussianBlur::Impl::Apply(const RenderTarget* pSource, const DepthStencil* pDepth, RenderTarget* pResult, float fSigma)
			{
				if (pSource == nullptr || pDepth == nullptr || pResult == nullptr)
					return;

				DX_PROFILING(GaussianDepthBlur);

				Device* pDeviceInstance = Device::GetInstance();
				ID3D11DeviceContext* pDeviceContext = pDeviceInstance->GetImmediateContext();

				SetRenderState(pDeviceInstance, pDeviceContext);

				ID3D11ShaderResourceView* pSRV = pDepth->GetShaderResourceView();
				pDeviceContext->PSSetShaderResources(shader::eSRV_Depth, 1, &pSRV);

				D3D11_TEXTURE2D_DESC desc{};
				pSource->GetDesc2D(&desc);

				RenderTarget* pGaussianBlur = pDeviceInstance->GetRenderTarget(&desc, false);
				ApplyBlur(pDeviceContext, m_pPixelShaders[shader::ePS_GaussianDepthBlurH], fSigma, pSource, pGaussianBlur);
				ApplyBlur(pDeviceContext, m_pPixelShaders[shader::ePS_GaussianDepthBlurV], fSigma, pGaussianBlur, pResult);

				pDeviceInstance->ReleaseRenderTargets(&pGaussianBlur);
			}

			void GaussianBlur::Impl::SetRenderState(Device* pDeviceInstance, ID3D11DeviceContext* pDeviceContext)
			{
				pDeviceContext->ClearState();

				pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

				ID3D11RasterizerState* pRasterizerState = pDeviceInstance->GetRasterizerState(EmRasterizerState::eSolidCCW);
				pDeviceContext->RSSetState(pRasterizerState);

				ID3D11BlendState* pBlendState = pDeviceInstance->GetBlendState(EmBlendState::eOff);
				pDeviceContext->OMSetBlendState(pBlendState, &math::Vector4::Zero.x, 0xffffffff);

				ID3D11DepthStencilState* pDepthStencilState = pDeviceInstance->GetDepthStencilState(EmDepthStencilState::eRead_Write_Off);
				pDeviceContext->OMSetDepthStencilState(pDepthStencilState, 0);

				ID3D11SamplerState* pSampler = pDeviceInstance->GetSamplerState(EmSamplerState::eMinMagMipPointWrap);
				pDeviceContext->PSSetSamplers(shader::eSampler_Point, 1, &pSampler);

				pDeviceContext->VSSetShader(m_pVertexShader, nullptr, 0);
			}

			void GaussianBlur::Impl::ApplyBlur(ID3D11DeviceContext* pDeviceContext, ID3D11PixelShader* pPixelShader, float fSigma, const RenderTarget* pSource, RenderTarget* pResult)
			{
				pDeviceContext->PSSetShader(pPixelShader, nullptr, 0);

				ID3D11RenderTargetView* pRTV[] = { pResult->GetRenderTargetView() };
				pDeviceContext->OMSetRenderTargets(_countof(pRTV), pRTV, nullptr);

				D3D11_TEXTURE2D_DESC desc_source{};
				pSource->GetDesc2D(&desc_source);

				shader::SetGaussianBlurContents(pDeviceContext, &m_gaussianBlurContents, fSigma, math::Vector2(static_cast<float>(desc_source.Width), static_cast<float>(desc_source.Height)));
				pDeviceContext->PSSetConstantBuffers(shader::eCB_GaussianBlurContents, 1, &m_gaussianBlurContents.pBuffer);

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

			GaussianBlur::GaussianBlur()
				: m_pImpl{ std::make_unique<Impl>() }
			{
			}

			GaussianBlur::~GaussianBlur()
			{
			}

			void GaussianBlur::Apply(const RenderTarget* pSource, RenderTarget* pResult, float fSigma)
			{
				m_pImpl->Apply(pSource, pResult, fSigma);
			}

			void GaussianBlur::Apply(const RenderTarget* pSource, const DepthStencil* pDepth, RenderTarget* pResult, float fSigma)
			{
				m_pImpl->Apply(pSource, pDepth, pResult, fSigma);
			}
		}
	}
}