#include "stdafx.h"
#include "DeferredRendererDX11.h"

#include "CommonLib/FileUtil.h"

#include "Graphics/Interface/Camera.h"

#include "UtilDX11.h"
#include "DeviceDX11.h"
#include "GBufferDX11.h"
#include "LightResourceManagerDX11.h"

#include "TextureDX11.h"

namespace est
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

					eSRV_DiffuseHDR = 17,
					eSRV_SpecularHDR = 18,
					eSRV_SpecularBRDF = 19,
					eSRV_ShadowMap = 20,
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
					eSampler_ShadowPCF = 3,
				};

				struct DeferredCcontents
				{
					math::Matrix matInvView;
					math::Matrix matInvProj;
				};

				struct CommonContents
				{
					math::float3 f3CameraPos;
					float farClip{ 0.f };

					uint32_t directionalLightCount{ 0 };
					uint32_t pointLightCount{ 0 };
					uint32_t spotLightCount{ 0 };
					uint32_t cascadeShadowCount{ 0 };

					std::array<DirectionalLightData, ILight::eMaxDirectionalLightCount> lightDirectional{};
					std::array<PointLightData, ILight::eMaxPointLightCount> lightPoint{};
					std::array<SpotLightData, ILight::eMaxSpotLightCount> lightSpot{};

					std::array<CascadedShadowData, ILight::eMaxDirectionalLightCount> cascadedShadow{};
				};

				void SetDeferredCcontents(ID3D11DeviceContext* pDeviceContext,
					ConstantBuffer<DeferredCcontents>* pCB_DeferredCcontents, 
					Camera* pCamera)
				{
					shader::DeferredCcontents* pDeferredContents = pCB_DeferredCcontents->Map(pDeviceContext);
					pDeferredContents->matInvView = pCamera->GetViewMatrix().Invert().Transpose();
					pDeferredContents->matInvProj = pCamera->GetProjectionMatrix().Invert().Transpose();
					pCB_DeferredCcontents->Unmap(pDeviceContext);
				}

				void SetCommonContents(ID3D11DeviceContext* pDeviceContext,
					ConstantBuffer<CommonContents>* pCB_CommonContents,
					const LightResourceManager* pLightResourceManager, const math::float3& f3CameraPos)
				{
					CommonContents* pCommonContents = pCB_CommonContents->Map(pDeviceContext);

					pCommonContents->f3CameraPos = f3CameraPos;

					const DirectionalLightData* pDirectionalLightData = nullptr;
					pLightResourceManager->GetDirectionalLightRenderData(&pDirectionalLightData, &pCommonContents->directionalLightCount);
					memory::Copy(pCommonContents->lightDirectional.data(), sizeof(pCommonContents->lightDirectional), pDirectionalLightData, sizeof(DirectionalLightData) * pCommonContents->directionalLightCount);

					const PointLightData* pPointLightData = nullptr;
					pLightResourceManager->GetPointLightRenderData(&pPointLightData, &pCommonContents->pointLightCount);
					memory::Copy(pCommonContents->lightPoint.data(), sizeof(pCommonContents->lightPoint), pPointLightData, sizeof(PointLightData) * pCommonContents->pointLightCount);

					const SpotLightData* pSpotLightData = nullptr;
					pLightResourceManager->GetSpotLightRenderData(&pSpotLightData, &pCommonContents->spotLightCount);
					memory::Copy(pCommonContents->lightSpot.data(), sizeof(pCommonContents->lightSpot), pSpotLightData, sizeof(SpotLightData) * pCommonContents->spotLightCount);

					pCommonContents->cascadeShadowCount = pLightResourceManager->GetShadowCount(ILight::Type::eDirectional);

					for (uint32_t i = 0; i < pCommonContents->directionalLightCount; ++i)
					{
						DirectionalLightPtr pDirectionalLight = std::static_pointer_cast<IDirectionalLight>(pLightResourceManager->GetLight(ILight::Type::eDirectional, i));
						if (pDirectionalLight != nullptr)
						{
							const CascadedShadows& cascadedShadows = pDirectionalLight->GetRenderCascadedShadows();
							pCommonContents->cascadedShadow[i] = cascadedShadows.GetRenderData();
						}
					}

					pCB_CommonContents->Unmap(pDeviceContext);
				}
			}

			class DeferredRenderer::Impl
			{
			public:
				Impl();
				~Impl();

			public:
				void Render(const RenderElement& renderElement);

			private:
				ID3D11VertexShader* m_pVertexShader{ nullptr };
				ID3D11PixelShader* m_pPixelShader{ nullptr };

				ID3D11SamplerState* m_pSamplerShadowPCF{ nullptr };

				ConstantBuffer<shader::DeferredCcontents> m_deferredCcontents;
				ConstantBuffer<shader::CommonContents> m_commonContents;
			};
			
			DeferredRenderer::Impl::Impl()
			{
				ID3D11Device* pDevice = Device::GetInstance()->GetInterface();

				std::wstring shaderPath = file::GetEngineDataPath();
				shaderPath.append(L"Fx\\Model\\Deferred.hlsl");

				ID3DBlob* pShaderBlob = nullptr;
				if (FAILED(D3DReadFileToBlob(shaderPath.c_str(), &pShaderBlob)))
				{
					throw_line("failed to read shader file : Model.hlsl");
				}

				const D3D_SHADER_MACRO macros[] = 
				{
					{ "DX11", "1" },
					{ nullptr, nullptr },
				};

				if (util::CreateVertexShader(pDevice, pShaderBlob, macros, shaderPath.c_str(), "VS", shader::VS_CompileVersion, &m_pVertexShader, "DeferredRenderer_VS") == false)
				{
					throw_line("failed to create DeferredRenderer VertexShader");
				}

				if (util::CreatePixelShader(pDevice, pShaderBlob, macros, shaderPath.c_str(), "PS", shader::PS_CompileVersion, &m_pPixelShader, "DeferredRenderer_PS") == false)
				{
					throw_line("failed to create DeferredRenderer PixelShader");
				}

				SafeRelease(pShaderBlob);

				m_deferredCcontents.Create(pDevice, "DeferredContents");
				m_commonContents.Create(pDevice, "CommonContents");

				D3D11_SAMPLER_DESC samplerDesc;
				samplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
				samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
				samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
				samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
				samplerDesc.MipLODBias = 0.f;
				samplerDesc.MaxAnisotropy = 0;
				samplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS;
				samplerDesc.BorderColor[0] = samplerDesc.BorderColor[1] = samplerDesc.BorderColor[2] = samplerDesc.BorderColor[3] = 0.f;
				samplerDesc.MinLOD = 0.f;
				samplerDesc.MaxLOD = 0.f;
				
				HRESULT hr = pDevice->CreateSamplerState(&samplerDesc, &m_pSamplerShadowPCF);
				if (FAILED(hr))
				{
					throw_line("failed to create sampler state");
				}
			}

			DeferredRenderer::Impl::~Impl()
			{
				SafeRelease(m_pVertexShader);
				SafeRelease(m_pPixelShader);

				m_deferredCcontents.Destroy();
				m_commonContents.Destroy();
			}

			void DeferredRenderer::Impl::Render(const RenderElement& renderElement)
			{
				TRACER_EVENT(__FUNCTIONW__);
				DX_PROFILING(DeferredRenderer);

				Device* pDeviceInstance = Device::GetInstance();
				LightResourceManager* pLightResourceManager = pDeviceInstance->GetLightResourceManager();
				const IImageBasedLight* pImageBasedLight = pDeviceInstance->GetImageBasedLight();

				Camera* pCamera = renderElement.pCamera;

				ID3D11DeviceContext* pDeviceContext = renderElement.pDeviceContext;
				pDeviceContext->ClearState();

				const math::Viewport& viewport = pDeviceInstance->GetViewport();
				pDeviceContext->RSSetViewports(1, util::Convert(viewport));

				pDeviceContext->OMSetRenderTargets(renderElement.rtvCount, renderElement.pRTVs, renderElement.pDSV);

				pDeviceContext->VSSetShader(m_pVertexShader, nullptr, 0);
				pDeviceContext->PSSetShader(m_pPixelShader, nullptr, 0);

				pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

				ID3D11RasterizerState* pRasterizerState = pDeviceInstance->GetRasterizerState(RasterizerState::eSolidCCW);
				pDeviceContext->RSSetState(pRasterizerState);

				ID3D11BlendState* pBlendState = pDeviceInstance->GetBlendState(BlendState::eOff);
				pDeviceContext->OMSetBlendState(pBlendState, &math::float4::Zero.x, 0xffffffff);

				ID3D11DepthStencilState* pDepthStencilState = pDeviceInstance->GetDepthStencilState(DepthStencilState::eRead_Write_Off);
				pDeviceContext->OMSetDepthStencilState(pDepthStencilState, 0);

				const GBuffer* pGBuffer = pDeviceInstance->GetGBuffer();
				ID3D11ShaderResourceView* pSRV = pGBuffer->GetDepthStencil()->GetShaderResourceView();
				pDeviceContext->PSSetShaderResources(shader::eSRV_Depth, 1, &pSRV);

				pSRV = pGBuffer->GetRenderTarget(GBufferType::eNormals)->GetShaderResourceView();
				pDeviceContext->PSSetShaderResources(shader::eSRV_Normal, 1, &pSRV);

				pSRV = pGBuffer->GetRenderTarget(GBufferType::eColors)->GetShaderResourceView();
				pDeviceContext->PSSetShaderResources(shader::eSRV_AlbedoSpecular, 1, &pSRV);

				pSRV = pGBuffer->GetRenderTarget(GBufferType::eDisneyBRDF)->GetShaderResourceView();
				pDeviceContext->PSSetShaderResources(shader::eSRV_DisneyBRDF, 1, &pSRV);

				Texture* pDiffuseHDR = static_cast<Texture*>(pImageBasedLight->GetDiffuseHDR().get());
				pSRV = pDiffuseHDR->GetShaderResourceView();
				pDeviceContext->PSSetShaderResources(shader::eSRV_DiffuseHDR, 1, &pSRV);

				Texture* pSpecularHDR = static_cast<Texture*>(pImageBasedLight->GetSpecularHDR().get());
				pSRV = pSpecularHDR->GetShaderResourceView();
				pDeviceContext->PSSetShaderResources(shader::eSRV_SpecularHDR, 1, &pSRV);

				Texture* pSpecularBRDF = static_cast<Texture*>(pImageBasedLight->GetSpecularBRDF().get());
				pSRV = pSpecularBRDF->GetShaderResourceView();
				pDeviceContext->PSSetShaderResources(shader::eSRV_SpecularBRDF, 1, &pSRV);

				std::vector<ID3D11ShaderResourceView*> cascadeShadowMaps;
				const size_t directionalLightCount = pLightResourceManager->GetLightCount(ILight::Type::eDirectional);
				for (size_t i = 0; i < directionalLightCount; ++i)
				{
					DirectionalLightPtr pDirectionalLight = std::static_pointer_cast<IDirectionalLight>(pLightResourceManager->GetLight(ILight::Type::eDirectional, i));
					if (pDirectionalLight != nullptr)
					{
						DepthStencil* pDepthStencil = static_cast<DepthStencil*>(pDirectionalLight->GetDepthMapResource());
						if (pDepthStencil != nullptr)
						{
							cascadeShadowMaps.emplace_back(pDepthStencil->GetShaderResourceView());
						}
					}
				}
				pDeviceContext->PSSetShaderResources(shader::eSRV_ShadowMap, static_cast<uint32_t>(cascadeShadowMaps.size()), cascadeShadowMaps.data());

				pDeviceContext->PSSetSamplers(shader::eSampler_ShadowPCF, 1, &m_pSamplerShadowPCF);

				ID3D11SamplerState* pSamplerPointClamp = pDeviceInstance->GetSamplerState(SamplerState::eMinMagMipPointClamp);
				pDeviceContext->PSSetSamplers(shader::eSampler_PointClamp, 1, &pSamplerPointClamp);

				ID3D11SamplerState* pSamplerClamp = pDeviceInstance->GetSamplerState(SamplerState::eMinMagMipLinearClamp);
				pDeviceContext->PSSetSamplers(shader::eSampler_Clamp, 1, &pSamplerClamp);

				shader::SetDeferredCcontents(pDeviceContext, &m_deferredCcontents, pCamera);
				pDeviceContext->PSSetConstantBuffers(shader::eCB_DeferredContents, 1, &m_deferredCcontents.pBuffer);

				shader::SetCommonContents(pDeviceContext, &m_commonContents, pLightResourceManager, pCamera->GetPosition());
				pDeviceContext->PSSetConstantBuffers(shader::eCB_CommonContents, 1, &m_commonContents.pBuffer);

				pDeviceContext->Draw(4, 0);
			}

			DeferredRenderer::DeferredRenderer()
				: m_pImpl{ std::make_unique<Impl>() }
			{
			}

			DeferredRenderer::~DeferredRenderer()
			{
			}

			void DeferredRenderer::Render(const RenderElement& renderElement)
			{
				m_pImpl->Render(renderElement);
			}
		}
	}
}