#pragma once

#include "Renderer.h"

namespace eastengine
{
	namespace graphics
	{
		class SkyRenderer : public IRenderer
		{
		public:
			SkyRenderer();
			virtual ~SkyRenderer();

		public:
			virtual void AddRender(const RenderSubsetSky& renderSubset) override;
			virtual void AddRender(const RenderSubsetSkybox& renderSubset) override;
			virtual void AddRender(const RenderSubsetSkyEffect& renderSubset) override;
			virtual void AddRender(const RenderSubsetSkyCloud& renderSubset) override;

		public:
			virtual void Render(IDevice* pDevice, IDeviceContext* pDeviceContext, Camera* pCamera, uint32_t nRenderGroupFlag) override;
			virtual void Flush() override;

		private:
			class Impl;
			std::unique_ptr<Impl> m_pImpl;
		};
	}
}