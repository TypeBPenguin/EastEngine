#pragma once

#include "Graphics/Interface/RenderJob.h"
#include "Graphics/Interface/Renderer.h"

namespace est
{
	namespace graphics
	{
		namespace dx11
		{
			struct RenderElement;

			class TerrainRenderer : public IRenderer
			{
			public:
				TerrainRenderer();
				virtual ~TerrainRenderer();

				enum Group
				{
					eDeferred = 0,
					eShadow,

					GroupCount,
				};

			public:
				virtual Type GetType() const { return IRenderer::eTerrain; }

			public:
				void Render(const RenderElement& element, Group group, const math::Matrix& matPrevViewProjection);
				void AllCleanup();
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