#include "stdafx.h"
#include "DepthStencilDX11.h"

#include "DeviceDX11.h"

namespace est
{
	namespace graphics
	{
		namespace dx11
		{
			DepthStencil::DepthStencil(const Key& key)
				: m_key(key)
			{
			}

			DepthStencil::~DepthStencil()
			{
				SafeRelease(m_pShaderResourceView);
				SafeRelease(m_pDepthStencilView);
				SafeRelease(m_pTexture2D);
			}

			std::unique_ptr<DepthStencil> DepthStencil::Create(const D3D11_TEXTURE2D_DESC* pDesc)
			{
				if ((pDesc->BindFlags & D3D11_BIND_DEPTH_STENCIL) != D3D11_BIND_DEPTH_STENCIL)
				{
					LOG_ERROR(L"DepthStencil's BindFlags is required include D3D11_BIND_DEPTH_STENCIL flag");
					return {};
				}

				D3D11_TEXTURE2D_DESC desc = *pDesc;

				DXGI_FORMAT dsvFormat = DXGI_FORMAT_UNKNOWN;;
				DXGI_FORMAT srvFormat = DXGI_FORMAT_UNKNOWN;
				switch (desc.Format)
				{
				case DXGI_FORMAT_R32_TYPELESS:
					dsvFormat = DXGI_FORMAT_D32_FLOAT;
					srvFormat = DXGI_FORMAT_R32_FLOAT;
					break;
				case DXGI_FORMAT_D24_UNORM_S8_UINT:
					dsvFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
					srvFormat = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
					break;
				case DXGI_FORMAT_R24G8_TYPELESS:
					dsvFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
					srvFormat = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
					break;
				case DXGI_FORMAT_D32_FLOAT:
					desc.Format = DXGI_FORMAT_R32_TYPELESS;
					dsvFormat = DXGI_FORMAT_D32_FLOAT;
					srvFormat = DXGI_FORMAT_R32_FLOAT;
					break;
				case DXGI_FORMAT_R16_TYPELESS:
					srvFormat = DXGI_FORMAT_R16_UNORM;
					dsvFormat = DXGI_FORMAT_D16_UNORM;
					break;
				case DXGI_FORMAT_R8_TYPELESS:
					srvFormat = DXGI_FORMAT_R8_UNORM;
					dsvFormat = DXGI_FORMAT_R8_UNORM;
					break;
				}

				ID3D11Device* pDevice = Device::GetInstance()->GetInterface();

				ID3D11Texture2D* pTexture2D = nullptr;
				HRESULT hr = pDevice->CreateTexture2D(&desc, nullptr, &pTexture2D);
				if (FAILED(hr))
				{
					LOG_ERROR(L"failed to create Texture2D");
					return {};
				}

				ID3D11DepthStencilView* pDepthStencilView = nullptr;
				CD3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc(pTexture2D, (desc.ArraySize == 1) ? D3D11_DSV_DIMENSION_TEXTURE2D : D3D11_DSV_DIMENSION_TEXTURE2DARRAY);
				dsvDesc.Format = dsvFormat;
				hr = pDevice->CreateDepthStencilView(pTexture2D, &dsvDesc, &pDepthStencilView);
				if (FAILED(hr))
				{
					SafeRelease(pTexture2D);
					LOG_ERROR(L"failed to create DepthStencilView");
					return {};
				}

				Key key = BuildKey(pDesc);

				if ((desc.BindFlags & D3D11_BIND_SHADER_RESOURCE) == D3D11_BIND_SHADER_RESOURCE)
				{
					ID3D11ShaderResourceView* pShaderResourceView = nullptr;
					CD3D11_SHADER_RESOURCE_VIEW_DESC srvDesc(pTexture2D, (desc.ArraySize == 1) ? D3D11_SRV_DIMENSION_TEXTURE2D : D3D11_SRV_DIMENSION_TEXTURE2DARRAY);
					srvDesc.Format = srvFormat;
					hr = pDevice->CreateShaderResourceView(pTexture2D, &srvDesc, &pShaderResourceView);
					if (FAILED(hr))
					{
						SafeRelease(pTexture2D);
						SafeRelease(pDepthStencilView);
						LOG_ERROR(L"failed to create ShaderResourceView");
						return {};
					}

					std::unique_ptr<DepthStencil> pDepthStencil = std::make_unique<DepthStencil>(key);
					pDepthStencil->m_pTexture2D = pTexture2D;
					pDepthStencil->m_pDepthStencilView = pDepthStencilView;
					pDepthStencil->m_pShaderResourceView = pShaderResourceView;

					return pDepthStencil;
				}
				else
				{
					std::unique_ptr<DepthStencil> pDepthStencil = std::make_unique<DepthStencil>(key);
					pDepthStencil->m_pTexture2D = pTexture2D;
					pDepthStencil->m_pDepthStencilView = pDepthStencilView;

					return pDepthStencil;
				}
			}

			DepthStencil::Key DepthStencil::BuildKey(const D3D11_TEXTURE2D_DESC* pDesc)
			{
				string::StringID strKey;
				strKey.Format(L"RenderTarget_%u_%u_%u_%u_%u_%u_%u_%u_%u_%u_%u",
					pDesc->Width,
					pDesc->Height,
					pDesc->MipLevels,
					pDesc->ArraySize,
					pDesc->Format,
					pDesc->SampleDesc.Count,
					pDesc->SampleDesc.Quality,
					pDesc->Usage,
					pDesc->BindFlags,
					pDesc->CPUAccessFlags,
					pDesc->MiscFlags);

				return DepthStencil::Key(strKey);
			}

			void DepthStencil::GetDesc2D(D3D11_TEXTURE2D_DESC* pDesc) const
			{
				m_pTexture2D->GetDesc(pDesc);
			}
		}
	}
}