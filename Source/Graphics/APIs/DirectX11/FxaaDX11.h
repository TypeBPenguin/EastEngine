#pragma once

#include "Graphics/Interface/Renderer.h"

namespace est
{
	namespace graphics
	{
		namespace dx11
		{
			class RenderTarget;

			class Fxaa : public IRenderer
			{
			public:
				Fxaa();
				virtual ~Fxaa();

			public:
				virtual Type GetType() const { return IRenderer::eFxaa; }

			public:
				void Apply(const RenderTarget* pSource, RenderTarget* pResult);

			private:
				class Impl;
				std::unique_ptr<Impl> m_pImpl;
			};
		}
	}
}