#pragma once

#include "RendererDX12.h"

namespace eastengine
{
	namespace graphics
	{
		namespace dx12
		{
			class RenderTarget;

			class BloomFilter : public IRendererDX12
			{
			public:
				BloomFilter();
				virtual ~BloomFilter();

			public:
				virtual Type GetType() const override { return IRenderer::eBloomFilter; }
				virtual void RefreshPSO(ID3D12Device* pDevice) override;

			public:
				void Apply(RenderTarget* pSource);

			private:
				class Impl;
				std::unique_ptr<Impl> m_pImpl;
			};
		}
	}
}