#include "stdafx.h"
#include "ImageBufferVulkan.h"

#include "DeviceVulkan.h"

namespace eastengine
{
	namespace graphics
	{
		namespace vulkan
		{
			ImageBuffer::ImageBuffer()
			{
			}

			ImageBuffer::ImageBuffer(const math::uint2& n2Size, VkFormat format, VkImageUsageFlags usage)
				: m_n2Size(n2Size)
				, m_format(format)
			{
				VkImageAspectFlags aspectMask = 0;
				VkImageLayout imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;

				if (usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
				{
					aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
					imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				}

				if (usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
				{
					aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
					imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
				}

				assert(aspectMask > 0);

				Device::GetInstance()->CreateImage(n2Size.x, n2Size.y, format, VK_IMAGE_TILING_OPTIMAL, usage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &m_image, &m_memory);
				m_imageView = Device::GetInstance()->CreateImageView(m_image, format, aspectMask);
			}

			ImageBuffer::~ImageBuffer()
			{
				VkDevice device = Device::GetInstance()->GetInterface();

				vkDestroyImageView(device, m_imageView, nullptr);
				vkDestroyImage(device, m_image, nullptr);
				vkFreeMemory(device, m_memory, nullptr);
			}
		}
	}
}