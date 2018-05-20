#pragma once

enum VkFormat;

struct VkImageView_T;
typedef VkImageView_T* VkImageView;

struct VkImage_T;
typedef VkImage_T* VkImage;

namespace eastengine
{
	namespace graphics
	{
		namespace vulkan
		{
			class ImageBuffer
			{
			public:
				ImageBuffer();
				ImageBuffer(const math::UInt2& n2Size, VkFormat format, VkImageUsageFlags usage);
				virtual ~ImageBuffer();

			public:
				const math::UInt2& GetSize() const { return m_n2Size; }
				VkFormat GetFormat() const { return m_format; }
				const VkImage GetImage() const { return m_image; }
				const VkImageView GetImageView() const { return m_imageView; }

			protected:
				math::UInt2 m_n2Size;

				VkImage m_image{ nullptr };
				VkImageView m_imageView{ nullptr };
				VkDeviceMemory m_memory{ nullptr };
				VkFormat m_format{ VK_FORMAT_UNDEFINED };
			};
		}
	}
}