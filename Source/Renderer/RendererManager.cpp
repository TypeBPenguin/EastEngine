#include "stdafx.h"
#include "RendererManager.h"

#include "ModelRenderer.h"
#include "ParticleRenderer.h"
#include "TerrainRenderer.h"
#include "UIRenderer.h"
#include "DeferredRenderer.h"
#include "PostProcessingRenderer.h"
#include "SkyRenderer.h"
#include "WaterRenderer.h"
#include "VertexRenderer.h"

#include "CommonLib/FileUtil.h"
#include "DirectX/Camera.h"

namespace StrID
{
	RegisterStringID(Copy);
	RegisterStringID(EffectCopy);

	RegisterStringID(g_texture);
	RegisterStringID(g_sampler);
}

namespace EastEngine
{
	namespace Graphics
	{
		RendererManager::RendererManager()
			: m_isInit(false)
			, m_pEffect(nullptr)
		{
			m_pRenderer.fill(nullptr);
		}

		RendererManager::~RendererManager()
		{
			Release();
		}

		bool RendererManager::Init(const Math::Viewport& viewport)
		{
			if (m_isInit == true)
				return true;

			m_isInit = true;

			std::string strPath(File::GetPath(File::EmPath::eFx));

#if defined(DEBUG) || defined(_DEBUG)
			strPath.append("Copy_D.cso");
#else
			strPath.append("Copy.cso");
#endif

			m_pEffect = IEffect::Create(StrID::EffectCopy, strPath.c_str());
			if (m_pEffect == nullptr)
				return false;

			m_pEffect->CreateTechnique(StrID::Copy, EmVertexFormat::eUnknown);

			m_pRenderer[EmRenderer::eParticle] = new ParticleRenderer;
			if (m_pRenderer[EmRenderer::eParticle]->Init(viewport) == false)
			{
				Release();
				return false;
			}

			m_pRenderer[EmRenderer::eModel] = new ModelRenderer;
			if (m_pRenderer[EmRenderer::eModel]->Init(viewport) == false)
			{
				Release();
				return false;
			}

			m_pRenderer[EmRenderer::eTerrain] = new TerrainRenderer;
			if (m_pRenderer[EmRenderer::eTerrain]->Init(viewport) == false)
			{
				Release();
				return false;
			}

			m_pRenderer[EmRenderer::eUI] = new UIRenderer;
			if (m_pRenderer[EmRenderer::eUI]->Init(viewport) == false)
			{
				Release();
				return false;
			}

			m_pRenderer[EmRenderer::eDeferred] = new DeferredRenderer;
			if (m_pRenderer[EmRenderer::eDeferred]->Init(viewport) == false)
			{
				Release();
				return false;
			}

			m_pRenderer[EmRenderer::ePostProcessing] = new PostProcessingRenderer;
			if (m_pRenderer[EmRenderer::ePostProcessing]->Init(viewport) == false)
			{
				Release();
				return false;
			}

			m_pRenderer[EmRenderer::eSky] = new SkyRenderer;
			if (m_pRenderer[EmRenderer::eSky]->Init(viewport) == false)
			{
				Release();
				return false;
			}

			m_pRenderer[EmRenderer::eVertex] = new VertexRenderer;
			if (m_pRenderer[EmRenderer::eVertex]->Init(viewport) == false)
			{
				Release();
				return false;
			}

			//m_pRenderer[EmRenderer::eWater] = new WaterRenderer;
			//if (m_pRenderer[EmRenderer::eWater]->Init(viewport) == false)
			//{
			//	Release();
			//	return false;
			//}

			return true;
		}

		void RendererManager::Release()
		{
			if (m_isInit == false)
				return;

			for (auto& pRenderer : m_pRenderer)
			{
				SafeDelete(pRenderer);
			}

			m_isInit = false;
		}

		void RendererManager::Render()
		{
			int nThreadID = GetThreadID(ThreadType::eRender);

			IDevice* pDevice = GetDevice();
			IDeviceContext* pDeviceContext = GetDeferredContext(nThreadID);

			Camera* pCamera = Camera::GetInstance();
			if (pCamera == nullptr)
			{
				LOG_ERROR("Not Exist Main Camera !!");
				return;
			}

			m_pRenderer[EmRenderer::eModel]->Render(pDevice, pDeviceContext, pCamera, ModelRenderer::Group::eDeferred);
			m_pRenderer[EmRenderer::eTerrain]->Render(pDevice, pDeviceContext, pCamera, 0);

			//m_pRenderer[EmRenderer::eParticle]->Render(EmParticleGroup::eDecal);

			m_pRenderer[EmRenderer::eSky]->Render(pDevice, pDeviceContext, pCamera, 0);
			m_pRenderer[EmRenderer::eDeferred]->Render(pDevice, pDeviceContext, pCamera, 0);

			m_pRenderer[EmRenderer::eModel]->Render(pDevice, pDeviceContext, pCamera, ModelRenderer::Group::eAlphaBlend);

			//m_pRenderer[EmRenderer::eWater]->Render(0);

			m_pRenderer[EmRenderer::eVertex]->Render(pDevice, pDeviceContext, pCamera, 0);

			//m_pRenderer[EmRenderer::eParticle]->Render(EmParticleGroup::eEmitter);

			m_pRenderer[EmRenderer::ePostProcessing]->Render(pDevice, pDeviceContext, pCamera, 0);

			//m_pRenderer[EmRenderer::eUI]->Render(pDevice, pDeviceContext, pCamera, 0);

			CopyToMainRenderTarget(pDevice, pDeviceContext);
		}

		void RendererManager::Flush()
		{
			for (auto& pRenderer : m_pRenderer)
			{
				if (pRenderer == nullptr)
					continue;

				pRenderer->Flush();
			}
		}

		void RendererManager::AddRender(const RenderSubsetVertex& renderSubset)
		{
			std::lock_guard<std::mutex> lock(m_pMutex[EmRenderer::eVertex]);
			m_pRenderer[EmRenderer::eVertex]->AddRender(renderSubset);
		}

		void RendererManager::AddRender(const RenderSubsetLine& renderSubset)
		{
			std::lock_guard<std::mutex> lock(m_pMutex[EmRenderer::eVertex]);
			m_pRenderer[EmRenderer::eVertex]->AddRender(renderSubset);
		}

		void RendererManager::AddRender(const RenderSubsetLineSegment& renderSubset)
		{
			std::lock_guard<std::mutex> lock(m_pMutex[EmRenderer::eVertex]);
			m_pRenderer[EmRenderer::eVertex]->AddRender(renderSubset);
		}

		void RendererManager::AddRender(const RenderSubsetStatic& renderSubset)
		{
			std::lock_guard<std::mutex> lock(m_pMutex[EmRenderer::eModel]);
			m_pRenderer[EmRenderer::eModel]->AddRender(renderSubset);
		}

		void RendererManager::AddRender(const RenderSubsetSkinned& renderSubset)
		{
			std::lock_guard<std::mutex> lock(m_pMutex[EmRenderer::eModel]);
			m_pRenderer[EmRenderer::eModel]->AddRender(renderSubset);
		}

		void RendererManager::AddRender(const RenderSubsetTerrain& renderSubset)
		{
			std::lock_guard<std::mutex> lock(m_pMutex[EmRenderer::eTerrain]);
			m_pRenderer[EmRenderer::eTerrain]->AddRender(renderSubset);
		}

		void RendererManager::AddRender(const RenderSubsetSky& renderSubset)
		{
			std::lock_guard<std::mutex> lock(m_pMutex[EmRenderer::eSky]);
			m_pRenderer[EmRenderer::eSky]->AddRender(renderSubset);
		}

		void RendererManager::AddRender(const RenderSubsetSkybox& renderSubset)
		{
			std::lock_guard<std::mutex> lock(m_pMutex[EmRenderer::eSky]);
			m_pRenderer[EmRenderer::eSky]->AddRender(renderSubset);
		}

		void RendererManager::AddRender(const RenderSubsetSkyEffect& renderSubset)
		{
			std::lock_guard<std::mutex> lock(m_pMutex[EmRenderer::eSky]);
			m_pRenderer[EmRenderer::eSky]->AddRender(renderSubset);
		}

		void RendererManager::AddRender(const RenderSubsetSkyCloud& renderSubset)
		{
			std::lock_guard<std::mutex> lock(m_pMutex[EmRenderer::eSky]);
			m_pRenderer[EmRenderer::eSky]->AddRender(renderSubset);
		}

		void RendererManager::AddRender(const RenderSubsetParticleEmitter& renderSubset)
		{
			std::lock_guard<std::mutex> lock(m_pMutex[EmRenderer::eParticle]);
			m_pRenderer[EmRenderer::eParticle]->AddRender(renderSubset);
		}

		void RendererManager::AddRender(const RenderSubsetParticleDecal& renderSubset)
		{
			std::lock_guard<std::mutex> lock(m_pMutex[EmRenderer::eParticle]);
			m_pRenderer[EmRenderer::eParticle]->AddRender(renderSubset);
		}

		void RendererManager::AddRender(const RenderSubsetUIText& renderSubset)
		{
			std::lock_guard<std::mutex> lock(m_pMutex[EmRenderer::eUI]);
			m_pRenderer[EmRenderer::eUI]->AddRender(renderSubset);
		}

		void RendererManager::AddRender(const RenderSubsetUISprite& renderSubset)
		{
			std::lock_guard<std::mutex> lock(m_pMutex[EmRenderer::eUI]);
			m_pRenderer[EmRenderer::eUI]->AddRender(renderSubset);
		}

		void RendererManager::AddRender(const RenderSubsetUIPanel& renderSubset)
		{
			std::lock_guard<std::mutex> lock(m_pMutex[EmRenderer::eUI]);
			m_pRenderer[EmRenderer::eUI]->AddRender(renderSubset);
		}

		void RendererManager::CopyToMainRenderTarget(IDevice* pDevice, IDeviceContext* pDeviceContext)
		{
			D3D_PROFILING(pDeviceContext, CopyToMainRenderTarget);

			IRenderTarget* pSource = pDevice->GetLastUseRenderTarget();
			if (pSource == nullptr)
				return;

			IEffectTech* pEffectTech = m_pEffect->GetTechnique(StrID::Copy);
			if (pEffectTech == nullptr)
			{
				LOG_ERROR("Not Exist EffectTech !!");
				return;
			}

			pDeviceContext->ClearState();
			pDeviceContext->SetDefaultViewport();

			pDeviceContext->SetDepthStencilState(EmDepthStencilState::eRead_Write_Off);
			pDeviceContext->SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

			IRenderTarget* pRenderTarget = pDevice->GetMainRenderTarget();
			pDeviceContext->SetRenderTargets(&pRenderTarget, 1, nullptr);

			m_pEffect->SetTexture(StrID::g_texture, pSource->GetTexture());
			m_pEffect->SetSamplerState(StrID::g_sampler, pDevice->GetSamplerState(EmSamplerState::eMinMagMipPointClamp), 0);

			uint32_t nPassCount = pEffectTech->GetPassCount();
			for (uint32_t p = 0; p < nPassCount; ++p)
			{
				pEffectTech->PassApply(p, pDeviceContext);

				pDeviceContext->Draw(4, 0);
			}

			ClearEffect(pDeviceContext, pEffectTech);

			pDevice->ReleaseRenderTargets(&pRenderTarget);
		}

		void RendererManager::ClearEffect(IDeviceContext* pd3dDeviceContext, IEffectTech* pEffectTech)
		{
			m_pEffect->SetTexture(StrID::g_texture, nullptr);
			m_pEffect->UndoSamplerState(StrID::g_sampler, 0);

			m_pEffect->ClearState(pd3dDeviceContext, pEffectTech);
		}
	}
}