#include "stdafx.h"
#include "DeferredRenderer.h"

#include "CommonLib/FileUtil.h"
#include "CommonLib/Config.h"

#include "DirectX/Camera.h"

#include "DirectX/LightMgr.h"
#include "DirectX/PointLight.h"
#include "DirectX/SpotLight.h"

#include "FXAA.h"

namespace StrID
{
	RegisterStringID(EffectDeferred);
	RegisterStringID(EffectDeferredShadow);

	RegisterStringID(Light);
	RegisterStringID(BRDF);
	RegisterStringID(Shadow);

	RegisterStringID(Deferred);

	RegisterStringID(Deferred_CascadedShadow);
	RegisterStringID(Deferred_ShadowMap);
	RegisterStringID(Deferred_ShadowCubeMap);

	RegisterStringID(g_texDepth);
	RegisterStringID(g_texNormal);
	RegisterStringID(g_texAlbedoSpecular);
	RegisterStringID(g_texDisneyBRDF);
	RegisterStringID(g_texShadowMap);
	RegisterStringID(g_texShadowCubeMap);
	RegisterStringID(g_texIBLMap);
	RegisterStringID(g_texDiffuseHDR);
	RegisterStringID(g_texSpecularHDR);
	RegisterStringID(g_texSpecularBRDF);
	RegisterStringID(g_f3CameraPos);
	RegisterStringID(g_nEnableShadowCount);
	RegisterStringID(g_matInvView);
	RegisterStringID(g_matInvProj);
	RegisterStringID(g_lightDirectional);
	RegisterStringID(g_lightPoint);
	RegisterStringID(g_lightSpot);
	RegisterStringID(g_nDirectionalLightCount);
	RegisterStringID(g_nPointLightCount);
	RegisterStringID(g_nSpotLightCount);

	RegisterStringID(g_samShadow);
	RegisterStringID(g_cascadeShadow);
	RegisterStringID(g_shadowMap);
	RegisterStringID(g_shadowCubeMap);
}

namespace EastEngine
{
	namespace Graphics
	{
		class DeferredRenderer::Impl
		{
		public:
			Impl();
			~Impl();

		public:
			void Render(IDevice* pDevice, IDeviceContext* pDeviceContext, Camera* pCamera, uint32_t nRenderGroupFlag);
			void Flush();

		private:
			int RenderShadowMap(IDevice* pDevice, IDeviceContext* pDeviceContext, Camera* pCamera, IRenderTarget* pRenderTarget);

		private:
			bool CreateEffect();
			void ClearEffect(IDeviceContext* pd3dDeviceContext, IEffectTech* pTech);
			void ClearEffect_Shadow(IDeviceContext* pd3dDeviceContext, IEffectTech* pTech);

		private:
			IEffect* m_pEffect{ nullptr };
			IEffect* m_pEffectShadow{ nullptr };
		};

		DeferredRenderer::Impl::Impl()
		{
			if (CreateEffect() == false)
			{
				assert(false);
			}
		}

		DeferredRenderer::Impl::~Impl()
		{
			IEffect::Destroy(&m_pEffect);
			IEffect::Destroy(&m_pEffectShadow);
		}

		void DeferredRenderer::Impl::Render(IDevice* pDevice, IDeviceContext* pDeviceContext, Camera* pCamera, uint32_t nRenderGroupFlag)
		{
			PERF_TRACER_EVENT("DeferredRenderer::Render", "");

			int nEnableShadowCount = 0;
			IRenderTarget* pRenderTargetShadow = nullptr;
			if (Config::IsEnable("Shadow"_s) == true)
			{
				auto desc = pDevice->GetMainRenderTarget()->GetDesc2D();
				desc.Format = DXGI_FORMAT_R16G16_FLOAT;
				desc.Build();

				pRenderTargetShadow = pDevice->GetRenderTarget(desc);

				nEnableShadowCount = RenderShadowMap(pDevice, pDeviceContext, pCamera, pRenderTargetShadow);

				/*if (nEnableShadowCount > 0)
				{
				IRenderTarget* pFxaa = pDevice->GetRenderTarget(pRenderTargetShadow->GetDesc2D(), false);
				IRenderTarget* pSource = pRenderTargetShadow;
				FXAA::GetInstance()->Apply(pFxaa, pSource);

				pDevice->ReleaseRenderTargets(&pSource);

				pRenderTargetShadow = pFxaa;
				}*/
			}

			IGBuffers* pGBuffers = GetGBuffers();
			IImageBasedLight* pIBL = GetImageBasedLight();

			pDeviceContext->ClearState();

			pDeviceContext->SetDefaultViewport();
			pDeviceContext->SetBlendState(EmBlendState::eOff);
			pDeviceContext->SetRasterizerState(EmRasterizerState::eSolidCCW);
			pDeviceContext->SetDepthStencilState(EmDepthStencilState::eRead_Write_Off);

			pDeviceContext->SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

			{
				D3D_PROFILING(pDeviceContext, DeferredRendering);
				IEffectTech* pEffectTech = m_pEffect->GetTechnique(StrID::Deferred);
				if (pEffectTech == nullptr)
				{
					LOG_ERROR("Not Exist EffectTech !! : %s", StrID::Deferred.c_str());
					pDevice->ReleaseRenderTargets(&pRenderTargetShadow, 1, false);
					return;
				}

				IRenderTarget* pRenderTarget = nullptr;
				if (Config::IsEnable("HDRFilter"_s) == true)
				{
					auto desc = pDevice->GetMainRenderTarget()->GetDesc2D();
					if (Config::IsEnable("HDRFilter"_s) == true)
					{
						desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
						desc.Build();
					}

					pRenderTarget = pDevice->GetRenderTarget(desc);
				}
				else
				{
					auto& desc = pDevice->GetMainRenderTarget()->GetDesc2D();
					pRenderTarget = pDevice->GetRenderTarget(desc);
				}

				pDeviceContext->SetRenderTargets(&pRenderTarget, 1, nullptr);

				int nThreadID = GetThreadID(ThreadType::eRender);
				const Math::Matrix& matInvView = pCamera->GetViewMatrix(nThreadID).Invert();
				const Math::Matrix& matInvProj = pCamera->GetProjMatrix(nThreadID).Invert();

				m_pEffect->SetVector(StrID::g_f3CameraPos, matInvView.Translation());

				if (nEnableShadowCount > 0)
				{
					m_pEffect->SetInt(StrID::g_nEnableShadowCount, nEnableShadowCount);
					m_pEffect->SetTexture(StrID::g_texShadowMap, pRenderTargetShadow->GetTexture());
				}
				else
				{
					m_pEffect->SetInt(StrID::g_nEnableShadowCount, 0);
					m_pEffect->SetTexture(StrID::g_texShadowMap, nullptr);
				}

				m_pEffect->SetMatrix(StrID::g_matInvView, matInvView);
				m_pEffect->SetMatrix(StrID::g_matInvProj, matInvProj);

				m_pEffect->SetStructuredBuffer(StrID::g_lightDirectional, LightManager::GetInstance()->GetLightBuffer(EmLight::eDirectional));
				m_pEffect->SetStructuredBuffer(StrID::g_lightPoint, LightManager::GetInstance()->GetLightBuffer(EmLight::ePoint));
				m_pEffect->SetStructuredBuffer(StrID::g_lightSpot, LightManager::GetInstance()->GetLightBuffer(EmLight::eSpot));

				m_pEffect->SetInt(StrID::g_nDirectionalLightCount, LightManager::GetInstance()->GetLightCountInView(EmLight::eDirectional));
				m_pEffect->SetInt(StrID::g_nPointLightCount, LightManager::GetInstance()->GetLightCountInView(EmLight::ePoint));
				m_pEffect->SetInt(StrID::g_nSpotLightCount, LightManager::GetInstance()->GetLightCountInView(EmLight::eSpot));

				m_pEffect->SetTexture(StrID::g_texDepth, pDevice->GetMainDepthStencil()->GetTexture());
				m_pEffect->SetTexture(StrID::g_texNormal, pGBuffers->GetGBuffer(EmGBuffer::eNormals)->GetTexture());
				m_pEffect->SetTexture(StrID::g_texAlbedoSpecular, pGBuffers->GetGBuffer(EmGBuffer::eColors)->GetTexture());
				m_pEffect->SetTexture(StrID::g_texDisneyBRDF, pGBuffers->GetGBuffer(EmGBuffer::eDisneyBRDF)->GetTexture());

				m_pEffect->SetTexture(StrID::g_texDiffuseHDR, pIBL->GetDiffuseHDR());

				m_pEffect->SetTexture(StrID::g_texSpecularHDR, pIBL->GetSpecularHDR());
				m_pEffect->SetTexture(StrID::g_texSpecularBRDF, pIBL->GetSpecularBRDF());

				uint32_t nPassCount = pEffectTech->GetPassCount();
				for (uint32_t p = 0; p < nPassCount; ++p)
				{
					pEffectTech->PassApply(p, pDeviceContext);

					pDeviceContext->Draw(4, 0);
				}

				ClearEffect(pDeviceContext, pEffectTech);

				pDevice->ReleaseRenderTargets(&pRenderTargetShadow, 1, false);

				pDevice->ReleaseRenderTargets(&pRenderTarget);
			}
		}

		void DeferredRenderer::Impl::Flush()
		{
		}

		int DeferredRenderer::Impl::RenderShadowMap(IDevice* pDevice, IDeviceContext* pDeviceContext, Camera* pCamera, IRenderTarget* pRenderTarget)
		{
			PERF_TRACER_EVENT("DeferredRenderer::RenderShadowMap", "");
			pDeviceContext->ClearRenderTargetView(pRenderTarget, Math::Color::Black);

			bool isEnableShadow = false;
			for (int i = 0; i < EmLight::eCount; ++i)
			{
				EmLight::Type emType = static_cast<EmLight::Type>(i);
				uint32_t nCount = LightManager::GetInstance()->GetLightCount(emType);
				for (uint32_t j = 0; j < nCount; ++j)
				{
					ILight* pLight = LightManager::GetInstance()->GetLight(emType, j);
					if (pLight != nullptr && pLight->IsEnableShadow() == true)
					{
						isEnableShadow = true;
						break;
					}
				}

				if (isEnableShadow == true)
					break;
			}

			if (isEnableShadow == false)
				return 0;

			pDeviceContext->ClearState();

			pDeviceContext->SetDefaultViewport();
			pDeviceContext->SetBlendState(EmBlendState::eAdditive);
			pDeviceContext->SetRasterizerState(EmRasterizerState::eSolidCCW);
			pDeviceContext->SetDepthStencilState(EmDepthStencilState::eRead_Write_Off);

			pDeviceContext->SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

			int nShadowCount = 0;
			{
				D3D_PROFILING(pDeviceContext, DeferredShdaow);

				pDeviceContext->SetRenderTargets(&pRenderTarget, 1, nullptr);

				int nThreadID = GetThreadID(ThreadType::eRender);
				const Math::Matrix& matInvView = pCamera->GetViewMatrix(nThreadID).Invert();
				const Math::Matrix& matInvProj = pCamera->GetProjMatrix(nThreadID).Invert();

				m_pEffectShadow->SetVector(StrID::g_f3CameraPos, matInvView.Translation());

				m_pEffectShadow->SetMatrix(StrID::g_matInvView, matInvView);
				m_pEffectShadow->SetMatrix(StrID::g_matInvProj, matInvProj);

				IEffectTech* pEffectTech = nullptr;
				uint32_t nPassCount = 0;

				for (int i = 0; i < EmLight::eCount; ++i)
				{
					EmLight::Type emType = static_cast<EmLight::Type>(i);
					uint32_t nCount = LightManager::GetInstance()->GetLightCount(emType);
					for (uint32_t j = 0; j < nCount; ++j)
					{
						ILight* pLight = LightManager::GetInstance()->GetLight(emType, j);
						if (pLight != nullptr && pLight->IsEnableShadow())
						{
							switch (pLight->GetType())
							{
							case EmLight::eDirectional:
							{
								D3D_PROFILING(pDeviceContext, Deferred_CascadedShadow);
								pEffectTech = m_pEffectShadow->GetTechnique(StrID::Deferred_CascadedShadow);
								if (pEffectTech == nullptr)
								{
									LOG_ERROR("Not Exist EffectTech !! : %s", StrID::Deferred_CascadedShadow.c_str());
									continue;
								}

								nPassCount = pEffectTech->GetPassCount();

								IDirectionalLight* pDirectionalLight = static_cast<IDirectionalLight*>(pLight);

								struct CascadedShadow
								{
									Math::Int2 n2PCFBlurSize;
									Math::Vector2 f2TexelOffset;

									int nCascadeLevel = 0;
									float fDepthBias = 0.f;
									Math::Vector2 padding;

									Math::Matrix matCascadeViewProj[CascadedShadowsConfig::eMaxLevel];
									Math::Vector4 f2SplitDpeths[CascadedShadowsConfig::eMaxLevel];
								};

								ICascadedShadows* pCascadedShadows = pDirectionalLight->GetCascadedShadow();
								if (pCascadedShadows == nullptr)
									continue;

								std::shared_ptr<ITexture> pTexture = pCascadedShadows->GetShadowMap();
								ISamplerState* pSamplerState = pCascadedShadows->GetSamplerPCF();

								CascadedShadow cascadedShadow;
								cascadedShadow.n2PCFBlurSize = pCascadedShadows->GetPCFBlurSize();
								cascadedShadow.f2TexelOffset = pCascadedShadows->GetTexelOffset();

								cascadedShadow.nCascadeLevel = pCascadedShadows->GetCascadeLevel();
								cascadedShadow.fDepthBias = pCascadedShadows->GetDepthBias();

								for (uint32_t k = 0; k < pCascadedShadows->GetCascadeLevel(); ++k)
								{
									const Math::Matrix& matView = pCascadedShadows->GetViewMatrix(k);
									const Math::Matrix& matProj = pCascadedShadows->GetProjectionMatrix(k);

									Math::Matrix matViewProj = matView * matProj;

									cascadedShadow.matCascadeViewProj[k] = matViewProj.Transpose();

									const Math::Vector2& f2SplitDepth = pCascadedShadows->GetSplitDepths(k);
									cascadedShadow.f2SplitDpeths[k] = Math::Vector4(f2SplitDepth.x, f2SplitDepth.y, 0.f, 0.f);
								}

								if (pSamplerState == nullptr)
								{
									pSamplerState = pCascadedShadows->GetSamplerPCF();
								}

								m_pEffectShadow->SetTexture(StrID::g_texShadowMap, pTexture);
								m_pEffectShadow->SetRawValue(StrID::g_cascadeShadow, &cascadedShadow, 0, sizeof(CascadedShadow));
								m_pEffectShadow->SetSamplerState(StrID::g_samShadow, pSamplerState, 0);
							}
							break;
							case EmLight::ePoint:
							{
								D3D_PROFILING(pDeviceContext, Deferred_ShadowCubeMap);
								pEffectTech = m_pEffectShadow->GetTechnique(StrID::Deferred_ShadowCubeMap);
								if (pEffectTech == nullptr)
								{
									LOG_ERROR("Not Exist EffectTech !! : %s", StrID::Deferred_ShadowCubeMap.c_str());
									continue;
								}

								nPassCount = pEffectTech->GetPassCount();

								IPointLight* pPointLight = static_cast<IPointLight*>(pLight);

								struct ShadowCubeMap
								{
									Math::Int2 n2PCFBlurSize;
									Math::Vector2 f2TexelOffset;

									float fDepthBias = 0.f;
									Math::Vector3 f3LightPos;

									float fLightIntensity;
									float fFarPlane;
									Math::Vector2 padding;
								};

								IShadowCubeMap* pShadowCubeMap = pPointLight->GetShadowCubeMap();
								if (pShadowCubeMap == nullptr)
									continue;

								const std::shared_ptr<ITexture>& pTexture = pShadowCubeMap->GetShadowMap();

								ShadowCubeMap shadowCubeMap;
								shadowCubeMap.n2PCFBlurSize = pShadowCubeMap->GetPCFBlurSize();
								shadowCubeMap.f2TexelOffset = pShadowCubeMap->GetTexelOffset();

								shadowCubeMap.fDepthBias = pShadowCubeMap->GetDepthBias();
								shadowCubeMap.f3LightPos = pPointLight->GetPosition();

								shadowCubeMap.fLightIntensity = pPointLight->GetIntensity();
								shadowCubeMap.fFarPlane = pShadowCubeMap->GetFarPlane();

								m_pEffectShadow->SetTexture(StrID::g_texShadowCubeMap, pTexture);
								m_pEffectShadow->SetRawValue(StrID::g_shadowCubeMap, &shadowCubeMap, 0, sizeof(ShadowCubeMap));
								m_pEffectShadow->SetSamplerState(StrID::g_samShadow, pShadowCubeMap->GetSamplerPCF(), 0);
								m_pEffectShadow->SetSamplerState("g_samShadow2", pShadowCubeMap->GetSamplerPoint(), 0);
							}
							break;
							case EmLight::eSpot:
							{
								D3D_PROFILING(pDeviceContext, Deferred_ShadowMap);
								pEffectTech = m_pEffectShadow->GetTechnique(StrID::Deferred_ShadowMap);
								if (pEffectTech == nullptr)
								{
									LOG_ERROR("Not Exist EffectTech !! : %s", StrID::Deferred_ShadowMap.c_str());
									continue;
								}

								nPassCount = pEffectTech->GetPassCount();

								ISpotLight* pSpotLight = static_cast<ISpotLight*>(pLight);

								struct ShadowMap
								{
									Math::Int2 n2PCFBlurSize;
									Math::Vector2 f2TexelOffset;

									float fDepthBias = 0.f;
									Math::Vector3 f3LightPos;

									Math::Vector3 f3LightDir;
									float fLightAngle = 0.f;

									float fLightIntensity;
									Math::Vector3 padding;

									Math::Matrix matViewProj;
								};

								IShadowMap* pShadowMap = pSpotLight->GetShadowMap();
								if (pShadowMap == nullptr)
									continue;

								const std::shared_ptr<ITexture>& pTexture = pShadowMap->GetShadowMap();

								ShadowMap shadowMap;
								shadowMap.n2PCFBlurSize = pShadowMap->GetPCFBlurSize();
								shadowMap.f2TexelOffset = pShadowMap->GetTexelOffset();

								shadowMap.fDepthBias = pShadowMap->GetDepthBias();
								shadowMap.f3LightPos = pSpotLight->GetPosition();

								shadowMap.f3LightDir = pSpotLight->GetDirection();
								shadowMap.fLightAngle = pSpotLight->GetAngle();

								shadowMap.fLightIntensity = pSpotLight->GetIntensity();

								Math::Matrix matViewProj = pShadowMap->GetViewMatrix() * pShadowMap->GetProjectionMatrix();
								shadowMap.matViewProj = matViewProj.Transpose();

								m_pEffectShadow->SetTexture(StrID::g_texShadowMap, pTexture);
								m_pEffectShadow->SetRawValue(StrID::g_shadowMap, &shadowMap, 0, sizeof(ShadowMap));
								m_pEffectShadow->SetSamplerState(StrID::g_samShadow, pShadowMap->GetSamplerPCF(), 0);
							}
							break;
							default:
								continue;
							}

							m_pEffectShadow->SetTexture(StrID::g_texDepth, pDevice->GetMainDepthStencil()->GetTexture());

							++nShadowCount;

							if (pEffectTech != nullptr)
							{
								for (uint32_t p = 0; p < nPassCount; ++p)
								{
									pEffectTech->PassApply(p, pDeviceContext);

									pDeviceContext->Draw(4, 0);
								}

								ClearEffect_Shadow(pDeviceContext, pEffectTech);
							}
						}
					}
				}
			}

			return nShadowCount;
		}

		bool DeferredRenderer::Impl::CreateEffect()
		{
			std::string strPath(File::GetPath(File::EmPath::eFx));

#if defined(DEBUG) || defined(_DEBUG)
			strPath.append("Model\\Deferred_D.cso");
#else
			strPath.append("Model\\Deferred.cso");
#endif

			m_pEffect = IEffect::Create(StrID::EffectDeferred, strPath.c_str());
			if (m_pEffect == nullptr)
			{
				LOG_ERROR("Can't Create Effect : %s", strPath.c_str());
				return false;
			}

			IEffectTech* pEffectTech = m_pEffect->CreateTechnique(StrID::Deferred, EmVertexFormat::eUnknown);
			if (pEffectTech == nullptr)
			{
				LOG_ERROR("Not Exist EffectTech !!, %s", StrID::Deferred.c_str());
				return false;
			}

			strPath = File::GetPath(File::EmPath::eFx);

#if defined(DEBUG) || defined(_DEBUG)
			strPath.append("Model\\DeferredShadow_D.cso");
#else
			strPath.append("Model\\DeferredShadow.cso");
#endif

			m_pEffectShadow = IEffect::Create(StrID::EffectDeferredShadow, strPath.c_str());
			if (m_pEffectShadow == nullptr)
			{
				LOG_ERROR("Can't Create Effect : %s", strPath.c_str());
				return false;
			}

			pEffectTech = m_pEffectShadow->CreateTechnique(StrID::Deferred_CascadedShadow, EmVertexFormat::eUnknown);
			if (pEffectTech == nullptr)
			{
				LOG_ERROR("Not Exist EffectTech !!, %s", StrID::Deferred_CascadedShadow.c_str());
				return false;
			}

			pEffectTech = m_pEffectShadow->CreateTechnique(StrID::Deferred_ShadowMap, EmVertexFormat::eUnknown);
			if (pEffectTech == nullptr)
			{
				LOG_ERROR("Not Exist EffectTech !!, %s", StrID::Deferred_ShadowMap.c_str());
				return false;
			}

			pEffectTech = m_pEffectShadow->CreateTechnique(StrID::Deferred_ShadowCubeMap, EmVertexFormat::eUnknown);
			if (pEffectTech == nullptr)
			{
				LOG_ERROR("Not Exist EffectTech !!, %s", StrID::Deferred_ShadowCubeMap.c_str());
				return false;
			}

			return true;
		}

		void DeferredRenderer::Impl::ClearEffect(IDeviceContext* pd3dDeviceContext, IEffectTech* pTech)
		{
			m_pEffect->SetStructuredBuffer(StrID::g_lightDirectional, nullptr);
			m_pEffect->SetStructuredBuffer(StrID::g_lightPoint, nullptr);
			m_pEffect->SetStructuredBuffer(StrID::g_lightSpot, nullptr);

			m_pEffect->SetTexture(StrID::g_texDepth, nullptr);
			m_pEffect->SetTexture(StrID::g_texNormal, nullptr);
			m_pEffect->SetTexture(StrID::g_texAlbedoSpecular, nullptr);
			m_pEffect->SetTexture(StrID::g_texDisneyBRDF, nullptr);

			//m_pEffect->SetTexture(StrID::g_texIBLMap, nullptr);
			m_pEffect->SetTexture(StrID::g_texDiffuseHDR, nullptr);

			m_pEffect->SetTexture(StrID::g_texSpecularHDR, nullptr);
			m_pEffect->SetTexture(StrID::g_texSpecularBRDF, nullptr);

			m_pEffect->SetTexture(StrID::g_texShadowMap, nullptr);

			m_pEffect->ClearState(pd3dDeviceContext, pTech);
		}

		void DeferredRenderer::Impl::ClearEffect_Shadow(IDeviceContext* pd3dDeviceContext, IEffectTech* pTech)
		{
			m_pEffectShadow->SetTexture(StrID::g_texDepth, nullptr);
			m_pEffectShadow->SetTexture(StrID::g_texShadowMap, nullptr);

			m_pEffectShadow->ClearState(pd3dDeviceContext, pTech);
		}

		DeferredRenderer::DeferredRenderer()
			: m_pImpl{ std::make_unique<Impl>() }
		{
		}

		DeferredRenderer::~DeferredRenderer()
		{
		}

		void DeferredRenderer::Render(IDevice* pDevice, IDeviceContext* pDeviceContext, Camera* pCamera, uint32_t nRenderGroupFlag)
		{
			m_pImpl->Render(pDevice, pDeviceContext, pCamera, nRenderGroupFlag);
		}

		void DeferredRenderer::Flush()
		{
			m_pImpl->Flush();
		}
	}
}