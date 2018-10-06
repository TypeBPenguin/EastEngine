#pragma once

#include "GraphicsInterface/RenderJob.h"

namespace eastengine
{
	namespace graphics
	{
		class Camera;

		namespace dx12
		{
			class TerrainRenderer
			{
			public:
				TerrainRenderer();
				~TerrainRenderer();

			public:
				void Render(Camera* pCamera);
				void Flush();

			public:
				void PushJob(const RenderJobTerrain& job);

			private:
				class Impl;
				std::unique_ptr<Impl> m_pImpl;
			};
		}
	}
}