#include "stdafx.h"
#include "PostProcessingRenderer.h"
#include "GaussianBlur.h"
#include "Downscale.h"
#include "DepthOfField.h"
#include "FXAA.h"
#include "ColorGrading.h"
#include "HDRFilter.h"
#include "ASSAO.h"

#include "../CommonLib/Timer.h"
#include "../CommonLib/Config.h"

#include "../DirectX/GBuffers.h"

namespace EastEngine
{
	namespace Graphics
	{
		PostProcessingRenderer::PostProcessingRenderer()
			: m_pGaussianBlur(nullptr)
			, m_pDownscale(nullptr)
			, m_pDepthOfField(nullptr)
			, m_pFxaa(nullptr)
			, m_pColorGrading(nullptr)
			, m_pASSAO(nullptr)
		{
		}

		PostProcessingRenderer::~PostProcessingRenderer()
		{
			SafeRelease(m_pGaussianBlur);
			GaussianBlur::DestroyInstance();

			SafeRelease(m_pDownscale);
			Downscale::DestroyInstance();

			SafeRelease(m_pDepthOfField);
			DepthOfField::DestroyInstance();

			SafeRelease(m_pFxaa);
			FXAA::DestroyInstance();

			SafeRelease(m_pHDRFilter);
			HDRFilter::DestroyInstance();

			SafeRelease(m_pColorGrading);
			ColorGrading::DestroyInstance();

			SafeRelease(m_pASSAO);
			ASSAO::DestroyInstance();
		}

		bool PostProcessingRenderer::Init(const Math::Viewport& viewport)
		{
			m_pGaussianBlur = GaussianBlur::GetInstance();
			if (m_pGaussianBlur->Init() == false)
				return false;

			m_pDownscale = Downscale::GetInstance();
			if (m_pDownscale->Init() == false)
				return false;

			m_pDepthOfField = DepthOfField::GetInstance();
			if (m_pDepthOfField->Init() == false)
				return false;

			m_pFxaa = FXAA::GetInstance();
			if (m_pFxaa->Init() == false)
				return false;

			m_pColorGrading = ColorGrading::GetInstance();
			if (m_pColorGrading->Init() == false)
				return false;

			m_pHDRFilter = HDRFilter::GetInstance();
			if (m_pHDRFilter->Init() == false)
				return false;

			//m_pASSAO = ASSAO::GetInstance();
			//if (m_pASSAO->Init(viewport) == false)
			//	return false;

			return true;
		}

		void PostProcessingRenderer::Render(uint32_t nRenderGroupFlag)
		{
			D3D_PROFILING(PostProcessing);

			IDevice* pDevice = GetDevice();

			if (Config::IsEnableSSAO() == true)
			{
				IRenderTarget* pRenderTarget = pDevice->GetRenderTarget(pDevice->GetLastUseRenderTarget()->GetDesc2D());
				m_pASSAO->Apply(pRenderTarget);

				pDevice->ReleaseRenderTargets(&pRenderTarget);
			}

			if (Config::IsEnableHDRFilter() == true)
			{
				float fElapsedTime = Timer::GetInstance()->GetDeltaTime();
				IRenderTarget* pHDR = pDevice->GetRenderTarget(pDevice->GetLastUseRenderTarget()->GetDesc2D(), false);
				IRenderTarget* pSource = pDevice->GetLastUseRenderTarget();
				m_pHDRFilter->ToneMap(pHDR, pSource, fElapsedTime);
			
				pDevice->ReleaseRenderTargets(&pHDR);
			}

			if (Config::IsEnableDepthOfField() == true)
			{
				IRenderTarget* pDepthOfField = pDevice->GetRenderTarget(pDevice->GetMainRenderTarget()->GetDesc2D(), false);
				IRenderTarget* pSource = pDevice->GetLastUseRenderTarget();
				const std::shared_ptr<ITexture>& pDepth = pDevice->GetMainDepthStencil()->GetTexture();
				m_pDepthOfField->Apply(pDepthOfField, pSource, pDepth);

				pDevice->ReleaseRenderTargets(&pDepthOfField);
			}

			if (Config::IsEnableFXAA() == true)
			{
				IRenderTarget* pFxaa = pDevice->GetRenderTarget(pDevice->GetMainRenderTarget()->GetDesc2D(), false);
				IRenderTarget* pSource = pDevice->GetLastUseRenderTarget();
				m_pFxaa->Apply(pFxaa, pSource);

				pDevice->ReleaseRenderTargets(&pFxaa);
			}
		}

		void PostProcessingRenderer::Flush()
		{
			m_pColorGrading->Flush();
			m_pFxaa->Flush();
		}
	}
}