#pragma once

#include "GraphicsInterface/RenderJob.h"

namespace eastengine
{
	namespace graphics
	{
		namespace dx12
		{
			class VertexBuffer;
			class IndexBuffer;
			class Texture;

			class RenderManager
			{
			public:
				RenderManager();
				~RenderManager();

			public:
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