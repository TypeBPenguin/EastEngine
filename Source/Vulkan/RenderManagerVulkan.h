#pragma once

#include "GraphicsInterface/RenderJob.h"

namespace eastengine
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
				void Render();

			public:
				void PushJob(const RenderJobStatic& renderJob);
				void PushJob(const RenderJobSkinned& renderJob);

			private:
				class Impl;
				std::unique_ptr<Impl> m_pImpl;
			};
		}
	}
}