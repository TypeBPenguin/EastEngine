#include "stdafx.h"
#include "Texture.h"

#include "TextureManager.h"

namespace eastengine
{
	namespace graphics
	{
		Texture::Texture(const Key& key)
			: m_key(key)
			, m_pTexture1D(nullptr)
			, m_pTexture2D(nullptr)
			, m_pTexture3D(nullptr)
			, m_pShaderResourceView(nullptr)
		{
		}

		Texture::~Texture()
		{
			SafeRelease(m_pTexture1D);
			SafeRelease(m_pTexture2D);
			SafeRelease(m_pTexture3D);
			SafeRelease(m_pShaderResourceView);
		}

		bool Texture::Load(const string::StringID& strName, ID3D11Texture2D* pTexture2D, const TextureDesc2D* pCustomDesc2D)
		{
			m_pTexture2D = pTexture2D;
			m_pTexture2D->AddRef();

			TextureDesc2D desc;
			if (pCustomDesc2D != nullptr)
			{
				desc = *pCustomDesc2D;
			}
			else
			{
				m_pTexture2D->GetDesc(&desc);
				desc.Build();
			}

			if (desc.BindFlags & D3D11_BIND_SHADER_RESOURCE)
			{
				if (FAILED(GetDevice()->CreateShaderResourceView(m_pTexture2D, desc.GetSRVDescPtr(), &m_pShaderResourceView)))
					return false;
			}

			m_n2Size.x = desc.Width;
			m_n2Size.y = desc.Height;
			m_strName = strName;

			SetAlive(true);

			return true;
		}

		bool Texture::Load(const string::StringID& strName, const TextureDesc1D& desc, D3D11_SUBRESOURCE_DATA* pData)
		{
			if (FAILED(GetDevice()->CreateTexture1D(&desc, pData, &m_pTexture1D)))
				return false;

			GetDevice()->SetDebugName(m_pTexture1D, strName.c_str());

			if (desc.BindFlags & D3D11_BIND_SHADER_RESOURCE)
			{
				if (FAILED(GetDevice()->CreateShaderResourceView(m_pTexture1D, desc.GetSRVDescPtr(), &m_pShaderResourceView)))
					return false;
			}

			m_n2Size.x = desc.Width;
			m_strName = strName;

			SetAlive(true);

			return true;
		}

		bool Texture::Load(const string::StringID& strName, const TextureDesc2D& desc, D3D11_SUBRESOURCE_DATA* pData)
		{
			if (FAILED(GetDevice()->CreateTexture2D(&desc, pData, &m_pTexture2D)))
				return false;

			GetDevice()->SetDebugName(m_pTexture2D, strName.c_str());

			if (desc.BindFlags & D3D11_BIND_SHADER_RESOURCE)
			{
				if (FAILED(GetDevice()->CreateShaderResourceView(m_pTexture2D, desc.GetSRVDescPtr(), &m_pShaderResourceView)))
					return false;
			}

			m_n2Size.x = desc.Width;
			m_n2Size.y = desc.Height;
			m_strName = strName;

			SetAlive(true);

			return true;
		}

		bool Texture::Load(const string::StringID& strName, const TextureDesc3D& desc, D3D11_SUBRESOURCE_DATA* pData)
		{
			if (FAILED(GetDevice()->CreateTexture3D(&desc, pData, &m_pTexture3D)))
				return false;

			GetDevice()->SetDebugName(m_pTexture3D, strName.c_str());

			if (desc.BindFlags & D3D11_BIND_SHADER_RESOURCE)
			{
				if (FAILED(GetDevice()->CreateShaderResourceView(m_pTexture3D, desc.GetSRVDescPtr(), &m_pShaderResourceView)))
					return false;
			}

			m_n2Size.x = desc.Width;
			m_n2Size.y = desc.Height;
			m_strName = strName;

			SetAlive(true);

			return true;
		}

		bool Texture::Load(ID3D11Texture2D* pTexture2D, ID3D11ShaderResourceView* pShaderResourceView)
		{
			m_pTexture2D = pTexture2D;
			m_pShaderResourceView = pShaderResourceView;

			graphics::TextureDesc2D desc;
			m_pTexture2D->GetDesc(&desc);

			m_n2Size.x = desc.Width;
			m_n2Size.y = desc.Height;

			SetAlive(true);

			return true;
		}

		void Texture::CopySubresourceRegion(ThreadType emThreadID, uint32_t DstSubresource, uint32_t DstX, uint32_t DstY, uint32_t DstZ, ITexture* pSrcResource, uint32_t SrcSubresource, const D3D11_BOX* pSrcBox)
		{
			if (m_pTexture2D != nullptr && pSrcResource != nullptr)
			{
				GetDeferredContext(emThreadID)->CopySubresourceRegion(m_pTexture2D, DstSubresource, DstX, DstY, DstZ, pSrcResource->GetTexture2D(), SrcSubresource, pSrcBox);
			}
		}

		bool Texture::Map(ThreadType emThreadID, uint32_t Subresource, D3D11_MAP emMap, void** ppData) const
		{
			D3D11_MAPPED_SUBRESOURCE map;
			Memory::Clear(&map, sizeof(D3D11_MAPPED_SUBRESOURCE));
			
			HRESULT hr = GetDeferredContext(emThreadID)->Map(m_pTexture2D, Subresource, emMap, 0, &map);
			if (FAILED(hr))
			{
				*ppData = nullptr;
				return false;
			}

			*ppData = map.pData;
			return true;
		}

		void Texture::Unmap(ThreadType emThreadID, uint32_t Subresource) const
		{
			GetDeferredContext(emThreadID)->Unmap(m_pTexture2D, Subresource);
		}
	}
}