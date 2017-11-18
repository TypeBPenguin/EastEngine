#include "stdafx.h"
#include "DeferredRenderer.h"

#include "CommonLib/FileUtil.h"
#include "CommonLib/Config.h"

#include "DirectX/CameraManager.h"

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

	RegisterStringID(g_texDepth);
	RegisterStringID(g_texNormal);
	RegisterStringID(g_texAlbedoSpecular);
	RegisterStringID(g_texDisneyBRDF);
	RegisterStringID(g_texShadowMap);
	RegisterStringID(g_texIBLMap);
	RegisterStringID(g_texIrradianceMap);
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
	RegisterStringID(g_samPoint);
	RegisterStringID(g_samLinearWrap);

	RegisterStringID(g_samShadow);
	RegisterStringID(g_cascadeShadow);
	RegisterStringID(g_shadowMap);
}

namespace EastEngine
{
	namespace Graphics
	{
		DeferredRenderer::DeferredRenderer()
			: m_pSamplerLinearWrap(nullptr)
			, m_pSamplerPointClamp(nullptr)
			, m_pSamplerComparison(nullptr)
			, m_pBlendStateAdditive(nullptr)
			, m_pEffect(nullptr)
			, m_pEffectShadow(nullptr)
		{
		}

		DeferredRenderer::~DeferredRenderer()
		{
			IEffect::Destroy(&m_pEffect);
			IEffect::Destroy(&m_pEffectShadow);
		}

		bool DeferredRenderer::Init(const Math::Viewport& viewport)
		{
			m_pSamplerLinearWrap = GetDevice()->GetSamplerState(EmSamplerState::eMinMagMipLinearWrap);
			m_pSamplerPointClamp = GetDevice()->GetSamplerState(EmSamplerState::eMinMagMipPointClamp);

			SamplerStateDesc SamDescShad;
			SamDescShad.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
			SamDescShad.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
			SamDescShad.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
			SamDescShad.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
			SamDescShad.MipLODBias = 0;
			SamDescShad.MaxAnisotropy = 0;
			SamDescShad.ComparisonFunc = D3D11_COMPARISON_LESS;
			Memory::Clear(&SamDescShad.BorderColor, sizeof(SamDescShad.BorderColor));
			SamDescShad.MinLOD = 0;
			SamDescShad.MaxLOD = 0;

			m_pSamplerComparison = ISamplerState::Create(SamDescShad);

			BlendStateDesc blendStateDesc;
			for (int i = 0; i < 8; ++i)
			{
				//blendStateDesc.RenderTarget[i].BlendEnable = true;
				//blendStateDesc.RenderTarget[i].SrcBlend = D3D11_BLEND_SRC1_COLOR;
				//blendStateDesc.RenderTarget[i].DestBlend = D3D11_BLEND_ONE;
				//blendStateDesc.RenderTarget[i].BlendOp = D3D11_BLEND_OP_ADD;
				//blendStateDesc.RenderTarget[i].SrcBlendAlpha = D3D11_BLEND_ONE;
				//blendStateDesc.RenderTarget[i].DestBlendAlpha = D3D11_BLEND_ONE;
				//blendStateDesc.RenderTarget[i].BlendOpAlpha = D3D11_BLEND_OP_ADD;
				//blendStateDesc.RenderTarget[i].RenderTargetWriteMask = 0x0f;

				blendStateDesc.RenderTarget[i].BlendEnable = true;
				blendStateDesc.RenderTarget[i].SrcBlend = D3D11_BLEND_ONE;
				blendStateDesc.RenderTarget[i].DestBlend = D3D11_BLEND_ONE;
				blendStateDesc.RenderTarget[i].BlendOp = D3D11_BLEND_OP_ADD;
				blendStateDesc.RenderTarget[i].SrcBlendAlpha = D3D11_BLEND_ONE;
				blendStateDesc.RenderTarget[i].DestBlendAlpha = D3D11_BLEND_ONE;
				blendStateDesc.RenderTarget[i].BlendOpAlpha = D3D11_BLEND_OP_ADD;
				blendStateDesc.RenderTarget[i].RenderTargetWriteMask = 0x0f;
			}
			m_pBlendStateAdditive = IBlendState::Create(blendStateDesc);

			std::string strPath(File::GetPath(File::EmPath::eFx));

#if defined(DEBUG) || defined(_DEBUG)
			strPath.append("Model\\Deferred_D.cso");
#else
			strPath.append("Model\\Deferred.cso");
#endif

			m_pEffect = IEffect::Create(StrID::EffectDeferred, strPath.c_str());
			if (m_pEffect == nullptr)
			{
				PRINT_LOG("Can't Create Effect : %s", strPath.c_str());
				return false;
			}

			IEffectTech* pEffectTech = m_pEffect->CreateTechnique(StrID::Deferred, EmVertexFormat::ePos);
			if (pEffectTech == nullptr)
			{
				PRINT_LOG("Not Exist EffectTech !!, %s", StrID::Deferred.c_str());
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
				PRINT_LOG("Can't Create Effect : %s", strPath.c_str());
				return false;
			}

			pEffectTech = m_pEffectShadow->CreateTechnique(StrID::Deferred_CascadedShadow, EmVertexFormat::ePos);
			if (pEffectTech == nullptr)
			{
				PRINT_LOG("Not Exist EffectTech !!, %s", StrID::Deferred_CascadedShadow.c_str());
				return false;
			}

			pEffectTech = m_pEffectShadow->CreateTechnique(StrID::Deferred_ShadowMap, EmVertexFormat::ePos);
			if (pEffectTech == nullptr)
			{
				PRINT_LOG("Not Exist EffectTech !!, %s", StrID::Deferred_ShadowMap.c_str());
				return false;
			}

			return true;
		}

		void DeferredRenderer::Render(uint32_t nRenderGroupFlag)
		{
			Camera* pCamera = CameraManager::GetInstance()->GetMainCamera();
			if (pCamera == nullptr)
			{
				PRINT_LOG("Not Exist Main Camera !!");
				return;
			}

			IDevice* pDevice = GetDevice();
			IDeviceContext* pDeviceContext = GetDeviceContext();

			int nEnableShadowCount = 0;
			IRenderTarget* pRenderTargetShadow = nullptr;
			if (Config::IsEnableShadow() == true)
			{
				auto desc = pDevice->GetMainRenderTarget()->GetDesc2D();
				desc.Format = DXGI_FORMAT_R16G16_FLOAT;
				//desc.Format = DXGI_FORMAT_R32_FLOAT;
				desc.Build();

				pRenderTargetShadow = pDevice->GetRenderTarget(desc);

				nEnableShadowCount = RenderShadowMap(pDevice, pDeviceContext, pRenderTargetShadow);

				/*if (nEnableShadowCount > 0)
				{
					IRenderTarget* pFxaa = pDevice->GetRenderTarget(pRenderTargetShadow->GetDesc2D(), false);
					IRenderTarget* pSource = pRenderTargetShadow;
					FXAA::GetInstance()->Apply(pFxaa, pSource);

					pDevice->ReleaseRenderTargets(&pSource);

					pRenderTargetShadow = pFxaa;
				}*/
			}

			if (pDeviceContext->SetInputLayout(EmVertexFormat::ePos) == false)
			{
				pDevice->ReleaseRenderTargets(&pRenderTargetShadow, 1, false);
				return;
			}

			IGBuffers* pGBuffers = GetGBuffers();
			IImageBasedLight* pIBL = GetImageBasedLight();

			pDeviceContext->ClearState();

			pDeviceContext->SetDefaultViewport();
			pDeviceContext->SetBlendState(EmBlendState::eOff);
			pDeviceContext->SetRasterizerState(EmRasterizerState::eCCW);
			pDeviceContext->SetDepthStencilState(EmDepthStencilState::eOff);

			pDeviceContext->SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

			{
				D3D_PROFILING(DeferredRendering);
				IEffectTech* pEffectTech = m_pEffect->GetTechnique(StrID::Deferred);
				if (pEffectTech == nullptr)
				{
					PRINT_LOG("Not Exist EffectTech !! : %s", StrID::Deferred.c_str());
					pDevice->ReleaseRenderTargets(&pRenderTargetShadow, 1, false);
					return;
				}

				IRenderTarget* pRenderTarget = nullptr;
				if (Config::IsEnableHDRFilter() == true)
				{
					auto desc = pDevice->GetMainRenderTarget()->GetDesc2D();
					if (Config::IsEnableHDRFilter() == true)
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

				pDeviceContext->ClearRenderTargetView(pRenderTarget, Math::Color::Black);
				pDeviceContext->SetRenderTargets(&pRenderTarget, 1, nullptr);

				m_pEffect->SetVector(StrID::g_f3CameraPos, pCamera->GetPosition());

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

				m_pEffect->SetMatrix(StrID::g_matInvView, pCamera->GetViewMatrix().Invert());
				m_pEffect->SetMatrix(StrID::g_matInvProj, pCamera->GetProjMatrix().Invert());

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

				m_pEffect->SetTexture(StrID::g_texIBLMap, pIBL->GetCubeMap());
				m_pEffect->SetTexture(StrID::g_texIrradianceMap, pIBL->GetIrradianceMap());

				m_pEffect->SetSamplerState(StrID::g_samPoint, m_pSamplerPointClamp, 0);
				m_pEffect->SetSamplerState(StrID::g_samLinearWrap, m_pSamplerLinearWrap, 0);

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

		void DeferredRenderer::Flush()
		{
		}

		int DeferredRenderer::RenderShadowMap(IDevice* pDevice, IDeviceContext* pDeviceContext, IRenderTarget* pRenderTarget)
		{
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

			Camera* pCamera = CameraManager::GetInstance()->GetMainCamera();
			if (pCamera == nullptr)
			{
				PRINT_LOG("Not Exist Main Camera !!");
				return 0;
			}

			if (pDeviceContext->SetInputLayout(EmVertexFormat::ePos) == false)
				return 0;

			pDeviceContext->ClearState();

			pDeviceContext->SetDefaultViewport();
			pDeviceContext->SetBlendState(m_pBlendStateAdditive);
			//pDeviceContext->SetBlendState(EmBlendState::eMultiplicative);
			pDeviceContext->SetRasterizerState(EmRasterizerState::eCCW);
			pDeviceContext->SetDepthStencilState(EmDepthStencilState::eOff);

			pDeviceContext->SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

			int nShadowCount = 0;
			{
				D3D_PROFILING(DeferredShdaow);

				pDeviceContext->SetRenderTargets(&pRenderTarget, 1, nullptr);

				m_pEffectShadow->SetVector(StrID::g_f3CameraPos, pCamera->GetPosition());

				m_pEffectShadow->SetMatrix(StrID::g_matInvView, pCamera->GetViewMatrix().Invert());
				m_pEffectShadow->SetMatrix(StrID::g_matInvProj, pCamera->GetProjMatrix().Invert());

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
								D3D_PROFILING(Deferred_CascadedShadow);
								pEffectTech = m_pEffectShadow->GetTechnique(StrID::Deferred_CascadedShadow);
								if (pEffectTech == nullptr)
								{
									PRINT_LOG("Not Exist EffectTech !! : %s", StrID::Deferred_CascadedShadow.c_str());
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
							}
							break;
							case EmLight::eSpot:
							{
								D3D_PROFILING(Deferred_ShadowMap);
								pEffectTech = m_pEffectShadow->GetTechnique(StrID::Deferred_ShadowMap);
								if (pEffectTech == nullptr)
								{
									PRINT_LOG("Not Exist EffectTech !! : %s", StrID::Deferred_ShadowMap.c_str());
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

							m_pEffectShadow->SetSamplerState(StrID::g_samPoint, m_pSamplerPointClamp, 0);

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

		void DeferredRenderer::ClearEffect(IDeviceContext* pd3dDeviceContext, IEffectTech* pTech)
		{
			m_pEffect->SetStructuredBuffer(StrID::g_lightDirectional, nullptr);
			m_pEffect->SetStructuredBuffer(StrID::g_lightPoint, nullptr);
			m_pEffect->SetStructuredBuffer(StrID::g_lightSpot, nullptr);

			m_pEffect->SetTexture(StrID::g_texDepth, nullptr);
			m_pEffect->SetTexture(StrID::g_texNormal, nullptr);
			m_pEffect->SetTexture(StrID::g_texAlbedoSpecular, nullptr);
			m_pEffect->SetTexture(StrID::g_texDisneyBRDF, nullptr);

			m_pEffect->SetTexture(StrID::g_texIBLMap, nullptr);
			m_pEffect->SetTexture(StrID::g_texIrradianceMap, nullptr);

			m_pEffect->SetTexture(StrID::g_texShadowMap, nullptr);

			m_pEffect->UndoSamplerState(StrID::g_samPoint, 0);
			m_pEffect->UndoSamplerState(StrID::g_samLinearWrap, 0);

			m_pEffect->ClearState(pd3dDeviceContext, pTech);
		}

		void DeferredRenderer::ClearEffect_Shadow(IDeviceContext* pd3dDeviceContext, IEffectTech* pTech)
		{
			m_pEffectShadow->SetTexture(StrID::g_texDepth, nullptr);
			m_pEffectShadow->SetTexture(StrID::g_texShadowMap, nullptr);

			m_pEffectShadow->UndoSamplerState(StrID::g_samPoint, 0);
			m_pEffectShadow->UndoSamplerState(StrID::g_samShadow, 0);

			m_pEffectShadow->ClearState(pd3dDeviceContext, pTech);
		}
	}
}