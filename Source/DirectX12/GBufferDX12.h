#pragma once

#include "GraphicsInterface/Define.h"

#include "RenderTargetDX12.h"
#include "DepthStencilDX12.h"

namespace eastengine
{
	namespace graphics
	{
		namespace dx12
		{
			class GBuffer
			{
			public:
				GBuffer(uint32_t nWidth, uint32_t nHeight);
				~GBuffer();

			public:
				void Resize(uint32_t nWidth, uint32_t nHeight);
				void Release();

			public:
				void Clear(ID3D12GraphicsCommandList* pCommandList);

			public:
				constexpr size_t GetCount() const noexcept { return EmGBuffer::Count; }
				RenderTarget* GetRenderTarget(EmGBuffer::Type emType) { return m_pRenderTargets[emType].get(); }
				DepthStencil* GetDepthStencil() { return m_pDepthStencil.get(); }

			private:
				std::array<std::unique_ptr<RenderTarget>, EmGBuffer::Count> m_pRenderTargets;
				std::unique_ptr<DepthStencil> m_pDepthStencil;
			};
		}
	}
}