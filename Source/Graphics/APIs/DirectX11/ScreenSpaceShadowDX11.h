#pragma once

#include "Graphics/Interface/Renderer.h"

namespace est
{
	namespace graphics
	{
		namespace dx11
		{
			class RenderTarget;
			class DepthStencil;

			class ScreenSpaceShadow : public IRenderer
			{
			public:
				ScreenSpaceShadow();
				virtual ~ScreenSpaceShadow();

			public:
				virtual Type GetType() const { return IRenderer::eScreenSpaceShadow; }

			public:
				void Apply(const RenderTarget* pSource, const DepthStencil* pDepth, RenderTarget* pResult);

			private:
				class Impl;
				std::unique_ptr<Impl> m_pImpl;
			};
		}
	}
}