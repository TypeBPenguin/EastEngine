#pragma once

#include "RendererDX12.h"

namespace est
{
	namespace graphics
	{
		class Camera;

		namespace dx12
		{
			class RenderTarget;
			class DepthStencil;

			class MotionBlur : public IRendererDX12
			{
			public:
				MotionBlur();
				virtual ~MotionBlur();

			public:
				virtual Type GetType() const { return IRenderer::eMotionBlur; }
				virtual void RefreshPSO(ID3D12Device* pDevice) override;

			public:
				void Apply(Camera* pCamera, const math::Matrix& matPrevViewProj, RenderTarget* pSource, DepthStencil* pDepth, RenderTarget* pResult);
				void Apply(Camera* pCamera, RenderTarget* pSource, RenderTarget* pVelocity, RenderTarget* pResult);
				void Apply(Camera* pCamera, RenderTarget* pSource, RenderTarget* pVelocity, RenderTarget* pPrevVelocity, RenderTarget* pResult);

			private:
				class Impl;
				std::unique_ptr<Impl> m_pImpl;
			};
		}
	}
}