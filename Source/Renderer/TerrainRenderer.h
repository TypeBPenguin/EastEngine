#pragma once

#include "Renderer.h"

namespace eastengine
{
	namespace graphics
	{
		class TerrainRenderer : public IRenderer
		{
		public:
			TerrainRenderer();
			virtual ~TerrainRenderer();

		public:
			virtual void AddRender(const RenderSubsetTerrain& renderPiece);

			virtual void Render(IDevice* pDevice, IDeviceContext* pDeviceContext, Camera* pCamera, uint32_t nRenderGroupFlag) override;
			virtual void Flush();

		private:
			class Impl;
			std::unique_ptr<Impl> m_pImpl;
		};
	}
}