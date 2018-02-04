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

			virtual void AddRender(const RenderSubsetVertex& renderSubset) override { m_vecVertexSubset[GetThreadID(ThreadType::eUpdate)].emplace_back(renderSubset); }
			virtual void AddRender(const RenderSubsetLine& renderSubset) override { m_vecLineSubset[GetThreadID(ThreadType::eUpdate)].emplace_back(renderSubset); }
			virtual void AddRender(const RenderSubsetLineSegment& renderSubset) override { m_vecLineSegmentSubset[GetThreadID(ThreadType::eUpdate)].emplace_back(renderSubset); }

			virtual void Render(IDevice* pDevice, IDeviceContext* pDeviceContext, Camera* pCamera, uint32_t nRenderGroupFlag) override;
			virtual void Flush() override;

		private:
			void SetRenderState(IDevice* pDevice, IDeviceContext* pDeviceContext);
			void ClearEffect(IDeviceContext* pDeviceContext, IEffectTech* pTech);

			void RenderVertex(IDevice* pDevice, IDeviceContext* pDeviceContext, Camera* pCamera);
			void RenderLine(IDevice* pDevice, IDeviceContext* pDeviceContext, Camera* pCamera);
			void RenderLineSegment(IDevice* pDevice, IDeviceContext* pDeviceContext, Camera* pCamera);

		private:
			IEffect* m_pEffect;

			std::array<std::vector<RenderSubsetVertex>, ThreadCount> m_vecVertexSubset;
			std::array<std::vector<RenderSubsetLine>, ThreadCount> m_vecLineSubset;
			std::array<std::vector<RenderSubsetLineSegment>, ThreadCount> m_vecLineSegmentSubset;

			IVertexBuffer* m_pLineSegmentVertexBuffer;

			struct RenderSubsetVertexBatch
			{
				struct InstVertexData
				{
					InstStaticData worldData;
					Math::Color colorData;

					InstVertexData(const Math::Matrix& matWorld, const Math::Color& color)
						: worldData(matWorld)
						, colorData(color)
					{
					}
				};

				const RenderSubsetVertex* pSubset = nullptr;
				std::vector<InstVertexData> vecInstData;

				RenderSubsetVertexBatch(const RenderSubsetVertex* pSubset, const Math::Matrix& matWorld, const Math::Color& color)
					: pSubset(pSubset)
				{
					vecInstData.emplace_back(matWorld, color);
				}
			};
		};
	}
}