#pragma once

#include "Graphics/Interface/RenderJob.h"
#include "Graphics/Interface/Renderer.h"

namespace est
{
	namespace graphics
	{
		class Camera;

		namespace dx11
		{
			struct RenderElement;

			class ModelRenderer : public IRenderer
			{
			public:
				ModelRenderer();
				virtual ~ModelRenderer();

				enum Group
				{
					eDeferred = 0,
					eAlphaBlend,
					eShadow,

					GroupCount,
				};

			public:
				virtual Type GetType() const { return IRenderer::eModel; }

			public:
				void Render(const RenderElement& element, Group emGroup, const math::Matrix& matPrevViewProjection);
				void AllCleanup();
				void Cleanup();

			public:
				void PushJob(const RenderJobStatic& job);
				void PushJob(const RenderJobSkinned& job);

			private:
				class Impl;
				std::unique_ptr<Impl> m_pImpl;
			};
		}
	}
}