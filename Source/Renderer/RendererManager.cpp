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

			m_pEffect->CreateTechnique(StrID::Copy, EmVertexFormat::ePos);

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
			m_pRenderer[EmRenderer::eModel]->Render(0);
			//m_pRenderer[EmRenderer::eModelShadow]->Render(0);
			m_pRenderer[EmRenderer::eTerrain]->Render(0);

			//m_pRenderer[EmRenderer::eParticle]->Render(EmParticleGroup::eDecal);

			m_pRenderer[EmRenderer::eDeferred]->Render(0);

			m_pRenderer[EmRenderer::eVertex]->Render(0);

			//m_pRenderer[EmRenderer::eWater]->Render(0);

			m_pRenderer[EmRenderer::eSky]->Render(0);

			//m_pRenderer[EmRenderer::eParticle]->Render(EmParticleGroup::eEmitter);

			m_pRenderer[EmRenderer::ePostProcessing]->Render(0);

			m_pRenderer[EmRenderer::eUI]->Render(0);

			CopyToMainRenderTarget();
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

		void RendererManager::CopyToMainRenderTarget()
		{
			D3D_PROFILING(CopyToMainRenderTarget);
			IDevice* pDevice = GetDevice();
			IDeviceContext* pDeviceContext = GetDeviceContext();

			IRenderTarget* pSource = pDevice->GetLastUseRenderTarget();
			if (pSource == nullptr)
				return;

			IEffectTech* pEffectTech = m_pEffect->GetTechnique(StrID::Copy);
			if (pEffectTech == nullptr)
			{
				PRINT_LOG("Not Exist EffectTech !!");
				return;
			}

			if (pDeviceContext->SetInputLayout(EmVertexFormat::ePos) == false)
				return;

			pDeviceContext->ClearState();
			pDeviceContext->SetDefaultViewport();

			pDeviceContext->SetDepthStencilState(EmDepthStencilState::eOff);
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