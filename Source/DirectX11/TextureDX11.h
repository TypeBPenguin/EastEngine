#pragma once

#include "GraphicsInterface/GraphicsInterface.h"

struct ID3D11Texture2D;
struct ID3D11ShaderResourceView;
struct D3D11_TEXTURE2D_DESC;
struct D3D11_SUBRESOURCE_DATA;

namespace eastengine
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
				virtual const math::uint2& GetSize() const override { return m_n2Size; }
				virtual const std::string& GetPath() const override { return m_strPath; }

			public:
				bool Initialize(const TextureDesc& desc);
				bool Initialize(const D3D11_TEXTURE2D_DESC* pDesc, const D3D11_SUBRESOURCE_DATA* pInitialData);
				bool Load(const char* strFilePath);

			public:
				ID3D11Texture2D* GetTexture2D() const { return m_pTexture2D; }
				ID3D11ShaderResourceView* GetShaderResourceView() const { return m_pShaderResourceView; }

			private:
				const ITexture::Key m_key;

				math::uint2 m_n2Size;
				std::string m_strPath;

				ID3D11Texture2D* m_pTexture2D{ nullptr };
				ID3D11ShaderResourceView* m_pShaderResourceView{ nullptr };
			};
		}
	}
}