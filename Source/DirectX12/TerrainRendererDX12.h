#pragma once

#include "GraphicsInterface/RenderJob.h"
#include "RendererDX12.h"

namespace eastengine
{
	namespace graphics
	{
		class Camera;

		namespace dx12
		{
			class TerrainRenderer : public IRendererDX12
			{
			public:
				TerrainRenderer();
				virtual ~TerrainRenderer();

			public:
				virtual Type GetType() const override { return IRenderer::eTerrain; }
				virtual void RefreshPSO(ID3D12Device* pDevice) override;

			public:
				void Render(Camera* pCamera);
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