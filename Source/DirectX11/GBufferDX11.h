#pragma once

#include "GraphicsInterface/Define.h"

#include "RenderTargetDX11.h"
#include "DepthStencilDX11.h"

namespace eastengine
{
	namespace graphics
	{
		namespace dx11
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
				void Clear(ID3D11DeviceContext* pImmediateContext);

			public:
				size_t GetCount() const { return EmGBuffer::Count; }
				const RenderTarget* GetRenderTarget(EmGBuffer::Type emType) const { return m_pRenderTargets[emType].get(); }
				const DepthStencil* GetDepthStencil() const { return m_pDepthStencil.get(); }

			private:
				std::array<std::unique_ptr<RenderTarget>, EmGBuffer::Count> m_pRenderTargets;
				std::unique_ptr<DepthStencil> m_pDepthStencil;
			};
		}
	}
}