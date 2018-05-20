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

namespace eastengine
{
	namespace graphics
	{
		class PostProcessingRenderer::Impl
		{
		public:
			Impl();
			~Impl();

		public:
			void Render(IDevice* pDevice, IDeviceContext* pDeviceContext, Camera* pCamera, uint32_t nRenderGroupFlag);
			void Flush();

		private:
			GaussianBlur* m_pGaussianBlur{ nullptr };
			Downscale* m_pDownscale{ nullptr };
			DepthOfField* m_pDepthOfField{ nullptr };
			FXAA* m_pFxaa{ nullptr };
			HDRFilter*	m_pHDRFilter{ nullptr };
			ColorGrading* m_pColorGrading{ nullptr };
			ASSAO* m_pASSAO{ nullptr };
			SSS* m_pSSS{ nullptr };
			BloomFilter* m_pBloomFilter{ nullptr };
		};

		PostProcessingRenderer::Impl::Impl()
		{
			m_pGaussianBlur = GaussianBlur::GetInstance();
			if (m_pGaussianBlur->Init() == false)
			{
				assert(false);
				return;
			}

			m_pDownscale = Downscale::GetInstance();
			if (m_pDownscale->Init() == false)
			{
				assert(false);
				return;
			}

			m_pDepthOfField = DepthOfField::GetInstance();
			if (m_pDepthOfField->Init() == false)
			{
				assert(false);
				return;
			}

			m_pFxaa = FXAA::GetInstance();
			if (m_pFxaa->Init() == false)
			{
				assert(false);
				return;
			}

			m_pColorGrading = ColorGrading::GetInstance();
			if (m_pColorGrading->Init() == false)
			{
				assert(false);
				return;
			}

			m_pHDRFilter = HDRFilter::GetInstance();

			m_pASSAO = ASSAO::GetInstance();
			if (m_pASSAO->Init() == false)
			{
				assert(false);
				return;
			}

			m_pSSS = SSS::GetInstance();
			if (m_pSSS->Init() == false)
			{
				assert(false);
				return;
			}

			m_pBloomFilter = BloomFilter::GetInstance();
			if (m_pBloomFilter->Init() == false)
			{
				assert(false);
				return;
			}
		}

		PostProcessingRenderer::Impl::~Impl()
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

		void PostProcessingRenderer::Impl::Render(IDevice* pDevice, IDeviceContext* pDeviceContext, Camera* pCamera, uint32_t nRenderGroupFlag)
		{
			TRACER_EVENT("PostProcessingRenderer::Render");
			D3D_PROFILING(pDeviceContext, PostProcessing);

			if (Config::IsEnable("SSS"_s) == true)
			{
				IRenderTarget* pSSS = pDevice->GetRenderTarget(pDevice->GetMainRenderTarget()->GetDesc2D(), false);
				IRenderTarget* pSource = pDevice->GetLastUseRenderTarget();
				const std::shared_ptr<ITexture>& pDepth = pDevice->GetMainDepthStencil()->GetTexture();
				m_pSSS->Apply(pDevice, pDeviceContext, pSSS, pSource, pDepth);

				pDevice->ReleaseRenderTargets(&pSSS);
			}

			if (Config::IsEnable("SSAO"_s) == true)
			{
				IRenderTarget* pRenderTarget = pDevice->GetLastUseRenderTarget();
				m_pASSAO->Apply(pDevice, pDeviceContext, pCamera, pRenderTarget);

				pDevice->ReleaseRenderTargets(&pRenderTarget);
			}

			if (Config::IsEnable("HDRFilter"_s) == true)
			{
				IRenderTarget* pHDR = pDevice->GetRenderTarget(pDevice->GetLastUseRenderTarget()->GetDesc2D(), false);
				IRenderTarget* pSource = pDevice->GetLastUseRenderTarget();
				m_pHDRFilter->Apply(pDevice, pDeviceContext, pHDR, pSource);

				pDevice->ReleaseRenderTargets(&pHDR);
			}

			if (Config::IsEnable("BloomFilter"_s) == true)
			{
				IRenderTarget* pSource = pDevice->GetLastUseRenderTarget();
				m_pBloomFilter->Apply(pDevice, pDeviceContext, pSource);
			}

			if (Config::IsEnable("ColorGrading"_s) == true)
			{
				IRenderTarget* pColorGrading = pDevice->GetRenderTarget(pDevice->GetLastUseRenderTarget()->GetDesc2D(), false);
				IRenderTarget* pSource = pDevice->GetLastUseRenderTarget();
				m_pColorGrading->Apply(pDevice, pDeviceContext, pColorGrading, pSource);

				pDevice->ReleaseRenderTargets(&pColorGrading);
			}

			if (Config::IsEnable("DepthOfField"_s) == true)
			{
				IRenderTarget* pDepthOfField = pDevice->GetRenderTarget(pDevice->GetMainRenderTarget()->GetDesc2D(), false);
				IRenderTarget* pSource = pDevice->GetLastUseRenderTarget();
				const std::shared_ptr<ITexture>& pDepth = pDevice->GetMainDepthStencil()->GetTexture();
				m_pDepthOfField->Apply(pDevice, pDeviceContext, pCamera, pDepthOfField, pSource, pDepth);

				pDevice->ReleaseRenderTargets(&pDepthOfField);
			}

			if (Config::IsEnable("FXAA"_s) == true)
			{
				IRenderTarget* pFxaa = pDevice->GetRenderTarget(pDevice->GetMainRenderTarget()->GetDesc2D(), false);
				IRenderTarget* pSource = pDevice->GetLastUseRenderTarget();
				m_pFxaa->Apply(pDevice, pDeviceContext, pFxaa, pSource);

				pDevice->ReleaseRenderTargets(&pFxaa);
			}
		}

		void PostProcessingRenderer::Impl::Flush()
		{
		}

		PostProcessingRenderer::PostProcessingRenderer()
			: m_pImpl{ std::make_unique<Impl>() }
		{
		}

		PostProcessingRenderer::~PostProcessingRenderer()
		{
		}

		void PostProcessingRenderer::Render(IDevice* pDevice, IDeviceContext* pDeviceContext, Camera* pCamera, uint32_t nRenderGroupFlag)
		{
			m_pImpl->Render(pDevice, pDeviceContext, pCamera, nRenderGroupFlag);
		}

		void PostProcessingRenderer::Flush()
		{
			m_pImpl->Flush();
		}
	}
}