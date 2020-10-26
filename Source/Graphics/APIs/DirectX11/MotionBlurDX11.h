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

			class MotionBlur : public IRenderer
			{
			public:
				MotionBlur();
				virtual ~MotionBlur();

			public:
				virtual Type GetType() const { return IRenderer::eMotionBlur; }

			public:
				void Apply(Camera* pCamera, const math::Matrix& matPrevViewProj, const RenderTarget* pSource, const DepthStencil* pDepth, RenderTarget* pResult);
				void Apply(Camera* pCamera, const RenderTarget* pSource, const RenderTarget* pVelocity, RenderTarget* pResult);
				void Apply(Camera* pCamera, const RenderTarget* pSource, const RenderTarget* pVelocity, const RenderTarget* pPrevVelocity, RenderTarget* pResult);

			private:
				class Impl;
				std::unique_ptr<Impl> m_pImpl;
			};
		}
	}
}