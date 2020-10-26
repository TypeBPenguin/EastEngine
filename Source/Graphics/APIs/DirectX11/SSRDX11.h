#pragma once

#include "Graphics/Interface/Renderer.h"

namespace est
{
	namespace graphics
	{
		class Camera;

		namespace dx11
		{
			class GBuffer;
			class RenderTarget;
			class DepthStencil;

			class SSR : public IRenderer
			{
			public:
				SSR();
				virtual ~SSR();

			public:
				virtual Type GetType() const { return IRenderer::eSSR; }

			public:
				void Apply(Camera* pCamera, const RenderTarget* pSource, const GBuffer* pGBuffer, const DepthStencil* pDepth, RenderTarget* pResult);

			private:
				class Impl;
				std::unique_ptr<Impl> m_pImpl;
			};
		}
	}
}