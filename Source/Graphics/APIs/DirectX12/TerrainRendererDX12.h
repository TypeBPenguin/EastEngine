#pragma once

#include "Graphics/Interface/RenderJob.h"
#include "RendererDX12.h"

namespace est
{
	namespace graphics
	{
		namespace dx12
		{
			struct RenderElement;

			class TerrainRenderer : public IRendererDX12
			{
			public:
				TerrainRenderer();
				virtual ~TerrainRenderer();

			public:
				virtual Type GetType() const override { return IRenderer::eTerrain; }
				virtual void RefreshPSO(ID3D12Device* pDevice) override;

			public:
				void Render(const RenderElement& renderElement, const math::Matrix& matPrevViewProjection);
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