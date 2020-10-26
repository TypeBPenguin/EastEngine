#include "stdafx.h"
#include "RenderTargetDX11.h"

#include "DeviceDX11.h"

namespace est
{
	namespace graphics
	{
		namespace dx11
		{
			RenderTarget::RenderTarget(const Key& key)
				: m_key(key)
			{
			}

			RenderTarget::~RenderTarget()
			{
				SafeRelease(m_pShaderResourceView);
				SafeRelease(m_pRenderTargetView);
				SafeRelease(m_pTexture2D);
			}

			std::unique_ptr<RenderTarget> RenderTarget::Create(ID3D11Texture2D* pTexture2D)
			{
				ID3D11Device* pDevice = Device::GetInstance()->GetInterface();

				ID3D11RenderTargetView* pRenderTargetView = nullptr;
				HRESULT hr = pDevice->CreateRenderTargetView(pTexture2D, nullptr, &pRenderTargetView);
				if (FAILED(hr))
				{
					throw_line("failed to create render target view");
				}

				D3D11_TEXTURE2D_DESC desc{};
				pTexture2D->GetDesc(&desc);

				Key key = BuildKey(&desc);

				if ((desc.BindFlags & D3D11_BIND_SHADER_RESOURCE) == D3D11_BIND_SHADER_RESOURCE)
				{
					ID3D11ShaderResourceView* pShaderResourceView = nullptr;
					CD3D11_SHADER_RESOURCE_VIEW_DESC srvDesc(pTexture2D, (desc.ArraySize == 1) ? D3D11_SRV_DIMENSION_TEXTURE2D : D3D11_SRV_DIMENSION_TEXTURE2DARRAY);
					hr = pDevice->CreateShaderResourceView(pTexture2D, &srvDesc, &pShaderResourceView);
					if (FAILED(hr))
					{
						SafeRelease(pRenderTargetView);
						LOG_ERROR(L"failed to create ShaderResourceView");
						return {};
					}

					std::unique_ptr<RenderTarget> pRenderTarget = std::make_unique<RenderTarget>(key);
					pRenderTarget->m_pTexture2D = pTexture2D;
					pRenderTarget->m_pTexture2D->AddRef();

					pRenderTarget->m_pRenderTargetView = pRenderTargetView;
					pRenderTarget->m_pShaderResourceView = pShaderResourceView;

					return pRenderTarget;
				}
				else
				{
					std::unique_ptr<RenderTarget> pRenderTarget = std::make_unique<RenderTarget>(key);
					pRenderTarget->m_pTexture2D = pTexture2D;
					pRenderTarget->m_pTexture2D->AddRef();

					pRenderTarget->m_pRenderTargetView = pRenderTargetView;

					return pRenderTarget;
				}
			}

			std::unique_ptr<RenderTarget> RenderTarget::Create(const D3D11_TEXTURE2D_DESC* pDesc)
			{
				if ((pDesc->BindFlags & D3D11_BIND_RENDER_TARGET) != D3D11_BIND_RENDER_TARGET)
				{
					LOG_ERROR(L"RenderTarget's BindFlags is required include D3D11_BIND_RENDER_TARGET flag");
					return {};
				}

				ID3D11Device* pDevice = Device::GetInstance()->GetInterface();

				ID3D11Texture2D* pTexture2D = nullptr;
				HRESULT hr = pDevice->CreateTexture2D(pDesc, nullptr, &pTexture2D);
				if (FAILED(hr))
				{
					LOG_ERROR(L"failed to create Texture2D");
					return {};
				}

				ID3D11RenderTargetView* pRenderTargetView = nullptr;
				CD3D11_RENDER_TARGET_VIEW_DESC rtvDesc(pTexture2D, (pDesc->ArraySize == 1) ? D3D11_RTV_DIMENSION_TEXTURE2D : D3D11_RTV_DIMENSION_TEXTURE2DARRAY);
				hr = pDevice->CreateRenderTargetView(pTexture2D, &rtvDesc, &pRenderTargetView);
				if (FAILED(hr))
				{
					SafeRelease(pTexture2D);
					LOG_ERROR(L"failed to create RenderTargetView");
					return {};
				}

				Key key = BuildKey(pDesc);

				if ((pDesc->BindFlags & D3D11_BIND_SHADER_RESOURCE) == D3D11_BIND_SHADER_RESOURCE)
				{
					ID3D11ShaderResourceView* pShaderResourceView = nullptr;
					CD3D11_SHADER_RESOURCE_VIEW_DESC srvDesc(pTexture2D, (pDesc->ArraySize == 1) ? D3D11_SRV_DIMENSION_TEXTURE2D : D3D11_SRV_DIMENSION_TEXTURE2DARRAY);
					hr = pDevice->CreateShaderResourceView(pTexture2D, &srvDesc, &pShaderResourceView);
					if (FAILED(hr))
					{
						SafeRelease(pTexture2D);
						SafeRelease(pRenderTargetView);
						LOG_ERROR(L"failed to create ShaderResourceView");
						return {};
					}

					std::unique_ptr<RenderTarget> pRenderTarget = std::make_unique<RenderTarget>(key);
					pRenderTarget->m_pTexture2D = pTexture2D;
					pRenderTarget->m_pRenderTargetView = pRenderTargetView;
					pRenderTarget->m_pShaderResourceView = pShaderResourceView;

					return pRenderTarget;
				}
				else
				{
					std::unique_ptr<RenderTarget> pRenderTarget = std::make_unique<RenderTarget>(key);
					pRenderTarget->m_pTexture2D = pTexture2D;
					pRenderTarget->m_pRenderTargetView = pRenderTargetView;

					return pRenderTarget;
				}
			}

			RenderTarget::Key RenderTarget::BuildKey(const D3D11_TEXTURE2D_DESC* pDesc)
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

				return RenderTarget::Key(strKey);
			}

			void RenderTarget::GetDesc2D(D3D11_TEXTURE2D_DESC* pDesc) const
			{
				m_pTexture2D->GetDesc(pDesc);
			}
		}
	}
}