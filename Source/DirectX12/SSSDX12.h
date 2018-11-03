#pragma once

#include "RendererDX12.h"

namespace eastengine
{
	namespace graphics
	{
		class Camera;

		namespace dx12
		{
			class RenderTarget;
			class DepthStencil;

			class SSS : public IRendererDX12
			{
			public:
				SSS();
				virtual ~SSS();

			public:
				virtual Type GetType() const override { return IRenderer::eGaussianBlur; }
				virtual void RefreshPSO(ID3D12Device* pDevice) override;

			public:
				void Apply(RenderTarget* pSource, const DepthStencil* pDepth, RenderTarget* pResult);

			private:
				class Impl;
				std::unique_ptr<Impl> m_pImpl;
			};
		}
	}
}