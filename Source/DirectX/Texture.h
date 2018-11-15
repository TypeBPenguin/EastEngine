#pragma once

#include "D3DInterface.h"

namespace eastengine
{
	namespace graphics
	{
		class Texture : public ITexture
		{
		public:
			Texture(const Key& key);
			virtual ~Texture();

		public:
			virtual Key GetKey() const override { return m_key; }

		public:
			virtual void CopySubresourceRegion(ThreadType emThreadID, uint32_t DstSubresource, uint32_t DstX, uint32_t DstY, uint32_t DstZ, ITexture* pSrcResource, uint32_t SrcSubresource, const D3D11_BOX* pSrcBox) override;

			virtual bool Map(ThreadType emThreadID, uint32_t Subresource, D3D11_MAP emMap, void** ppData) const override;
			virtual void Unmap(ThreadType emThreadID, uint32_t Subresource) const override;

		public:
			virtual ID3D11Texture1D* GetTexture1D() override { return m_pTexture1D; }
			virtual ID3D11Texture2D* GetTexture2D() override { return m_pTexture2D; }
			virtual ID3D11Texture3D* GetTexture3D() override { return m_pTexture3D; }
			virtual ID3D11ShaderResourceView* GetShaderResourceView() override { return m_pShaderResourceView; }
			virtual ID3D11ShaderResourceView** GetShaderResourceViewPtr() override { return &m_pShaderResourceView; }

			virtual const math::uint2& GetSize() override { return m_n2Size; }
			virtual const string::StringID& GetName() override { return m_strName; }

		public:
			void SetName(const string::StringID& strName) { m_strName = strName; }

		public:
			bool Load(const string::StringID& strName, ID3D11Texture2D* pTexture2D, const TextureDesc2D* pCustomDesc2D = nullptr);
			bool Load(const string::StringID& strName, const TextureDesc1D& desc, D3D11_SUBRESOURCE_DATA* pData = nullptr);
			bool Load(const string::StringID& strName, const TextureDesc2D& desc, D3D11_SUBRESOURCE_DATA* pData = nullptr);
			bool Load(const string::StringID& strName, const TextureDesc3D& desc, D3D11_SUBRESOURCE_DATA* pData = nullptr);
			bool Load(ID3D11Texture2D* pTexture2D, ID3D11ShaderResourceView* pShaderResourceView);

		protected:
			const Key m_key;

			ID3D11Texture1D* m_pTexture1D;
			ID3D11Texture2D* m_pTexture2D;
			ID3D11Texture3D* m_pTexture3D;
			ID3D11ShaderResourceView* m_pShaderResourceView;

			math::uint2 m_n2Size;

			string::StringID m_strName;
		};
	}
}