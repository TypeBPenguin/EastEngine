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

			class EnvironmentRenderer : public IRenderer
			{
			public:
				EnvironmentRenderer();
				virtual ~EnvironmentRenderer();

			public:
				virtual Type GetType() const { return IRenderer::eEnvironment; }

			public:
				void Render(const RenderElement& element);

			private:
				class Impl;
				std::unique_ptr<Impl> m_pImpl;
			};
		}
	}
}