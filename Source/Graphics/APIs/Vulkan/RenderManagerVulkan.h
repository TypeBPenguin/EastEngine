#pragma once

#include "Graphics/Interface/RenderJob.h"

namespace est
{
	namespace graphics
	{
		namespace vulkan
		{
			class VertexBuffer;
			class IndexBuffer;
			class Texture;
			class ImageBuffer;

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
				void PushJob(const RenderJobStatic& renderJob);
				void PushJob(const RenderJobSkinned& renderJob);
				void PushJob(const RenderJobTerrain& renderJob);
				void PushJob(const RenderJobVertex& renderJob);

			private:
				class Impl;
				std::unique_ptr<Impl> m_pImpl;
			};
		}
	}
}