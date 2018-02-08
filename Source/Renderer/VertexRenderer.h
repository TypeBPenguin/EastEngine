#pragma once

#include "Renderer.h"

namespace EastEngine
{
	namespace Graphics
	{
		class VertexRenderer : public IRenderer
		{
		public:
			VertexRenderer();
			virtual ~VertexRenderer();

		public:
			virtual void AddRender(const RenderSubsetVertex& renderSubset) override;
			virtual void AddRender(const RenderSubsetLine& renderSubset) override;
			virtual void AddRender(const RenderSubsetLineSegment& renderSubset) override;

		public:
			virtual void Render(IDevice* pDevice, IDeviceContext* pDeviceContext, Camera* pCamera, uint32_t nRenderGroupFlag) override;
			virtual void Flush() override;

		private:
			class Impl;
			std::unique_ptr<Impl> m_pImpl;
		};
	}
}