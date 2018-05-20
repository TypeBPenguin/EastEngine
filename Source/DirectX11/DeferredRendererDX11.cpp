#include "stdafx.h"
#include "DeferredRendererDX11.h"

#include "CommonLib/FileUtil.h"

#include "GraphicsInterface/Camera.h"
#include "GraphicsInterface/LightManager.h"

#include "UtilDX11.h"
#include "DeviceDX11.h"
#include "GBufferDX11.h"

#include "TextureDX11.h"

namespace eastengine
{
	namespace graphics
	{
		namespace dx11
		{
			namespace deferredshader
			{
				enum SRVSlot
				{
					eSRV_Depth = 0,
					eSRV_Normal = 1,
					eSRV_AlbedoSpecular = 2,
					eSRV_DisneyBRDF = 3,

					eSRV_DiffuseHDR = 16,
					eSRV_SpecularHDR = 17,
					eSRV_SpecularBRDF = 18,
					eSRV_ShadowMap = 19,

					eSRV_DirectionalLight = 20,
					eSRV_PointLight = 21,
					eSRV_SpotLight = 22,
				};

				enum CBSlot
				{
					eCB_LightContents = 4,
					eCB_CommonContents = 5,
				};

				enum SamplerSlot
				{
					eSampler_PointClamp = 1,
					eSampler_Clamp = 2,
				};

				struct LightContents
				{
					uint32_t nDirectionalLightCount{ 0 };
					uint32_t nPointLightCount{ 0 };
					uint32_t nSpotLightCount{ 0 };
					uint32_t padding{ 0 };

					std::array<DirectionalLightData, ILight::eMaxDirectionalLightCount> lightDirectional{};
					std::array<PointLightData, ILight::eMaxPointLightCount> lightPoint{};
					std::array<SpotLightData, ILight::eMaxSpotLightCount> lightSpot{};
				};

				struct CommonContents
				{
					math::Matrix matInvView;
					math::Matrix matInvProj;

					math::Vector3 f3CameraPos;
					int nEnableShadowCount{ 0 };
				};
			}

			class DeferredRenderer::Impl
			{
			public:
				Impl();
				~Impl();

			public:
				void Render(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, Camera* pCamera);
				void Flush();

			private:
				ID3D11VertexShader* m_pVertexShader{ nullptr };
				ID3D11PixelShader* m_pPixelShader{ nullptr };

				ID3D11Buffer* m_pCommonContentsBuffer{ nullptr };
				ID3D11Buffer* m_pLightContentsBuffer{ nullptr };
			};
			
			DeferredRenderer::Impl::Impl()
			{
				ID3D11Device* pDevice = Device::GetInstance()->GetInterface();

				std::string strShaderPath = file::GetPath(file::eFx);
				strShaderPath.append("Model\\Deferred.hlsl");

				ID3DBlob* pShaderBlob = nullptr;
				if (FAILED(D3DReadFileToBlob(String::MultiToWide(strShaderPath).c_str(), &pShaderBlob)))
				{
					throw_line("failed to read shader file : Model.hlsl");
				}

				const D3D_SHADER_MACRO macros[] = 
				{
					{ "DX11", "1" },
					{ nullptr, nullptr },
				};

				{
					ID3DBlob* pVertexShaderBlob = nullptr;
					if (util::CompileShader(pShaderBlob, macros, strShaderPath.c_str(), "VS", "vs_5_0", &pVertexShaderBlob) == false)
					{
						throw_line("failed to compile shader");
					}

					HRESULT hr = pDevice->CreateVertexShader(pVertexShaderBlob->GetBufferPointer(), pVertexShaderBlob->GetBufferSize(), nullptr, &m_pVertexShader);
					if (FAILED(hr))
					{
						throw_line("failed to create vertex shader");
					}
				}

				{
					ID3DBlob* pPixelShaderBlob = nullptr;
					if (util::CompileShader(pShaderBlob, macros, strShaderPath.c_str(), "PS", "ps_5_0", &pPixelShaderBlob) == false)
					{
						throw_line("failed to compile shader");
					}

					HRESULT hr = pDevice->CreatePixelShader(pPixelShaderBlob->GetBufferPointer(), pPixelShaderBlob->GetBufferSize(), nullptr, &m_pPixelShader);
					if (FAILED(hr))
					{
						throw_line("failed to create pixel shader");
					}
				}

				SafeRelease(pShaderBlob);

				if (util::CreateConstantBuffer(pDevice, sizeof(deferredshader::CommonContents), &m_pCommonContentsBuffer) == false)
				{
					throw_line("failed to create CommonContents buffer");
				}

				if (util::CreateConstantBuffer(pDevice, sizeof(deferredshader::LightContents), &m_pLightContentsBuffer) == false)
				{
					throw_line("failed to create LightContents buffer");
				}
			}

			DeferredRenderer::Impl::~Impl()
			{
				SafeRelease(m_pCommonContentsBuffer);
				SafeRelease(m_pLightContentsBuffer);

				SafeRelease(m_pVertexShader);
				SafeRelease(m_pPixelShader);
			}

			void DeferredRenderer::Impl::Render(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, Camera* pCamera)
			{
				Device* pDeviceInstance = Device::GetInstance();
				LightManager* pLightManager = LightManager::GetInstance();

				RenderTarget* pSwapChainRenderTarget = pDeviceInstance->GetSwapChainRenderTarget();
				D3D11_TEXTURE2D_DESC desc{};
				pSwapChainRenderTarget->GetDesc2D(&desc);

				RenderTarget* pRenderTarget = pDeviceInstance->GetRenderTarget(&desc, true);
				if (pRenderTarget == nullptr)
				{
					throw_line("failed to get render target");
				}

				pDeviceContext->ClearRenderTargetView(pRenderTarget->GetRenderTargetView(), &math::Color::Transparent.r);

				pDeviceContext->ClearState();

				pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

				const D3D11_VIEWPORT* pViewport = pDeviceInstance->GetViewport();
				pDeviceContext->RSSetViewports(1, pViewport);

				ID3D11RenderTargetView* pRTV[] = { pRenderTarget->GetRenderTargetView() };
				pDeviceContext->OMSetRenderTargets(_countof(pRTV), pRTV, nullptr);

				pDeviceContext->VSSetShader(m_pVertexShader, nullptr, 0);
				pDeviceContext->PSSetShader(m_pPixelShader, nullptr, 0);

				ID3D11RasterizerState* pRasterizerState = pDeviceInstance->GetRasterizerState(EmRasterizerState::eSolidCCW);
				pDeviceContext->RSSetState(pRasterizerState);

				ID3D11BlendState* pBlendState = pDeviceInstance->GetBlendState(EmBlendState::eOff);
				pDeviceContext->OMSetBlendState(pBlendState, &math::Vector4::Zero.x, 0xffffffff);

				ID3D11DepthStencilState* pDepthStencilState = pDeviceInstance->GetDepthStencilState(EmDepthStencilState::eRead_Write_Off);
				pDeviceContext->OMSetDepthStencilState(pDepthStencilState, 0);

				const GBuffer* pGBuffer = pDeviceInstance->GetGBuffer();
				ID3D11ShaderResourceView* pSRV = pGBuffer->GetDepthStencil()->GetShaderResourceView();
				pDeviceContext->PSSetShaderResources(deferredshader::eSRV_Depth, 1, &pSRV);

				pSRV = pGBuffer->GetRenderTarget(EmGBuffer::eNormals)->GetShaderResourceView();
				pDeviceContext->PSSetShaderResources(deferredshader::eSRV_Normal, 1, &pSRV);

				pSRV = pGBuffer->GetRenderTarget(EmGBuffer::eColors)->GetShaderResourceView();
				pDeviceContext->PSSetShaderResources(deferredshader::eSRV_AlbedoSpecular, 1, &pSRV);

				pSRV = pGBuffer->GetRenderTarget(EmGBuffer::eDisneyBRDF)->GetShaderResourceView();
				pDeviceContext->PSSetShaderResources(deferredshader::eSRV_DisneyBRDF, 1, &pSRV);

				const ImageBasedLight* pImageBasedLight = pDeviceInstance->GetImageBasedLight();
				Texture* pDiffuseHDR = static_cast<Texture*>(pImageBasedLight->GetDiffuseHDR());
				pSRV = pDiffuseHDR->GetShaderResourceView();
				pDeviceContext->PSSetShaderResources(deferredshader::eSRV_DiffuseHDR, 1, &pSRV);

				Texture* pSpecularHDR = static_cast<Texture*>(pImageBasedLight->GetSpecularHDR());
				pSRV = pSpecularHDR->GetShaderResourceView();
				pDeviceContext->PSSetShaderResources(deferredshader::eSRV_SpecularHDR, 1, &pSRV);

				Texture* pSpecularBRDF = static_cast<Texture*>(pImageBasedLight->GetSpecularBRDF());
				pSRV = pSpecularBRDF->GetShaderResourceView();
				pDeviceContext->PSSetShaderResources(deferredshader::eSRV_SpecularBRDF, 1, &pSRV);

				ID3D11SamplerState* pSamplerPointClamp = pDeviceInstance->GetSamplerState(EmSamplerState::eMinMagMipPointClamp);
				pDeviceContext->PSSetSamplers(deferredshader::eSampler_PointClamp, 1, &pSamplerPointClamp);

				ID3D11SamplerState* pSamplerClamp = pDeviceInstance->GetSamplerState(EmSamplerState::eMinMagMipLinearClamp);
				pDeviceContext->PSSetSamplers(deferredshader::eSampler_Clamp, 1, &pSamplerClamp);

				D3D11_MAPPED_SUBRESOURCE mapped{};
				HRESULT hr = pDeviceContext->Map(m_pCommonContentsBuffer, 0, D3D11_MAP::D3D11_MAP_WRITE_DISCARD, 0, &mapped);
				if (FAILED(hr))
				{
					throw_line("failed to map CommonContents buffer");
				}

				deferredshader::CommonContents* pCommonContents = reinterpret_cast<deferredshader::CommonContents*>(mapped.pData);
				pCommonContents->matInvView = pCamera->GetViewMatrix().Invert().Transpose();
				pCommonContents->matInvProj = pCamera->GetProjMatrix().Invert().Transpose();

				pCommonContents->f3CameraPos = pCamera->GetPosition();
				pDeviceContext->Unmap(m_pCommonContentsBuffer, 0);

				pDeviceContext->PSSetConstantBuffers(deferredshader::eCB_CommonContents, 1, &m_pCommonContentsBuffer);

				hr = pDeviceContext->Map(m_pLightContentsBuffer, 0, D3D11_MAP::D3D11_MAP_WRITE_DISCARD, 0, &mapped);
				if (FAILED(hr))
				{
					throw_line("failed to map LightContents buffer");
				}

				deferredshader::LightContents* pLightContents = reinterpret_cast<deferredshader::LightContents*>(mapped.pData);
				const DirectionalLightData* pDirectionalLightData = nullptr;
				pLightManager->GetDirectionalLightData(&pDirectionalLightData, &pLightContents->nDirectionalLightCount);
				Memory::Copy(pLightContents->lightDirectional.data(), sizeof(pLightContents->lightDirectional), pDirectionalLightData, sizeof(DirectionalLightData) * pLightContents->nDirectionalLightCount);

				const PointLightData* pPointLightData = nullptr;
				pLightManager->GetPointLightData(&pPointLightData, &pLightContents->nPointLightCount);
				Memory::Copy(pLightContents->lightPoint.data(), sizeof(pLightContents->lightPoint), pPointLightData, sizeof(PointLightData) * pLightContents->nPointLightCount);

				const SpotLightData* pSpotLightData = nullptr;
				pLightManager->GetSpotLightData(&pSpotLightData, &pLightContents->nSpotLightCount);
				Memory::Copy(pLightContents->lightSpot.data(), sizeof(pLightContents->lightSpot), pSpotLightData, sizeof(SpotLightData) * pLightContents->nSpotLightCount);

				pDeviceContext->Unmap(m_pLightContentsBuffer, 0);

				pDeviceContext->PSSetConstantBuffers(deferredshader::eCB_LightContents, 1, &m_pLightContentsBuffer);

				pDeviceContext->Draw(4, 0);

				pDeviceInstance->ReleaseRenderTarget(&pRenderTarget);
			}

			void DeferredRenderer::Impl::Flush()
			{
			}

			DeferredRenderer::DeferredRenderer()
				: m_pImpl{ std::make_unique<Impl>() }
			{
			}

			DeferredRenderer::~DeferredRenderer()
			{
			}

			void DeferredRenderer::Render(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, Camera* pCamera)
			{
				m_pImpl->Render(pDevice, pDeviceContext, pCamera);
			}

			void DeferredRenderer::Flush()
			{
				m_pImpl->Flush();
			}
		}
	}
}