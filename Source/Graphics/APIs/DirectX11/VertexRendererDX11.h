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

			class VertexRenderer : public IRenderer
			{
			public:
				VertexRenderer();
				virtual ~VertexRenderer();

			public:
				virtual Type GetType() const { return IRenderer::eVertex; }

			public:
				void Render(const RenderElement& renderElement);
				void Cleanup();

			public:
				void PushJob(const RenderJobVertex& job);

			private:
				class Impl;
				std::unique_ptr<Impl> m_pImpl;
			};
		}
	}
}