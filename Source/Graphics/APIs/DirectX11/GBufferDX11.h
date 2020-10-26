#pragma once

#include "Graphics/Interface/Define.h"

#include "RenderTargetDX11.h"
#include "DepthStencilDX11.h"

namespace est
{
	namespace graphics
	{
		namespace dx11
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
				void Clear(ID3D11DeviceContext* pImmediateContext);

			public:
				constexpr size_t GetCount() const { return GBufferTypeCount; }
				const RenderTarget* GetRenderTarget(GBufferType emType) const { return m_pRenderTargets[emType].get(); }
				const DepthStencil* GetDepthStencil() const { return m_pDepthStencil.get(); }

			public:
				const RenderTarget* GetPrevVelocityBuffer() const { return m_pPrevVelocityBuffer.get(); }

			private:
				std::array<std::unique_ptr<RenderTarget>, GBufferTypeCount> m_pRenderTargets;
				std::unique_ptr<RenderTarget> m_pPrevVelocityBuffer;

				std::unique_ptr<DepthStencil> m_pDepthStencil;
			};
		}
	}
}