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
			namespace shader
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
					eCB_DeferredContents = 0,
					eCB_CommonContents = 5,
				};

				enum SamplerSlot
				{
					eSampler_PointClamp = 1,
					eSampler_Clamp = 2,
				};

				struct DeferredCcontents
				{
					math::Matrix matInvView;
					math::Matrix matInvProj;
				};

				struct CommonContents
				{
					math::Vector3 f3CameraPos;
					int nEnableShadowCount{ 0 };

					uint32_t nDirectionalLightCount{ 0 };
					uint32_t nPointLightCount{ 0 };
					uint32_t nSpotLightCount{ 0 };
					uint32_t padding{ 0 };

					std::array<DirectionalLightData, ILight::eMaxDirectionalLightCount> lightDirectional{};
					std::array<PointLightData, ILight::eMaxPointLightCount> lightPoint{};
					std::array<SpotLightData, ILight::eMaxSpotLightCount> lightSpot{};
				};

				void SetDeferredCcontents(ID3D11DeviceContext* pDeviceContext,
					ConstantBuffer<DeferredCcontents>* pCB_DeferredCcontents, 
					const Camera* pCamera)
				{
					shader::DeferredCcontents* pDeferredContents = pCB_DeferredCcontents->Map(pDeviceContext);
					pDeferredContents->matInvView = pCamera->GetViewMatrix().Invert().Transpose();
					pDeferredContents->matInvProj = pCamera->GetProjMatrix().Invert().Transpose();
					pCB_DeferredCcontents->Unmap(pDeviceContext);
				}

				void SetCommonContents(ID3D11DeviceContext* pDeviceContext,
					ConstantBuffer<CommonContents>* pCB_CommonContents,
					const LightManager* pLightManager, const math::Vector3& f3CameraPos, int nEnableShadowCount)
				{
					CommonContents* pCommonContents = pCB_CommonContents->Map(pDeviceContext);

					pCommonContents->f3CameraPos = f3CameraPos;
					pCommonContents->nEnableShadowCount = nEnableShadowCount;

					const DirectionalLightData* pDirectionalLightData = nullptr;
					pLightManager->GetDirectionalLightData(&pDirectionalLightData, &pCommonContents->nDirectionalLightCount);
					Memory::Copy(pCommonContents->lightDirectional.data(), sizeof(pCommonContents->lightDirectional), pDirectionalLightData, sizeof(DirectionalLightData) * pCommonContents->nDirectionalLightCount);

					const PointLightData* pPointLightData = nullptr;
					pLightManager->GetPointLightData(&pPointLightData, &pCommonContents->nPointLightCount);
					Memory::Copy(pCommonContents->lightPoint.data(), sizeof(pCommonContents->lightPoint), pPointLightData, sizeof(PointLightData) * pCommonContents->nPointLightCount);

					const SpotLightData* pSpotLightData = nullptr;
					pLightManager->GetSpotLightData(&pSpotLightData, &pCommonContents->nSpotLightCount);
					Memory::Copy(pCommonContents->lightSpot.data(), sizeof(pCommonContents->lightSpot), pSpotLightData, sizeof(SpotLightData) * pCommonContents->nSpotLightCount);

					pCB_CommonContents->Unmap(pDeviceContext);
				}
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

				ConstantBuffer<shader::DeferredCcontents> m_deferredCcontents;
				ConstantBuffer<shader::CommonContents> m_commonContents;
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

				if (util::CreateVertexShader(pDevice, pShaderBlob, macros, strShaderPath.c_str(), "VS", "vs_5_0", &m_pVertexShader, "DeferredRenderer_VS") == false)
				{
					throw_line("failed to create DeferredRenderer VertexShader");
				}

				if (util::CreatePixelShader(pDevice, pShaderBlob, macros, strShaderPath.c_str(), "PS", "ps_5_0", &m_pPixelShader, "DeferredRenderer_PS") == false)
				{
					throw_line("failed to create DeferredRenderer PixelShader");
				}

				SafeRelease(pShaderBlob);

				m_deferredCcontents.Create(pDevice, "DeferredContents");
				m_commonContents.Create(pDevice, "CommonContents");
			}

			DeferredRenderer::Impl::~Impl()
			{
				SafeRelease(m_pVertexShader);
				SafeRelease(m_pPixelShader);

				m_deferredCcontents.Destroy();
				m_commonContents.Destroy();
			}

			void DeferredRenderer::Impl::Render(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, Camera* pCamera)
			{
				DX_PROFILING(DeferredRenderer);

				Device* pDeviceInstance = Device::GetInstance();
				LightManager* pLightManager = LightManager::GetInstance();

				D3D11_TEXTURE2D_DESC desc{};
				pDeviceInstance->GetSwapChainRenderTarget()->GetDesc2D(&desc);
				desc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;

				if (GetOptions().OnHDR == true)
				{
					desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
				}

				RenderTarget* pRenderTarget = pDeviceInstance->GetRenderTarget(&desc, true);
				if (pRenderTarget == nullptr)
				{
					throw_line("failed to get render target");
				}

				const IImageBasedLight* pImageBasedLight = pDeviceInstance->GetImageBasedLight();
				{
					if (pImageBasedLight->GetEnvironmentHDR() == nullptr ||
						pImageBasedLight->GetEnvironmentSphereVB() == nullptr ||
						pImageBasedLight->GetEnvironmentSphereIB() == nullptr)
					{
						pDeviceContext->ClearRenderTargetView(pRenderTarget->GetRenderTargetView(), &math::Color::Transparent.r);
					}
				}

				pDeviceContext->ClearState();

				const D3D11_VIEWPORT* pViewport = pDeviceInstance->GetViewport();
				pDeviceContext->RSSetViewports(1, pViewport);

				ID3D11RenderTargetView* pRTV[] = { pRenderTarget->GetRenderTargetView() };
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

				const GBuffer* pGBuffer = pDeviceInstance->GetGBuffer();
				ID3D11ShaderResourceView* pSRV = pGBuffer->GetDepthStencil()->GetShaderResourceView();
				pDeviceContext->PSSetShaderResources(shader::eSRV_Depth, 1, &pSRV);

				pSRV = pGBuffer->GetRenderTarget(EmGBuffer::eNormals)->GetShaderResourceView();
				pDeviceContext->PSSetShaderResources(shader::eSRV_Normal, 1, &pSRV);

				pSRV = pGBuffer->GetRenderTarget(EmGBuffer::eColors)->GetShaderResourceView();
				pDeviceContext->PSSetShaderResources(shader::eSRV_AlbedoSpecular, 1, &pSRV);

				pSRV = pGBuffer->GetRenderTarget(EmGBuffer::eDisneyBRDF)->GetShaderResourceView();
				pDeviceContext->PSSetShaderResources(shader::eSRV_DisneyBRDF, 1, &pSRV);

				Texture* pDiffuseHDR = static_cast<Texture*>(pImageBasedLight->GetDiffuseHDR());
				pSRV = pDiffuseHDR->GetShaderResourceView();
				pDeviceContext->PSSetShaderResources(shader::eSRV_DiffuseHDR, 1, &pSRV);

				Texture* pSpecularHDR = static_cast<Texture*>(pImageBasedLight->GetSpecularHDR());
				pSRV = pSpecularHDR->GetShaderResourceView();
				pDeviceContext->PSSetShaderResources(shader::eSRV_SpecularHDR, 1, &pSRV);

				Texture* pSpecularBRDF = static_cast<Texture*>(pImageBasedLight->GetSpecularBRDF());
				pSRV = pSpecularBRDF->GetShaderResourceView();
				pDeviceContext->PSSetShaderResources(shader::eSRV_SpecularBRDF, 1, &pSRV);

				ID3D11SamplerState* pSamplerPointClamp = pDeviceInstance->GetSamplerState(EmSamplerState::eMinMagMipPointClamp);
				pDeviceContext->PSSetSamplers(shader::eSampler_PointClamp, 1, &pSamplerPointClamp);

				ID3D11SamplerState* pSamplerClamp = pDeviceInstance->GetSamplerState(EmSamplerState::eMinMagMipLinearClamp);
				pDeviceContext->PSSetSamplers(shader::eSampler_Clamp, 1, &pSamplerClamp);

				shader::SetDeferredCcontents(pDeviceContext, &m_deferredCcontents, pCamera);
				pDeviceContext->PSSetConstantBuffers(shader::eCB_DeferredContents, 1, &m_deferredCcontents.pBuffer);

				shader::SetCommonContents(pDeviceContext, &m_commonContents, pLightManager, pCamera->GetPosition(), 0);
				pDeviceContext->PSSetConstantBuffers(shader::eCB_CommonContents, 1, &m_commonContents.pBuffer);

				pDeviceContext->Draw(4, 0);

				pDeviceInstance->ReleaseRenderTargets(&pRenderTarget);
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