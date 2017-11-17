#pragma once

#include "D3DInterface.h"

namespace EastEngine
{
	namespace Graphics
	{
		class RenderTarget : public IRenderTarget
		{
		public:
			RenderTarget();
			virtual ~RenderTarget();

			bool Init(ID3D11Texture2D* pTexture2D, const RenderTargetDesc2D* pRenderTargetDesc = nullptr);
			bool Init(const RenderTargetDesc1D& renderTargetDesc);
			bool Init(const RenderTargetDesc2D& renderTargetDesc);

		public:
			virtual const std::shared_ptr<ITexture>& GetTexture() { return m_pTexture; }
			virtual ID3D11RenderTargetView* GetRenderTargetView() { return m_vecRenderTargetView[std::min(m_nMipLevel, m_nMaxMipLevel)]; }
			virtual ID3D11RenderTargetView** GetRenderTargetViewPtr() { return &m_vecRenderTargetView[0]; }
			virtual ID3D11UnorderedAccessView* GetUnorderedAccessView() { return m_vecUav[std::min(m_nMipLevel, m_nMaxMipLevel)]; }
			
			virtual void SetMipLevel(uint32_t nMipLevel) { m_nMipLevel = std::min(nMipLevel, m_nMaxMipLevel); }
			virtual uint32_t GetMipLevel() { return m_nMipLevel; }
			
			virtual void SetClear(const Math::Color& color) { m_isNeedClear = true; m_colorClear = color; }
			virtual void OnClear(IDeviceContext* pImmediateContext);
			
			virtual const RenderTargetDesc1D& GetDesc1D() { return m_renderTargetDesc1D; }
			virtual const RenderTargetDesc2D& GetDesc2D() { return m_renderTargetDesc2D; }
			virtual const RenderTargetKey& GetKey() { return m_keyRenderTarget; }
			
			virtual const Math::UInt2& GetSize() { return m_vecSize[std::min(m_nMipLevel, m_nMaxMipLevel)]; }

		protected:
			uint32_t m_nMipLevel;
			uint32_t m_nMaxMipLevel;

			RenderTargetDesc1D m_renderTargetDesc1D;
			RenderTargetDesc2D m_renderTargetDesc2D;
			RenderTargetKey m_keyRenderTarget;

			std::shared_ptr<ITexture> m_pTexture;
			std::vector<ID3D11RenderTargetView*> m_vecRenderTargetView;
			std::vector<ID3D11UnorderedAccessView*> m_vecUav;
			std::vector<Math::UInt2> m_vecSize;

			bool m_isNeedClear;
			Math::Color m_colorClear;
		};
	}
}