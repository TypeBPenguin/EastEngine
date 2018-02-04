#pragma once

#include "Renderer.h"

namespace EastEngine
{
	namespace Graphics
	{
		class GaussianBlur;
		class Downscale;
		class DepthOfField;
		class FXAA;
		class ASSAO;
		class HDRFilter;
		class ColorGrading;
		class SSS;
		class BloomFilter;

		class PostProcessingRenderer : public IRenderer
		{
		public:
			PostProcessingRenderer();
			virtual ~PostProcessingRenderer();

		public:
			virtual bool Init(const Math::Viewport& viewport) override;

			virtual void Render(IDevice* pDevice, IDeviceContext* pDeviceContext, Camera* pCamera, uint32_t nRenderGroupFlag) override;
			virtual void Flush() override;

		private:
			GaussianBlur* m_pGaussianBlur;
			Downscale* m_pDownscale;
			DepthOfField* m_pDepthOfField;
			FXAA* m_pFxaa;
			HDRFilter*	m_pHDRFilter;
			ColorGrading* m_pColorGrading;
			ASSAO* m_pASSAO;
			SSS* m_pSSS;
			BloomFilter* m_pBloomFilter;
		};
	}
}