#pragma once

#include "Graphics/Interface/Renderer.h"
#include "Graphics/Interface/RenderJob.h"

namespace est
{
	namespace graphics
	{
		namespace dx11
		{
			class RenderTarget;

			class RenderManager
			{
			public:
				RenderManager();
				~RenderManager();

			public:
				void AllCleanup();
				void Cleanup();
				void Render();

			public:
				void Copy_RGBA(const RenderTarget* pSource, RenderTarget* pResult);
				void Copy_RGB(const RenderTarget* pSource, RenderTarget* pResult);

			public:
				void PushJob(const RenderJobStatic& renderJob);
				void PushJob(const RenderJobSkinned& renderJob);
				void PushJob(const RenderJobTerrain& renderJob);
				void PushJob(const RenderJobVertex& renderJob);

			public:
				IRenderer* GetRenderer(IRenderer::Type emType) const;

			private:
				class Impl;
				std::unique_ptr<Impl> m_pImpl;
			};
		}
	}
}