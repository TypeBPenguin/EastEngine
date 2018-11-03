#pragma once

#include "GraphicsInterface/Renderer.h"

namespace eastengine
{
	namespace graphics
	{
		class Camera;

		namespace dx11
		{
			class RenderTarget;
			class DepthStencil;

			class Assao : public IRenderer
			{
			public:
				Assao();
				virtual ~Assao();

			public:
				virtual Type GetType() const { return IRenderer::eAssao; }

			public:
				void Apply(const Camera* pCamera, const RenderTarget* pNormalMap, const DepthStencil* pDepth, RenderTarget* pResult);

			private:
				class Impl;
				std::unique_ptr<Impl> m_pImpl;
			};
		}
	}
}