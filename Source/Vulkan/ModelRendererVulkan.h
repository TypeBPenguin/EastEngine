#pragma once

#include "GraphicsInterface/RenderJob.h"

namespace eastengine
{
	namespace graphics
	{
		class Camera;

		namespace vulkan
		{
			class ModelRenderer
			{
			public:
				ModelRenderer();
				~ModelRenderer();

			public:
				void Render(Camera* pCamera);
				void Cleanup();

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