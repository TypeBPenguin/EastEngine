#include "stdafx.h"
#include "TextureVulkan.h"

#include "DeviceVulkan.h"

//#define STB_IMAGE_IMPLEMENTATION
#include "ExternLib/stb-master/stb_image.h"

namespace eastengine
{
	namespace graphics
	{
		namespace vulkan
		{
			Texture::Texture(const ITexture::Key& key)
				: m_key(key)
			{
			}

			Texture::~Texture()
			{
				VkDevice device = Device::GetInstance()->GetInterface();

				vkDestroyImageView(device, m_textureImageView, nullptr);

				vkDestroyImage(device, m_textureImage, nullptr);
				vkFreeMemory(device, m_textureImageMemory, nullptr);
			}

			bool Texture::Initialize(const TextureDesc& desc)
			{
				return true;
			}

			bool Texture::Load(const char* strFilePath)
			{
				VkDevice device = Device::GetInstance()->GetInterface();

				SetState(State::eLoading);

				m_strPath = strFilePath;

				int nTexWidth = 0;
				int nTexHeight = 0;
				int nTexChannels = 0;

				stbi_uc* pixels = stbi_load(strFilePath, &nTexWidth, &nTexHeight, &nTexChannels, STBI_rgb_alpha);

				if (pixels == nullptr)
				{
					SetState(State::eInvalid);
					LOG_ERROR("faield to load texture image : %s", strFilePath);
					return false;
				}

				const VkDeviceSize imageSize = nTexWidth * nTexHeight * 4;

				VkBuffer stagingBuffer = nullptr;
				VkDeviceMemory stagingBufferMemory = nullptr;
				Device::GetInstance()->CreateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingBufferMemory, nullptr);

				void* pData = nullptr;
				vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &pData);
				memcpy(pData, pixels, static_cast<size_t>(imageSize));
				vkUnmapMemory(device, stagingBufferMemory);

				stbi_image_free(pixels);

				Device::GetInstance()->CreateImage(nTexWidth, nTexHeight, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &m_textureImage, &m_textureImageMemory);

				Device::GetInstance()->TransitionImageLayout(m_textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
				Device::GetInstance()->CopyBufferToImage(stagingBuffer, m_textureImage, static_cast<uint32_t>(nTexWidth), static_cast<uint32_t>(nTexHeight));
				Device::GetInstance()->TransitionImageLayout(m_textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

				vkDestroyBuffer(device, stagingBuffer, nullptr);
				vkFreeMemory(device, stagingBufferMemory, nullptr);

				m_textureImageView = Device::GetInstance()->CreateImageView(m_textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);

				m_n2Size.x = static_cast<uint32_t>(nTexWidth);
				m_n2Size.y = static_cast<uint32_t>(nTexHeight);

				SetState(State::eComplete);

				return true;
			}

			void Texture::Bind(uint32_t nWidth, uint32_t nHeight, const uint8_t* pPixels)
			{
				VkDevice device = Device::GetInstance()->GetInterface();

				SetState(State::eLoading);

				const VkDeviceSize imageSize = nWidth * nHeight * 4;

				VkBuffer stagingBuffer = nullptr;
				VkDeviceMemory stagingBufferMemory = nullptr;
				Device::GetInstance()->CreateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingBufferMemory, nullptr);

				void* pData = nullptr;
				vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &pData);
				memcpy(pData, pPixels, static_cast<size_t>(imageSize));
				vkUnmapMemory(device, stagingBufferMemory);

				Device::GetInstance()->CreateImage(nWidth, nHeight, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &m_textureImage, &m_textureImageMemory);

				Device::GetInstance()->TransitionImageLayout(m_textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
				Device::GetInstance()->CopyBufferToImage(stagingBuffer, m_textureImage, nWidth, nHeight);
				Device::GetInstance()->TransitionImageLayout(m_textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

				vkDestroyBuffer(device, stagingBuffer, nullptr);
				vkFreeMemory(device, stagingBufferMemory, nullptr);

				m_textureImageView = Device::GetInstance()->CreateImageView(m_textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);

				m_n2Size.x = nWidth;
				m_n2Size.y = nHeight;

				SetState(State::eComplete);
			}
		}
	}
}