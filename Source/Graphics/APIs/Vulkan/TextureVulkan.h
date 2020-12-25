#pragma once

#include "Graphics/Interface/GraphicsInterface.h"

struct VkImageView_T;

namespace est
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
				virtual const math::uint2& GetSize() const override { return m_size; }
				virtual const std::wstring& GetPath() const override { return m_path; }

				virtual bool Map(MappedSubResourceData& mappedSubResourceData) override;
				virtual void Unmap() override;

			public:
				bool Initialize(const TextureDesc& desc);
				bool Load(const wchar_t* strFilePath);
				void Bind(uint32_t nWidth, uint32_t nHeight, const uint8_t* pPixels);

			public:
				VkImageView_T* GetImageView() const { return m_textureImageView; }

			private:
				const ITexture::Key m_key;

				math::uint2 m_size;
				std::wstring m_path;

				VkImage m_textureImage{ nullptr };
				VkDeviceMemory m_textureImageMemory{ nullptr };
				VkImageView m_textureImageView{ nullptr };
			};
		}
	}
}