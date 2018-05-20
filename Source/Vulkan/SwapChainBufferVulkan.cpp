#include "stdafx.h"
#include "SwapChainBufferVulkan.h"

#include "DeviceVulkan.h"

namespace eastengine
{
	namespace graphics
	{
		namespace vulkan
		{
			SwapChainBuffer::SwapChainBuffer(const math::UInt2& n2Size, VkImage image, VkFormat format)
			{
				m_n2Size = n2Size;

				m_image = image;
				m_format = format;

				m_imageView = Device::GetInstance()->CreateImageView(m_image, m_format, VK_IMAGE_ASPECT_COLOR_BIT);
			}

			SwapChainBuffer::~SwapChainBuffer()
			{
				m_image = nullptr;
			}
		}
	}
}
