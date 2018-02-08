#pragma once

#include "Renderer.h"

namespace EastEngine
{
	namespace Graphics
	{
		class DeferredRenderer : public IRenderer
		{
		public:
			DeferredRenderer();
			virtual ~DeferredRenderer();

		public:
			virtual void Render(IDevice* pDevice, IDeviceContext* pDeviceContext, Camera* pCamera, uint32_t nRenderGroupFlag) override;
			virtual void Flush() override;

		private:
			class Impl;
			std::unique_ptr<Impl> m_pImpl;
		};
	}
}