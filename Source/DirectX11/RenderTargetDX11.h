#pragma once

#include "CommonLib/PhantomType.h"

namespace eastengine
{
	namespace graphics
	{
		namespace dx11
		{
			class RenderTarget
			{
			private:
				struct tKey {};

			public:
				using Key = PhantomType<tKey, const String::StringID>;

			public:
				RenderTarget(const Key& key);
				~RenderTarget();

			public:
				static std::unique_ptr<RenderTarget> Create(ID3D11Texture2D* pTexture2D);
				static std::unique_ptr<RenderTarget> Create(const D3D11_TEXTURE2D_DESC* pDesc);
				static Key BuildKey(const D3D11_TEXTURE2D_DESC* pDesc);

			public:
				const Key& GetKey() const { return m_key; }
				void GetDesc2D(D3D11_TEXTURE2D_DESC* pDesc) const;

				ID3D11Texture2D* GetTexture2D() const { return m_pTexture2D; }
				ID3D11RenderTargetView* GetRenderTargetView() const { return m_pRenderTargetView; }
				ID3D11ShaderResourceView* GetShaderResourceView() const { return m_pShaderResourceView; }

			private:
				const Key m_key;
				ID3D11Texture2D* m_pTexture2D{ nullptr };
				ID3D11RenderTargetView* m_pRenderTargetView{ nullptr };
				ID3D11ShaderResourceView* m_pShaderResourceView{ nullptr };
			};
		}
	}
}

namespace std
{
	template <>
	struct hash<eastengine::graphics::dx11::RenderTarget::Key>
	{
		const eastengine::String::StringData* operator()(const eastengine::graphics::dx11::RenderTarget::Key& key) const
		{
			return key.value.Key();
		}
	};
}