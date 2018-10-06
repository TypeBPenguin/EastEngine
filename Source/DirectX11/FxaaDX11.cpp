#include "stdafx.h"
#include "FxaaDX11.h"

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
				struct FxaaContents
				{
					math::Vector4 f4RcpFrame;
				};

				enum CBSlot
				{
					eCB_FxaaContents = 0,
				};

				enum SamplerSlot
				{
					eSampler_Anisotropic = 0,
				};

				enum SRVSlot
				{
					eSRV_Source = 0,
				};

				void SetFxaaContents(ID3D11DeviceContext* pDeviceContext, ConstantBuffer<FxaaContents>* pCB_FxaaContents, const math::UInt2& n2ScreenSize)
				{
					FxaaContents* pFxaaContents = pCB_FxaaContents->Map(pDeviceContext);

					const math::Vector4 vRcpFrame(1.f / static_cast<float>(n2ScreenSize.x), 1.f / static_cast<float>(n2ScreenSize.y), 0.f, 0.f);
					pFxaaContents->f4RcpFrame = vRcpFrame;

					pCB_FxaaContents->Unmap(pDeviceContext);
				}
			}

			class Fxaa::Impl
			{
			public:
				Impl();
				~Impl();

			public:
				void Apply(const RenderTarget* pSource, RenderTarget* pResult);

			private:
				ID3D11VertexShader* m_pVertexShader{ nullptr };
				ID3D11PixelShader* m_pPixelShader{ nullptr };

				ConstantBuffer<shader::FxaaContents> m_fxaaContents;
			};

			Fxaa::Impl::Impl()
			{
				ID3D11Device* pDevice = Device::GetInstance()->GetInterface();

				std::string strShaderPath = file::GetPath(file::eFx);
				strShaderPath.append("PostProcessing\\FXAA\\FXAA.hlsl");

				ID3DBlob* pShaderBlob{ nullptr };
				if (FAILED(D3DReadFileToBlob(String::MultiToWide(strShaderPath).c_str(), &pShaderBlob)))
				{
					throw_line("failed to read shader file : FXAA.hlsl");
				}

				const D3D_SHADER_MACRO macros[] =
				{
					{ "DX11", "1" },
					{ nullptr, nullptr },
				};

				if (util::CreateVertexShader(pDevice, pShaderBlob, macros, strShaderPath.c_str(), "VS", "vs_5_0", &m_pVertexShader, "FxaaVS") == false)
				{
					throw_line("failed to create FxaaVS");
				}

				if (util::CreatePixelShader(pDevice, pShaderBlob, macros, strShaderPath.c_str(), "PS", "ps_5_0", &m_pPixelShader, "FxaaPS") == false)
				{
					throw_line("failed to create FxaaPS");
				}

				SafeRelease(pShaderBlob);

				m_fxaaContents.Create(pDevice, "FxaaContents");
			}

			Fxaa::Impl::~Impl()
			{
				m_fxaaContents.Destroy();

				SafeRelease(m_pVertexShader);
				SafeRelease(m_pPixelShader);
			}

			void Fxaa::Impl::Apply(const RenderTarget* pSource, RenderTarget* pResult)
			{
				if (pResult == nullptr || pSource == nullptr)
					return;

				DX_PROFILING(FXAA);

				Device* pDeviceInstance = Device::GetInstance();
				ID3D11DeviceContext* pDeviceContext = pDeviceInstance->GetImmediateContext();
				const math::UInt2& n2ScreenSize = pDeviceInstance->GetScreenSize();

				pDeviceContext->ClearState();

				const D3D11_VIEWPORT* pViewport = pDeviceInstance->GetViewport();
				pDeviceContext->RSSetViewports(1, pViewport);

				ID3D11RenderTargetView* pRTV[] = { pResult->GetRenderTargetView() };
				pDeviceContext->OMSetRenderTargets(_countof(pRTV), pRTV, nullptr);

				pDeviceContext->VSSetShader(m_pVertexShader, nullptr, 0);
				pDeviceContext->PSSetShader(m_pPixelShader, nullptr, 0);

				pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

				ID3D11RasterizerState* pRasterizerState = pDeviceInstance->GetRasterizerState(EmRasterizerState::eSolidCCW);
				pDeviceContext->RSSetState(pRasterizerState);

				ID3D11BlendState* pBlendState = pDeviceInstance->GetBlendState(EmBlendState::eOff);
				pDeviceContext->OMSetBlendState(pBlendState, &math::Vector4::Zero.x, 0xffffffff);

				ID3D11DepthStencilState* pDepthStencilState = pDeviceInstance->GetDepthStencilState(EmDepthStencilState::eRead_Write_Off);
				pDeviceContext->OMSetDepthStencilState(pDepthStencilState, 0);

				ID3D11ShaderResourceView* pSRV = pSource->GetShaderResourceView();
				pDeviceContext->PSSetShaderResources(shader::eSRV_Source, 1, &pSRV);

				ID3D11SamplerState* pSamplerAnisotropic = pDeviceInstance->GetSamplerState(EmSamplerState::eAnisotropicWrap);
				pDeviceContext->PSSetSamplers(shader::eSampler_Anisotropic, 1, &pSamplerAnisotropic);

				shader::SetFxaaContents(pDeviceContext, &m_fxaaContents, n2ScreenSize);
				pDeviceContext->PSSetConstantBuffers(shader::eCB_FxaaContents, 1, &m_fxaaContents.pBuffer);

				pDeviceContext->Draw(4, 0);
			}

			Fxaa::Fxaa()
				: m_pImpl{ std::make_unique<Impl>() }
			{
			}

			Fxaa::~Fxaa()
			{
			}

			void Fxaa::Apply(const RenderTarget* pSource, RenderTarget* pResult)
			{
				m_pImpl->Apply(pSource, pResult);
			}
		}
	}
}