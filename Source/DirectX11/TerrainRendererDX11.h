#pragma once

#include "GraphicsInterface/RenderJob.h"
#include "GraphicsInterface/Renderer.h"

namespace eastengine
{
	namespace graphics
	{
		class Camera;

		namespace dx11
		{
			class TerrainRenderer : public IRenderer
			{
			public:
				TerrainRenderer();
				virtual ~TerrainRenderer();

			public:
				virtual Type GetType() const { return IRenderer::eTerrain; }

			public:
				void Render(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, Camera* pCamera);
				void Cleanup();

			public:
				void PushJob(const RenderJobTerrain& job);

			private:
				class Impl;
				std::unique_ptr<Impl> m_pImpl;
			};
		}
	}
}