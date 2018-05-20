#pragma once

#include "GraphicsInterface/RenderJob.h"

namespace eastengine
{
	namespace graphics
	{
		class Camera;

		namespace dx12
		{
			class ModelRenderer
			{
			public:
				ModelRenderer();
				~ModelRenderer();

				enum Group
				{
					eDeferred = 0,
					eAlphaBlend,

					GroupCount,
				};

			public:
				void Render(Camera* pCamera, Group emGroup);
				void Flush();

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