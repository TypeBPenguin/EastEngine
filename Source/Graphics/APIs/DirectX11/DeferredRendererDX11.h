#pragma once

#include "Graphics/Interface/Renderer.h"

namespace est
{
	namespace graphics
	{
		namespace dx11
		{
			struct RenderElement;

			class DeferredRenderer : public IRenderer
			{
			public:
				DeferredRenderer();
				virtual ~DeferredRenderer();

			public:
				virtual Type GetType() const { return IRenderer::eDeferred; }

			public:
				void Render(const RenderElement& renderElement);
				
			private:
				class Impl;
				std::unique_ptr<Impl> m_pImpl;
			};
		}
	}
}