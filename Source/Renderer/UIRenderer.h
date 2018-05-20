#pragma once

#include "Renderer.h"

namespace eastengine
{
	namespace graphics
	{
		class UIRenderer : public IRenderer
		{
		public:
			UIRenderer();
			virtual ~UIRenderer();

		public:
			virtual void AddRender(const RenderSubsetUIText& renderSubset) override;
			virtual void AddRender(const RenderSubsetUISprite& renderSubset) override;
			virtual void AddRender(const RenderSubsetUIPanel& renderSubset) override;

		public:
			virtual void Render(IDevice* pDevice, IDeviceContext* pDeviceContext, Camera* pCamera, uint32_t nRenderGroupFlag) override;
			virtual void Flush() override;

		private:
			class Impl;
			std::unique_ptr<Impl> m_pImpl;
		};
	}
}