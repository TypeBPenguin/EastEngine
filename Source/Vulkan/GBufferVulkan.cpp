#include "stdafx.h"
#include "GBufferVulkan.h"

#include "DeviceVulkan.h"

namespace eastengine
{
	namespace graphics
	{
		namespace vulkan
		{
			GBuffer::GBuffer(uint32_t nWidth, uint32_t nHeight)
			{
				Resize(nWidth, nHeight);
			}

			GBuffer::~GBuffer()
			{
				Release();
			}

			void GBuffer::Resize(uint32_t nWidth, uint32_t nHeight)
			{
				Release();

				const math::uint2 n2Size{ nWidth, nHeight };

				m_pRenderTargets[EmGBuffer::eNormals] = std::make_unique<ImageBuffer>(n2Size, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
				m_pRenderTargets[EmGBuffer::eColors] = std::make_unique<ImageBuffer>(n2Size, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
				m_pRenderTargets[EmGBuffer::eDisneyBRDF] = std::make_unique<ImageBuffer>(n2Size, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);

				const VkFormat depthStencilFormat = Device::GetInstance()->FindDepthFormat();
				m_pDepthStencil = std::make_unique<ImageBuffer>(n2Size, depthStencilFormat, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
			}

			void GBuffer::Release()
			{
				for (auto& pRenderTarget : m_pRenderTargets)
				{
					pRenderTarget.reset();
				}

				m_pDepthStencil.reset();
			}
		}
	}
}
