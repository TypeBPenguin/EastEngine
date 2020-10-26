#pragma once

#include "Graphics/Interface/Define.h"

#include "ImageBufferVulkan.h"

namespace est
{
	namespace graphics
	{
		namespace vulkan
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
				constexpr size_t GetCount() const noexcept { return GBufferTypeCount; }
				const ImageBuffer* GetRenderTarget(GBufferType emType) const { return m_pRenderTargets[emType].get(); }
				const ImageBuffer* GetDepthStencil() const { return m_pDepthStencil.get(); }

			private:
				std::array<std::unique_ptr<ImageBuffer>, GBufferTypeCount> m_pRenderTargets;
				std::unique_ptr<ImageBuffer> m_pDepthStencil;
			};
		}
	}
}