#pragma once

#include "GraphicsInterface/RenderJob.h"

namespace eastengine
{
	namespace graphics
	{
		namespace dx11
		{
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

			private:
				class Impl;
				std::unique_ptr<Impl> m_pImpl;
			};
		}
	}
}