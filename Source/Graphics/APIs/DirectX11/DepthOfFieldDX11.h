#pragma once

#include "Graphics/Interface/Renderer.h"

namespace est
{
	namespace graphics
	{
		class Camera;

		namespace dx11
		{
			class RenderTarget;
			class DepthStencil;

			class DepthOfField : public IRenderer
			{
			public:
				DepthOfField();
				virtual ~DepthOfField();

			public:
				virtual Type GetType() const { return IRenderer::eDepthOfField; }

			public:
				void Apply(Camera* pCamera, const RenderTarget* pSource, const DepthStencil* pDepth, RenderTarget* pResult);

			private:
				class Impl;
				std::unique_ptr<Impl> m_pImpl;
			};
		}
	}
}