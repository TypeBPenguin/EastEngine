#pragma once

#include "GraphicsInterface/Renderer.h"

namespace eastengine
{
	namespace graphics
	{
		namespace dx11
		{
			class RenderTarget;

			class HDRFilter : public IRenderer
			{
			public:
				HDRFilter();
				virtual ~HDRFilter();

			public:
				virtual Type GetType() const { return IRenderer::eHDR; }

			public:
				void Apply(const RenderTarget* pSource, RenderTarget* pResult);

			private:
				class Impl;
				std::unique_ptr<Impl> m_pImpl;
			};
		}
	}
}