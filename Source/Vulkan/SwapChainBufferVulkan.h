#pragma once

#include "ImageBufferVulkan.h"

namespace eastengine
{
	namespace graphics
	{
		namespace vulkan
		{
			class SwapChainBuffer : public ImageBuffer
			{
			public:
				SwapChainBuffer(const math::UInt2& n2Size, VkImage image, VkFormat format);
				virtual ~SwapChainBuffer();
			};
		}
	}
}