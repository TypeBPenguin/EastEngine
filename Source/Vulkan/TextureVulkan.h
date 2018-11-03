#pragma once

#include "GraphicsInterface/GraphicsInterface.h"

struct VkImageView_T;

namespace eastengine
{
	namespace graphics
	{
		namespace vulkan
		{
			class Texture : public ITexture
			{
			public:
				Texture(const ITexture::Key& key);
				virtual ~Texture();

			public:
				virtual const ITexture::Key& GetKey() const override { return m_key; }
				virtual const string::StringID& GetName() const override { return m_key.Value(); }

			public:
				virtual const math::UInt2& GetSize() const override { return m_n2Size; }
				virtual const std::string& GetPath() const override { return m_strPath; }

			public:
				bool Initialize(const TextureDesc& desc);
				bool Load(const char* strFilePath);
				void Bind(uint32_t nWidth, uint32_t nHeight, const uint8_t* pPixels);

			public:
				VkImageView_T* GetImageView() const { return m_textureImageView; }

			private:
				const ITexture::Key m_key;

				math::UInt2 m_n2Size;
				std::string m_strPath;

				VkImage m_textureImage{ nullptr };
				VkDeviceMemory m_textureImageMemory{ nullptr };
				VkImageView m_textureImageView{ nullptr };
			};
		}
	}
}