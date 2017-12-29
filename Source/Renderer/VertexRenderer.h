#pragma once

#include "Renderer.h"

namespace EastEngine
{
	namespace Graphics
	{
		class IEffect;
		class IEffectTEch;

		class IVertexBuffer;
		class IIndexBuffer;

		class VertexRenderer : public IRenderer
		{
		public:
			VertexRenderer();
			virtual ~VertexRenderer();

		public:
			virtual bool Init(const Math::Viewport& viewport) override;

			virtual void AddRender(const RenderSubsetVertex& renderSubset) override { m_vecVertexSubset.emplace_back(renderSubset); }
			virtual void AddRender(const RenderSubsetLine& renderSubset) override { m_vecLineSubset.emplace_back(renderSubset); }
			virtual void AddRender(const RenderSubsetLineSegment& renderSubset) override { m_vecLineSegmentSubset.emplace_back(renderSubset); }

			virtual void Render(uint32_t nRenderGroupFlag) override;
			virtual void Flush() override;

		private:
			void SetRenderState(IDevice* pDevice, IDeviceContext* pDeviceContext);
			void ClearEffect(IDeviceContext* pDeviceContext, IEffectTech* pTech);

			void RenderVertex(IDevice* pDevice, IDeviceContext* pDeviceContext);
			void RenderLine(IDevice* pDevice, IDeviceContext* pDeviceContext);
			void RenderLineSegment(IDevice* pDevice, IDeviceContext* pDeviceContext);

		private:
			IEffect* m_pEffect;

			std::vector<RenderSubsetVertex> m_vecVertexSubset;
			std::vector<RenderSubsetLine> m_vecLineSubset;
			std::vector<RenderSubsetLineSegment> m_vecLineSegmentSubset;

			IVertexBuffer* m_pLineSegmentVertexBuffer;
			IIndexBuffer* m_pLineSegmentIndexBuffer;
		};
	}
}