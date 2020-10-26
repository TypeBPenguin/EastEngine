#pragma once

#include "Graphics/Interface/Renderer.h"

namespace est
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