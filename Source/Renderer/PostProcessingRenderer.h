#pragma once

#include "Renderer.h"

namespace EastEngine
{
	namespace Graphics
	{
		class PostProcessingRenderer : public IRenderer
		{
		public:
			PostProcessingRenderer();
			virtual ~PostProcessingRenderer();

		public:
			virtual void Render(IDevice* pDevice, IDeviceContext* pDeviceContext, Camera* pCamera, uint32_t nRenderGroupFlag) override;
			virtual void Flush() override;

		private:
			class Impl;
			std::unique_ptr<Impl> m_pImpl;
		};
	}
}