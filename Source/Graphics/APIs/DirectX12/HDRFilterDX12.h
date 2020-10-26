#pragma once

#include "RendererDX12.h"

namespace est
{
	namespace graphics
	{
		namespace dx12
		{
			class RenderTarget;

			class HDRFilter : public IRendererDX12
			{
			public:
				HDRFilter();
				virtual ~HDRFilter();

			public:
				virtual Type GetType() const override { return IRenderer::eHDR; }
				virtual void RefreshPSO(ID3D12Device* pDevice) override;

			public:
				void Apply(RenderTarget* pSource, RenderTarget* pResult);

			private:
				class Impl;
				std::unique_ptr<Impl> m_pImpl;
			};
		}
	}
}