#pragma once

#include "Renderer.h"

namespace EastEngine
{
	namespace Graphics
	{
		class IEffect;
		class IEffectTech;

		class TerrainRenderer : public IRenderer
		{
		public:
			TerrainRenderer();
			virtual ~TerrainRenderer();

		public:
			virtual bool Init(const Math::Viewport& viewport) override;

			virtual void AddRender(const RenderSubsetTerrain& renderPiece) override { m_vecTerrain.emplace_back(renderPiece); }

			virtual void Render(uint32_t nRenderGroupFlag) override;
			virtual void Flush() override { m_vecTerrain.clear(); }

		private:
			void ClearEffect(IDeviceContext* pd3dDeviceContext, IEffectTech* pTech);

		private:
			IEffect* m_pEffect;

			std::vector<RenderSubsetTerrain> m_vecTerrain;
		};
	}
}