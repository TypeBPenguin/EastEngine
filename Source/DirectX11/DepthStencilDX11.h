#pragma once

namespace eastengine
{
	namespace graphics
	{
		namespace dx11
		{
			class DepthStencil
			{
			public:
				DepthStencil();
				~DepthStencil();

			public:
				static std::unique_ptr<DepthStencil> Create(const D3D11_TEXTURE2D_DESC* pDesc);

			public:
				bool GetDesc2D(D3D11_TEXTURE2D_DESC* pDesc) const;

				ID3D11Texture2D* GetTexture2D() const { return m_pTexture2D; }
				ID3D11DepthStencilView* GetDepthStencilView() const { return m_pDepthStencilView; }
				ID3D11ShaderResourceView* GetShaderResourceView() const { return m_pShaderResourceView; }

			private:
				ID3D11Texture2D* m_pTexture2D{ nullptr };
				ID3D11DepthStencilView * m_pDepthStencilView{ nullptr };
				ID3D11ShaderResourceView* m_pShaderResourceView{ nullptr };
			};
		}
	}
}