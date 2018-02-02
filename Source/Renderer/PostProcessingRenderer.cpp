#include "stdafx.h"
#include "PostProcessingRenderer.h"

#include "GaussianBlur.h"
#include "Downscale.h"
#include "DepthOfField.h"
#include "FXAA.h"
#include "ColorGrading.h"
#include "HDRFilter.h"
#include "ASSAO.h"
#include "SSS.h"
#include "BloomFilter.h"

#include "CommonLib/Timer.h"
#include "CommonLib/Config.h"

#include "DirectX/GBuffers.h"

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
			, m_pSSS(nullptr)
			, m_pBloomFilter(nullptr)
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

			HDRFilter::DestroyInstance();

			SafeRelease(m_pColorGrading);
			ColorGrading::DestroyInstance();

			SafeRelease(m_pASSAO);
			ASSAO::DestroyInstance();

			SafeRelease(m_pSSS);
			SSS::DestroyInstance();

			SafeRelease(m_pBloomFilter);
			BloomFilter::DestroyInstance();
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

			m_pASSAO = ASSAO::GetInstance();
			if (m_pASSAO->Init(viewport) == false)
				return false;

			m_pSSS = SSS::GetInstance();
			if (m_pSSS->Init() == false)
				return false;

			m_pBloomFilter = BloomFilter::GetInstance();
			if (m_pBloomFilter->Init() == false)
				return false;

			return true;
		}

		void PostProcessingRenderer::Render(uint32_t nRenderGroupFlag)
		{
			D3D_PROFILING(PostProcessing);

			IDevice* pDevice = GetDevice();

			if (Config::IsEnable("SSS"_s) == true)
			{
				IRenderTarget* pSSS = pDevice->GetRenderTarget(pDevice->GetMainRenderTarget()->GetDesc2D(), false);
				IRenderTarget* pSource = pDevice->GetLastUseRenderTarget();
				const std::shared_ptr<ITexture>& pDepth = pDevice->GetMainDepthStencil()->GetTexture();
				m_pSSS->Apply(pSSS, pSource, pDepth);

				pDevice->ReleaseRenderTargets(&pSSS);
			}

			if (Config::IsEnable("SSAO"_s) == true)
			{
				IRenderTarget* pRenderTarget = pDevice->GetLastUseRenderTarget();
				m_pASSAO->Apply(pRenderTarget);

				pDevice->ReleaseRenderTargets(&pRenderTarget);
			}

			if (Config::IsEnable("HDRFilter"_s) == true)
			{
				IRenderTarget* pHDR = pDevice->GetRenderTarget(pDevice->GetLastUseRenderTarget()->GetDesc2D(), false);
				IRenderTarget* pSource = pDevice->GetLastUseRenderTarget();
				m_pHDRFilter->Apply(pHDR, pSource);
			
				pDevice->ReleaseRenderTargets(&pHDR);
			}

			if (Config::IsEnable("BloomFilter"_s) == true)
			{
				IRenderTarget* pSource = pDevice->GetLastUseRenderTarget();
				m_pBloomFilter->Apply(pSource);
			}

			if (Config::IsEnable("ColorGrading"_s) == true)
			{
				IRenderTarget* pColorGrading = pDevice->GetRenderTarget(pDevice->GetLastUseRenderTarget()->GetDesc2D(), false);
				IRenderTarget* pSource = pDevice->GetLastUseRenderTarget();
				m_pColorGrading->Apply(pColorGrading, pSource);

				pDevice->ReleaseRenderTargets(&pColorGrading);
			}

			if (Config::IsEnable("DepthOfField"_s) == true)
			{
				IRenderTarget* pDepthOfField = pDevice->GetRenderTarget(pDevice->GetMainRenderTarget()->GetDesc2D(), false);
				IRenderTarget* pSource = pDevice->GetLastUseRenderTarget();
				const std::shared_ptr<ITexture>& pDepth = pDevice->GetMainDepthStencil()->GetTexture();
				m_pDepthOfField->Apply(pDepthOfField, pSource, pDepth);

				pDevice->ReleaseRenderTargets(&pDepthOfField);
			}

			if (Config::IsEnable("FXAA"_s) == true)
			{
				IRenderTarget* pFxaa = pDevice->GetRenderTarget(pDevice->GetMainRenderTarget()->GetDesc2D(), false);
				IRenderTarget* pSource = pDevice->GetLastUseRenderTarget();
				m_pFxaa->Apply(pFxaa, pSource);

				pDevice->ReleaseRenderTargets(&pFxaa);
			}
		}

		void PostProcessingRenderer::Flush()
		{
		}
	}
}