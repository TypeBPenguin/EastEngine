#include "stdafx.h"
#include "TextureVulkan.h"

#include "DeviceVulkan.h"

//#define STB_IMAGE_IMPLEMENTATION
#include "ExternLib/stb-master/stb_image.h"

namespace est
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

			bool Texture::Map(MappedSubResourceData& mappedSubResourceData)
			{
				return false;
			}

			void Texture::Unmap()
			{
			}

			bool Texture::Initialize(const TextureDesc& desc)
			{
				return true;
			}

			bool Texture::Load(const wchar_t* filePath)
			{
				VkDevice device = Device::GetInstance()->GetInterface();

				SetState(State::eLoading);

				m_path = filePath;

				int texWidth = 0;
				int texHeight = 0;
				int nTexChannels = 0;

				const std::string path = string::WideToMulti(filePath);
				stbi_uc* pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &nTexChannels, STBI_rgb_alpha);

				if (pixels == nullptr)
				{
					SetState(State::eInvalid);
					LOG_ERROR(L"faield to load texture image : %s", filePath);
					return false;
				}

				const VkDeviceSize imageSize = texWidth * texHeight * 4;

				VkBuffer stagingBuffer = nullptr;
				VkDeviceMemory stagingBufferMemory = nullptr;
				Device::GetInstance()->CreateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingBufferMemory, nullptr);

				void* pData = nullptr;
				vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &pData);
				memcpy(pData, pixels, static_cast<size_t>(imageSize));
				vkUnmapMemory(device, stagingBufferMemory);

				stbi_image_free(pixels);

				Device::GetInstance()->CreateImage(texWidth, texHeight, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &m_textureImage, &m_textureImageMemory);

				Device::GetInstance()->TransitionImageLayout(m_textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
				Device::GetInstance()->CopyBufferToImage(stagingBuffer, m_textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
				Device::GetInstance()->TransitionImageLayout(m_textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

				vkDestroyBuffer(device, stagingBuffer, nullptr);
				vkFreeMemory(device, stagingBufferMemory, nullptr);

				m_textureImageView = Device::GetInstance()->CreateImageView(m_textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);

				m_size.x = static_cast<uint32_t>(texWidth);
				m_size.y = static_cast<uint32_t>(texHeight);

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

				m_size.x = nWidth;
				m_size.y = nHeight;

				SetState(State::eComplete);
			}
		}
	}
}