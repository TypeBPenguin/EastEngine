#pragma once

#include "Graphics/Interface/Define.h"

#include "RenderTargetDX12.h"
#include "DepthStencilDX12.h"

namespace est
{
	namespace graphics
	{
		namespace dx12
		{
			class GBuffer
			{
			public:
				GBuffer(uint32_t width, uint32_t height);
				~GBuffer();

			public:
				void Resize(uint32_t width, uint32_t height);
				void Resize(GBufferType emType, uint32_t width, uint32_t height);
				void Release();
				void Release(GBufferType emType);

			public:
				void Cleanup();
				void Clear(ID3D12GraphicsCommandList* pCommandList);

			public:
				constexpr size_t GetCount() const noexcept { return GBufferTypeCount; }
				RenderTarget* GetRenderTarget(GBufferType emType) { return m_pRenderTargets[emType].get(); }
				DepthStencil* GetDepthStencil() { return m_pDepthStencil.get(); }

			public:
				RenderTarget* GetPrevVelocityBuffer() const { return m_pPrevVelocityBuffer.get(); }

			private:
				std::array<std::unique_ptr<RenderTarget>, GBufferTypeCount> m_pRenderTargets;
				std::unique_ptr<RenderTarget> m_pPrevVelocityBuffer;

				std::unique_ptr<DepthStencil> m_pDepthStencil;
			};
		}
	}
}