#include "stdafx.h"
#include "RenderTarget.h"

namespace eastengine
{
	namespace graphics
	{
		static int s_nRenderTargetCount = 0;

		RenderTarget::RenderTarget()
			: m_nMipLevel(0)
			, m_nMaxMipLevel(1)
			, m_pTexture(nullptr)
			, m_isNeedClear(false)
			, m_colorClear(math::Color::Black)
			, m_keyRenderTarget(String::UnregisteredKey)
		{
		}

		RenderTarget::~RenderTarget()
		{
			if (m_vecRenderTargetView.empty() == false)
			{
				std::for_each(m_vecRenderTargetView.begin(), m_vecRenderTargetView.end(), ReleaseSTLObject());
				m_vecRenderTargetView.clear();
			}

			if (m_vecUav.empty() == false)
			{
				std::for_each(m_vecUav.begin(), m_vecUav.end(), ReleaseSTLObject());
				m_vecUav.clear();
			}

			m_pTexture.reset();
		}

		bool RenderTarget::Init(ID3D11Texture2D* pTexture2D, const RenderTargetDesc2D* pRenderTargetDesc)
		{
			String::StringID strName;
			strName.Format("EastEngine_RenderTarget_%d", s_nRenderTargetCount);

			if (pRenderTargetDesc == nullptr)
			{
				pTexture2D->GetDesc(&m_renderTargetDesc2D);
				m_renderTargetDesc2D.Build();
			}
			else
			{
				m_renderTargetDesc2D = *pRenderTargetDesc;
			}

			m_pTexture = ITexture::Create(strName, pTexture2D, pRenderTargetDesc);
			if (m_pTexture == nullptr)
				return false;

			const uint32_t nMipLevel = m_renderTargetDesc2D.MipLevels;

			m_vecSize.resize(nMipLevel);

			if (m_renderTargetDesc2D.BindFlags & D3D11_BIND_RENDER_TARGET)
			{
				m_vecRenderTargetView.resize(nMipLevel);
			}

			if (m_renderTargetDesc2D.BindFlags & D3D11_BIND_UNORDERED_ACCESS)
			{
				m_vecUav.resize(nMipLevel);
			}

			auto CreateRTAndUAV = [&](uint32_t nIdx)
			{
				if (m_renderTargetDesc2D.BindFlags & D3D11_BIND_RENDER_TARGET)
				{
					const CD3D11_RENDER_TARGET_VIEW_DESC* pRTVDesc = m_renderTargetDesc2D.GetRTVDescPtr(nIdx);

					if (FAILED(GetDevice()->CreateRenderTargetView(m_pTexture->GetTexture2D(), pRTVDesc, &m_vecRenderTargetView[nIdx])))
						return false;
				}

				if (m_renderTargetDesc2D.BindFlags & D3D11_BIND_UNORDERED_ACCESS)
				{
					const CD3D11_UNORDERED_ACCESS_VIEW_DESC* pUAVDesc = m_renderTargetDesc2D.GetUAVDescPtr(nIdx);

					if (FAILED(GetDevice()->CreateUnorderedAccessView(m_pTexture->GetTexture2D(), pUAVDesc, &m_vecUav[nIdx])))
						return false;
				}

				math::UInt2 n2Size(m_renderTargetDesc2D.Width, m_renderTargetDesc2D.Height);
				for (uint32_t i = 0; i < nIdx; ++i)
				{
					n2Size.x = (n2Size.x + 1) / 2;
					n2Size.y = (n2Size.y + 1) / 2;
				}
				n2Size.x = math::Max(n2Size.x, 1u);
				n2Size.y = math::Max(n2Size.y, 1u);

				m_vecSize[nIdx] = n2Size;

				return true;
			};

			for (uint32_t i = 0; i < nMipLevel; ++i)
			{
				if (CreateRTAndUAV(i) == false)
					return false;
			}

			m_keyRenderTarget = m_renderTargetDesc2D.GetKey();

			m_nMipLevel = 0;
			m_nMaxMipLevel = 1;

			++s_nRenderTargetCount;

			return true;
		}

		bool RenderTarget::Init(const RenderTargetDesc1D& renderTargetDesc)
		{
			String::StringID strName;
			strName.Format("EastEngine_RenderTarget_%d", s_nRenderTargetCount);

			m_renderTargetDesc1D = renderTargetDesc;

			if (m_renderTargetDesc1D.MipLevels > 1 && (D3D11_BIND_RENDER_TARGET & m_renderTargetDesc1D.BindFlags) != 0)
			{
				m_renderTargetDesc1D.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
			}

			m_pTexture = ITexture::Create(strName, m_renderTargetDesc1D);
			if (m_pTexture == nullptr)
				return false;

			const uint32_t nMipLevel = m_renderTargetDesc1D.MipLevels;

			m_vecSize.resize(nMipLevel);

			if (m_renderTargetDesc1D.BindFlags & D3D11_BIND_RENDER_TARGET)
			{
				m_vecRenderTargetView.resize(nMipLevel);
			}

			if (m_renderTargetDesc1D.BindFlags & D3D11_BIND_UNORDERED_ACCESS)
			{
				m_vecUav.resize(nMipLevel);
			}

			auto CreateRTAndUAV = [&](uint32_t nIdx)
			{
				if (m_renderTargetDesc1D.BindFlags & D3D11_BIND_RENDER_TARGET)
				{
					const CD3D11_RENDER_TARGET_VIEW_DESC* pRTVDesc = m_renderTargetDesc1D.GetRTVDescPtr(nIdx);

					if (FAILED(GetDevice()->CreateRenderTargetView(m_pTexture->GetTexture1D(), pRTVDesc, &m_vecRenderTargetView[nIdx])))
						return false;
				}

				if (m_renderTargetDesc1D.BindFlags & D3D11_BIND_UNORDERED_ACCESS)
				{
					const CD3D11_UNORDERED_ACCESS_VIEW_DESC* pUAVDesc = m_renderTargetDesc1D.GetUAVDescPtr(nIdx);

					if (FAILED(GetDevice()->CreateUnorderedAccessView(m_pTexture->GetTexture1D(), pUAVDesc, &m_vecUav[nIdx])))
						return false;
				}

				math::UInt2 n2Size(m_renderTargetDesc2D.Width, m_renderTargetDesc2D.Height);
				for (uint32_t i = 0; i < nIdx; ++i)
				{
					n2Size.x = (n2Size.x + 1) / 2;
					n2Size.y = (n2Size.y + 1) / 2;
				}
				n2Size.x = math::Max(n2Size.x, 1u);
				n2Size.y = math::Max(n2Size.y, 1u);

				m_vecSize[nIdx] = n2Size;

				return true;
			};

			for (uint32_t i = 0; i < nMipLevel; ++i)
			{
				if (CreateRTAndUAV(i) == false)
					return false;
			}

			m_keyRenderTarget = m_renderTargetDesc1D.GetKey();
			m_nMipLevel = 0;
			m_nMaxMipLevel = nMipLevel;

			++s_nRenderTargetCount;

			return true;
		}

		bool RenderTarget::Init(const RenderTargetDesc2D& renderTargetDesc)
		{
			String::StringID strName;
			strName.Format("EastEngine_RenderTarget_%d", s_nRenderTargetCount);

			m_renderTargetDesc2D = renderTargetDesc;

			if (m_renderTargetDesc2D.MipLevels > 1 && (D3D11_BIND_RENDER_TARGET & m_renderTargetDesc2D.BindFlags) != 0)
			{
				m_renderTargetDesc2D.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
			}

			m_pTexture = ITexture::Create(strName, m_renderTargetDesc2D);
			if (m_pTexture == nullptr)
				return false;

			uint32_t nMipLevel = m_renderTargetDesc2D.MipLevels;

			m_vecSize.resize(nMipLevel);

			if (m_renderTargetDesc2D.BindFlags & D3D11_BIND_RENDER_TARGET)
			{
				m_vecRenderTargetView.resize(nMipLevel);
			}

			if (m_renderTargetDesc2D.BindFlags & D3D11_BIND_UNORDERED_ACCESS)
			{
				m_vecUav.resize(nMipLevel);
			}

			auto CreateRTAndUAV = [&](uint32_t nIdx)
			{
				if (m_renderTargetDesc2D.BindFlags & D3D11_BIND_RENDER_TARGET)
				{
					const CD3D11_RENDER_TARGET_VIEW_DESC* pRTVDesc = m_renderTargetDesc2D.GetRTVDescPtr(nIdx);

					if (FAILED(GetDevice()->CreateRenderTargetView(m_pTexture->GetTexture2D(), pRTVDesc, &m_vecRenderTargetView[nIdx])))
						return false;
				}

				if (m_renderTargetDesc2D.BindFlags & D3D11_BIND_UNORDERED_ACCESS)
				{
					const CD3D11_UNORDERED_ACCESS_VIEW_DESC* pUAVDesc = m_renderTargetDesc2D.GetUAVDescPtr(nIdx);

					if (FAILED(GetDevice()->CreateUnorderedAccessView(m_pTexture->GetTexture2D(), pUAVDesc, &m_vecUav[nIdx])))
						return false;
				}

				math::UInt2 n2Size(m_renderTargetDesc2D.Width, m_renderTargetDesc2D.Height);
				for (uint32_t i = 0; i < nIdx; ++i)
				{
					n2Size.x = (n2Size.x + 1) / 2;
					n2Size.y = (n2Size.y + 1) / 2;
				}
				n2Size.x = math::Max(n2Size.x, 1u);
				n2Size.y = math::Max(n2Size.y, 1u);

				m_vecSize[nIdx] = n2Size;

				return true;
			};

			for (uint32_t i = 0; i < nMipLevel; ++i)
			{
				if (CreateRTAndUAV(i) == false)
					return false;
			}

			m_keyRenderTarget = m_renderTargetDesc2D.GetKey();
			m_nMipLevel = 0;
			m_nMaxMipLevel = nMipLevel;

			++s_nRenderTargetCount;

			return true;
		}

		void RenderTarget::OnClear(IDeviceContext* pImmediateContext)
		{
			if (m_isNeedClear == true)
			{
				pImmediateContext->ClearRenderTargetView(this, m_colorClear);

				m_isNeedClear = false;
			}
		}
	}
}