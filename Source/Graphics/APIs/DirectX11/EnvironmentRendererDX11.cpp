#include "stdafx.h"
#include "EnvironmentRendererDX11.h"

#include "CommonLib/FileUtil.h"

#include "Graphics/Interface/Camera.h"

#include "UtilDX11.h"
#include "DeviceDX11.h"
#include "RenderTargetDX11.h"

#include "TextureDX11.h"
#include "VertexBufferDX11.h"
#include "IndexBufferDX11.h"

namespace est
{
	namespace graphics
	{
		namespace dx11
		{
			namespace shader
			{
				struct EnvironmentContents
				{
					math::Matrix matInvView;
					math::Matrix matProjection;
					float fTextureGamma{ 1.f };
					math::float3 padding;
				};

				enum CBSlot
				{
					eCB_EnvironmentContents = 0,
				};

				enum SamplerSlot
				{
					eSampler_Anisotropic = 0,
				};

				enum SRVSlot
				{
					eSRV_EnvironmentMap = 0,
				};

				void SetEnvironmentContents(ID3D11DeviceContext* pDeviceContext, ConstantBuffer<EnvironmentContents>* pCB_EnvironmentContents, Camera* pCamera, float fTextureGamma)
				{
					EnvironmentContents* pEnvironmentContents = pCB_EnvironmentContents->Map(pDeviceContext);
					pEnvironmentContents->matInvView = pCamera->GetViewMatrix().Invert().Transpose();
					pEnvironmentContents->matProjection = pCamera->GetProjectionMatrix().Transpose();
					pEnvironmentContents->fTextureGamma = fTextureGamma;
					pEnvironmentContents->padding = {};
					pCB_EnvironmentContents->Unmap(pDeviceContext);
				}
			}

			class EnvironmentRenderer::Impl
			{
			public:
				Impl();
				~Impl();

			public:
				void Render(const RenderElement& element);
				void Cleanup();

			private:
				ID3D11VertexShader* m_pVertexShader{ nullptr };
				ID3D11PixelShader* m_pPixelShader{ nullptr };
				ID3D11InputLayout* m_pInputLayout{ nullptr };

				ConstantBuffer<shader::EnvironmentContents> m_environmentContents;
			};

			EnvironmentRenderer::Impl::Impl()
			{
				ID3D11Device* pDevice = Device::GetInstance()->GetInterface();

				std::wstring shaderPath = file::GetEngineDataPath();
				shaderPath.append(L"Fx\\Environment\\Environment.hlsl");

				ID3DBlob* pShaderBlob{ nullptr };
				if (FAILED(D3DReadFileToBlob(shaderPath.c_str(), &pShaderBlob)))
				{
					throw_line("failed to read shader file : Environment.hlsl");
				}

				const D3D_SHADER_MACRO macros[] =
				{
					{ "DX11", "1" },
					{ nullptr, nullptr },
				};

				{
					const D3D11_INPUT_ELEMENT_DESC* pInputElements = nullptr;
					size_t nElementCount = 0;

					util::GetInputElementDesc(VertexPosTexNor::Format(), &pInputElements, &nElementCount);

					if (util::CreateVertexShader(pDevice, pShaderBlob, macros, shaderPath.c_str(), "VS", shader::VS_CompileVersion, &m_pVertexShader, pInputElements, nElementCount, &m_pInputLayout, "Environment_VS") == false)
					{
						throw_line("failed to create vertex shader");
					}
				}

				{
					if (util::CreatePixelShader(pDevice, pShaderBlob, macros, shaderPath.c_str(), "PS", shader::PS_CompileVersion, &m_pPixelShader, "Environment_PS") == false)
					{
						throw_line("failed to create pixel shader");
					}
				}

				SafeRelease(pShaderBlob);

				m_environmentContents.Create(pDevice, "EnvironmentContents");
			}

			EnvironmentRenderer::Impl::~Impl()
			{
				SafeRelease(m_pVertexShader);
				SafeRelease(m_pPixelShader);
				SafeRelease(m_pInputLayout);

				m_environmentContents.Destroy();
			}

			void EnvironmentRenderer::Impl::Render(const RenderElement& element)
			{
				TRACER_EVENT(__FUNCTIONW__);
				DX_PROFILING(EnvironmentRenderer);

				Device* pDeviceInstance = Device::GetInstance();

				const IImageBasedLight* pImageBasedLight = pDeviceInstance->GetImageBasedLight();
				Texture* pEnvironmentHDR = static_cast<Texture*>(pImageBasedLight->GetEnvironmentHDR().get());
				if (pEnvironmentHDR == nullptr)
					return;

				VertexBuffer* pVertexBuffer = static_cast<VertexBuffer*>(pImageBasedLight->GetEnvironmentSphereVB().get());
				IndexBuffer* pIndexBuffer = static_cast<IndexBuffer*>(pImageBasedLight->GetEnvironmentSphereIB().get());
				if (pVertexBuffer == nullptr || pIndexBuffer == nullptr)
					return;

				ID3D11DeviceContext* pDeviceContext = element.pDeviceContext;
				pDeviceContext->ClearState();

				const math::Viewport& viewport = pDeviceInstance->GetViewport();
				pDeviceContext->RSSetViewports(1, util::Convert(viewport));

				pDeviceContext->OMSetRenderTargets(element.rtvCount, element.pRTVs, element.pDSV);

				pDeviceContext->VSSetShader(m_pVertexShader, nullptr, 0);
				pDeviceContext->PSSetShader(m_pPixelShader, nullptr, 0);

				pDeviceContext->IASetInputLayout(m_pInputLayout);
				pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

				ID3D11RasterizerState* pRasterizerState = pDeviceInstance->GetRasterizerState(RasterizerState::eSolidCullNone);
				pDeviceContext->RSSetState(pRasterizerState);

				ID3D11BlendState* pBlendState = pDeviceInstance->GetBlendState(BlendState::eOff);
				pDeviceContext->OMSetBlendState(pBlendState, &math::float4::Zero.x, 0xffffffff);

				ID3D11DepthStencilState* pDepthStencilState = pDeviceInstance->GetDepthStencilState(DepthStencilState::eRead_Write_Off);
				pDeviceContext->OMSetDepthStencilState(pDepthStencilState, 0);

				ID3D11ShaderResourceView* pSRV = pEnvironmentHDR->GetShaderResourceView();
				pDeviceContext->PSSetShaderResources(shader::eSRV_EnvironmentMap, 1, &pSRV);

				shader::SetEnvironmentContents(pDeviceContext, &m_environmentContents, element.pCamera, 1.f);
				pDeviceContext->VSSetConstantBuffers(shader::eCB_EnvironmentContents, 1, &m_environmentContents.pBuffer);
				pDeviceContext->PSSetConstantBuffers(shader::eCB_EnvironmentContents, 1, &m_environmentContents.pBuffer);

				ID3D11SamplerState* pSamplers[] = { pDeviceInstance->GetSamplerState(SamplerState::eAnisotropicWrap) };
				pDeviceContext->PSSetSamplers(shader::eSampler_Anisotropic, _countof(pSamplers), pSamplers);

				ID3D11Buffer* pBuffers[] = { pVertexBuffer->GetBuffer(), };
				const uint32_t nStrides[] = { pVertexBuffer->GetFormatSize(), };
				const uint32_t nOffsets[] = { 0, };
				pDeviceContext->IASetVertexBuffers(0, _countof(pBuffers), pBuffers, nStrides, nOffsets);
				pDeviceContext->IASetIndexBuffer(pIndexBuffer->GetBuffer(), DXGI_FORMAT_R32_UINT, 0);
				pDeviceContext->DrawIndexed(pIndexBuffer->GetIndexCount(), 0, 0);
			}

			void EnvironmentRenderer::Impl::Cleanup()
			{
			}

			EnvironmentRenderer::EnvironmentRenderer()
				: m_pImpl{ std::make_unique<Impl>() }
			{
			}

			EnvironmentRenderer::~EnvironmentRenderer()
			{
			}

			void EnvironmentRenderer::Render(const RenderElement& element)
			{
				m_pImpl->Render(element);
			}

			void EnvironmentRenderer::Cleanup()
			{
				m_pImpl->Cleanup();
			}
		}
	}
}