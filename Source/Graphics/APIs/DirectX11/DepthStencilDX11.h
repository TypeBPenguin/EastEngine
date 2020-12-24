#pragma once

#include "CommonLib/PhantomType.h"

namespace est
{
	namespace graphics
	{
		namespace dx11
		{
			class DepthStencil
			{
			private:
				struct tKey { static constexpr const wchar_t* DefaultValue() { return L""; } };

			public:
				using Key = PhantomType<tKey, string::StringID>;

			public:
				DepthStencil(const Key& key);
				~DepthStencil();

			public:
				static std::unique_ptr<DepthStencil> Create(const D3D11_TEXTURE2D_DESC* pDesc);
				static Key BuildKey(const D3D11_TEXTURE2D_DESC* pDesc);

			public:
				const Key& GetKey() const { return m_key; }
				void GetDesc2D(D3D11_TEXTURE2D_DESC* pDesc) const;

				ID3D11Texture2D* GetTexture2D() const { return m_pTexture2D; }
				ID3D11DepthStencilView* GetDepthStencilView() const { return m_pDepthStencilView; }
				ID3D11ShaderResourceView* GetShaderResourceView() const { return m_pShaderResourceView; }

			private:
				const Key m_key;
				ID3D11Texture2D* m_pTexture2D{ nullptr };
				ID3D11DepthStencilView * m_pDepthStencilView{ nullptr };
				ID3D11ShaderResourceView* m_pShaderResourceView{ nullptr };
			};
		}
	}
}

namespace std
{
	template <>
	struct hash<est::graphics::dx11::DepthStencil::Key>
	{
		const size_t operator()(const est::graphics::dx11::DepthStencil::Key& key) const
		{
			return reinterpret_cast<size_t>(key.Value().GetData());
		}
	};
}