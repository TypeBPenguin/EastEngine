#pragma once

#include "Graphics/Interface/GraphicsInterface.h"

struct ID3D11Texture2D;
struct ID3D11ShaderResourceView;
struct D3D11_TEXTURE2D_DESC;
struct D3D11_SUBRESOURCE_DATA;

namespace est
{
	namespace graphics
	{
		namespace dx11
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
				bool Map(ID3D11DeviceContext* pDeviceContext, MappedSubResourceData& mappedSubResourceData);
				void Unmap(ID3D11DeviceContext* pDeviceContext);

			public:
				bool Initialize(const TextureDesc& desc);
				bool Initialize(const D3D11_TEXTURE2D_DESC* pDesc, const D3D11_SUBRESOURCE_DATA* pInitialData);
				bool Load(const wchar_t* filePath);

			public:
				ID3D11Texture2D* GetTexture2D() const { return m_pTexture2D; }
				ID3D11ShaderResourceView* GetShaderResourceView() const { return m_pShaderResourceView; }

			private:
				const ITexture::Key m_key;

				math::uint2 m_size;
				std::wstring m_path;

				ID3D11Texture2D* m_pTexture2D{ nullptr };
				ID3D11ShaderResourceView* m_pShaderResourceView{ nullptr };
			};
		}
	}
}