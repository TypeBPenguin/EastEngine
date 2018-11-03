#pragma once

#include "GraphicsInterface/Renderer.h"

namespace eastengine
{
	namespace graphics
	{
		namespace dx11
		{
			class RenderTarget;
			class DepthStencil;

			class SSS : public IRenderer
			{
			public:
				SSS();
				virtual ~SSS();

			public:
				virtual Type GetType() const { return IRenderer::eSSS; }

			public:
				void Apply(const RenderTarget* pSource, const DepthStencil* pDepth, RenderTarget* pResult);

			private:
				class Impl;
				std::unique_ptr<Impl> m_pImpl;
			};
		}
	}
}