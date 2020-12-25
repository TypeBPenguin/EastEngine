#include "stdafx.h"
#include "ColorGradingDX11.h"

#include "CommonLib/FileUtil.h"

#include "Graphics/Interface/Camera.h"

#include "UtilDX11.h"
#include "DeviceDX11.h"
#include "RenderTargetDX11.h"

namespace est
{
	namespace graphics
	{
		namespace dx11
		{
			namespace shader
			{
				struct ColorGradingContents
				{
					math::float3 colorGuide{ 1.f, 0.5f, 0.f };
					float padding{ 0.f };
				};

				enum CBSlot
				{
					eCB_ColorGradingContents = 0,
				};

				enum SamplerSlot
				{
					eSampler_LinearWrap = 0,
				};

				enum SRVSlot
				{
					eSRV_Source = 0,
				};

				void SetColorGradingContents(ID3D11DeviceContext* pDeviceContext, ConstantBuffer<ColorGradingContents>* pCB_ColorGradingContents, const math::float3& colorGuide)
				{
					ColorGradingContents* pColorGradingContents = pCB_ColorGradingContents->Map(pDeviceContext);
					
					pColorGradingContents->colorGuide = colorGuide;
					pColorGradingContents->padding = {};

					pCB_ColorGradingContents->Unmap(pDeviceContext);
				}
			}

			class ColorGrading::Impl
			{
			public:
				Impl();
				~Impl();

			public:
				void Apply(Camera* pCamera, const RenderTarget* pSource, RenderTarget* pResult);

			private:
				ID3D11VertexShader* m_pVertexShader{ nullptr };
				ID3D11PixelShader* m_pPixelShader{ nullptr };

				ConstantBuffer<shader::ColorGradingContents> m_colorGradingContents;
			};

			ColorGrading::Impl::Impl()
			{
				ID3D11Device* pDevice = Device::GetInstance()->GetInterface();

				std::wstring shaderPath = file::GetEngineDataPath();
				shaderPath.append(L"Fx\\PostProcessing\\ColorGrading\\ColorGrading.hlsl");

				ID3DBlob* pShaderBlob{ nullptr };
				if (FAILED(D3DReadFileToBlob(shaderPath.c_str(), &pShaderBlob)))
				{
					throw_line("failed to read shader file : ColorGrading.hlsl");
				}

				const D3D_SHADER_MACRO macros[] =
				{
					{ "DX11", "1" },
					{ nullptr, nullptr },
				};

				if (util::CreateVertexShader(pDevice, pShaderBlob, macros, shaderPath.c_str(), "VS", shader::VS_CompileVersion, &m_pVertexShader, "ColorGrading_VS") == false)
				{
					throw_line("failed to create ColorGrading_VS");
				}

				if (util::CreatePixelShader(pDevice, pShaderBlob, macros, shaderPath.c_str(), "PS", shader::PS_CompileVersion, &m_pPixelShader, "ColorGrading_PS") == false)
				{
					throw_line("failed to create ColorGrading_PS");
				}

				SafeRelease(pShaderBlob);

				m_colorGradingContents.Create(pDevice, "ColorGradingContents");
			}

			ColorGrading::Impl::~Impl()
			{
				m_colorGradingContents.Destroy();

				SafeRelease(m_pVertexShader);
				SafeRelease(m_pPixelShader);
			}

			void ColorGrading::Impl::Apply(Camera* pCamera, const RenderTarget* pSource, RenderTarget* pResult)
			{
				if (pSource == nullptr || pResult == nullptr)
					return;

				DX_PROFILING(ColorGrading);

				Device* pDeviceInstance = Device::GetInstance();
				ID3D11DeviceContext* pDeviceContext = pDeviceInstance->GetRenderContext();

				pDeviceContext->ClearState();

				pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

				ID3D11RasterizerState* pRasterizerState = pDeviceInstance->GetRasterizerState(RasterizerState::eSolidCCW);
				pDeviceContext->RSSetState(pRasterizerState);

				ID3D11BlendState* pBlendState = pDeviceInstance->GetBlendState(BlendState::eOff);
				pDeviceContext->OMSetBlendState(pBlendState, &math::float4::Zero.x, 0xffffffff);

				ID3D11DepthStencilState* pDepthStencilState = pDeviceInstance->GetDepthStencilState(DepthStencilState::eRead_Write_Off);
				pDeviceContext->OMSetDepthStencilState(pDepthStencilState, 0);

				ID3D11SamplerState* pSamplerLinearWrap = pDeviceInstance->GetSamplerState(SamplerState::eMinMagMipLinearWrap);
				pDeviceContext->PSSetSamplers(shader::eSampler_LinearWrap, 1, &pSamplerLinearWrap);

				pDeviceContext->VSSetShader(m_pVertexShader, nullptr, 0);
				pDeviceContext->PSSetShader(m_pPixelShader, nullptr, 0);

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

				ID3D11ShaderResourceView* pSRV = pSource->GetShaderResourceView();
				pDeviceContext->PSSetShaderResources(shader::eSRV_Source, 1, &pSRV);

				const Options& options = RenderOptions();
				const Options::ColorGradingConfig& config = options.colorGradingConfig;

				shader::SetColorGradingContents(pDeviceContext, &m_colorGradingContents, config.colorGuide);
				pDeviceContext->PSSetConstantBuffers(shader::eCB_ColorGradingContents, 1, &m_colorGradingContents.pBuffer);

				pDeviceContext->Draw(4, 0);
			}

			ColorGrading::ColorGrading()
				: m_pImpl{ std::make_unique<Impl>() }
			{
			}

			ColorGrading::~ColorGrading()
			{
			}

			void ColorGrading::Apply(Camera* pCamera, const RenderTarget* pSource, RenderTarget* pResult)
			{
				m_pImpl->Apply(pCamera, pSource, pResult);
			}
		}
	}
}