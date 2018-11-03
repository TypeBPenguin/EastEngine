#pragma once

#include "RendererDX12.h"

namespace eastengine
{
	namespace graphics
	{
		namespace dx12
		{
			class RenderTarget;
			class DepthStencil;

			class GaussianBlur : public IRendererDX12
			{
			public:
				GaussianBlur();
				virtual ~GaussianBlur();

			public:
				virtual Type GetType() const override { return IRenderer::eGaussianBlur; }
				virtual void RefreshPSO(ID3D12Device* pDevice) override;

			public:
				void Apply(const RenderTarget* pSource, RenderTarget* pResult, float fSigma);
				void Apply(const RenderTarget* pSource, const DepthStencil* pDepth, RenderTarget* pResult, float fSigma);

			private:
				class Impl;
				std::unique_ptr<Impl> m_pImpl;
			};
		}
	}
}