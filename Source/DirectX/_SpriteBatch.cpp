#include "stdafx.h"
#include "_SpriteBatch.h"

#include "D3DInterface.h"

#include <SpriteBatch.h>

namespace EastEngine
{
	namespace Graphics
	{
		CSpriteBatch::CSpriteBatch(ID3D11DeviceContext* pd3dDeviceContext)
		{
			m_pSpriteBatch = new DirectX::SpriteBatch(pd3dDeviceContext);
		}

		CSpriteBatch::~CSpriteBatch()
		{
			SafeDelete(m_pSpriteBatch);
		}

		void CSpriteBatch::Begin(EmSprite::SortMode emSortMode, IBlendState* pBlendState, ISamplerState* pSamplerState, IDepthStencilState* pDepthStencilState, IRasterizerState* pRasterizerState, std::function<void DIRECTX_STD_CALLCONV()> setCustomShaders, const Math::Matrix& matTransformMatrix)
		{
			m_pSpriteBatch->Begin(static_cast<DirectX::SpriteSortMode>(emSortMode), pBlendState != nullptr ? pBlendState->GetInterface() : nullptr, pSamplerState != nullptr ? pSamplerState->GetInterface() : nullptr, pDepthStencilState != nullptr ? pDepthStencilState->GetInterface() : nullptr, pRasterizerState != nullptr ? pRasterizerState->GetInterface() : nullptr, setCustomShaders, DirectX::XMLoadFloat4x4(reinterpret_cast<const DirectX::XMFLOAT4X4*>(&matTransformMatrix)));
		}

		void __cdecl CSpriteBatch::End()
		{
			m_pSpriteBatch->End();
		}

		void CSpriteBatch::Draw(const std::shared_ptr<ITexture>& pTexture, const Math::Vector2& f2Position, const Math::Color& color)
		{
			m_pSpriteBatch->Draw(pTexture != nullptr ? pTexture->GetShaderResourceView() : nullptr, f2Position, color);
		}

		void CSpriteBatch::Draw(const std::shared_ptr<ITexture>& pTexture, const Math::Vector2& f2Position, const Math::Rect* pSourceRectangle, const Math::Color& color, float fRotation, const Math::Vector2& f2Origin, float fScale, EmSprite::Effects emEffects, float fLayerDepth)
		{
			m_pSpriteBatch->Draw(pTexture != nullptr ? pTexture->GetShaderResourceView() : nullptr, f2Position, pSourceRectangle, color, fRotation, f2Origin, fScale, static_cast<DirectX::SpriteEffects>(emEffects), fLayerDepth);
		}

		void CSpriteBatch::Draw(const std::shared_ptr<ITexture>& pTexture, const Math::Vector2& f2Position, const Math::Rect* pSourceRectangle, const Math::Color& color, float fRotation, const Math::Vector2& f2Origin, const Math::Vector2& f2Scale, EmSprite::Effects emEffects, float fLayerDepth)
		{
			m_pSpriteBatch->Draw(pTexture != nullptr ? pTexture->GetShaderResourceView() : nullptr, f2Position, pSourceRectangle, color, fRotation, f2Origin, f2Scale, static_cast<DirectX::SpriteEffects>(emEffects), fLayerDepth);
		}

		void CSpriteBatch::Draw(const std::shared_ptr<ITexture>& pTexture, const Math::Rect& destinationRectangle, const Math::Color& color)
		{
			m_pSpriteBatch->Draw(pTexture != nullptr ? pTexture->GetShaderResourceView() : nullptr, destinationRectangle, color);
		}

		void CSpriteBatch::Draw(const std::shared_ptr<ITexture>& pTexture, const Math::Rect& destinationRectangle, const Math::Rect* pSourceRectangle, const Math::Color& color, float fRotation, const Math::Vector2& f2Origin, EmSprite::Effects emEffects, float fLayerDepth)
		{
			m_pSpriteBatch->Draw(pTexture != nullptr ? pTexture->GetShaderResourceView() : nullptr, destinationRectangle, pSourceRectangle, color, fRotation, reinterpret_cast<const DirectX::XMFLOAT2&>(f2Origin), static_cast<DirectX::SpriteEffects>(emEffects), fLayerDepth);
		}

		void __cdecl CSpriteBatch::SetRotation(DXGI_MODE_ROTATION mode)
		{
			m_pSpriteBatch->SetRotation(mode);
		}

		DXGI_MODE_ROTATION __cdecl CSpriteBatch::GetRotation() const
		{
			return m_pSpriteBatch->GetRotation();
		}

		void __cdecl CSpriteBatch::SetViewport(const Math::Viewport& viewPort)
		{
			m_pSpriteBatch->SetViewport(*viewPort.Get11());
		}
	}
}