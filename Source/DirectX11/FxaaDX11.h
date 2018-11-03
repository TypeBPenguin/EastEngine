#pragma once

#include "GraphicsInterface/Renderer.h"

namespace eastengine
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