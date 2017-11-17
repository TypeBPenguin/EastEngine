#pragma once

#include "ISpriteBatch.h"

namespace EastEngine
{
	namespace Graphics
	{
		class CSpriteBatch : public ISpriteBatch
		{
		public:
			CSpriteBatch(ID3D11DeviceContext* pd3dDeviceContext);
			virtual ~CSpriteBatch();

		public:
			virtual void Begin(EmSprite::SortMode emSortMode = EmSprite::eDeferred, IBlendState* pBlendState = nullptr, ISamplerState* pSamplerState = nullptr, IDepthStencilState* pDepthStencilState = nullptr, IRasterizerState* pRasterizerState = nullptr,
				std::function<void DIRECTX_STD_CALLCONV()> setCustomShaders = nullptr, const Math::Matrix& matTransformMatrix = Math::Matrix::Identity) override;
			virtual void __cdecl End() override;

			virtual void Draw(const std::shared_ptr<ITexture>& pTexture, const Math::Vector2& f2Position, const Math::Color& color = Math::Color::White) override;
			virtual void Draw(const std::shared_ptr<ITexture>& pTexture, const Math::Vector2& f2Position, const Math::Rect* pSourceRectangle, const Math::Color& color = Math::Color::White, float fRotation = 0.f, const Math::Vector2& f2Origin = Math::Vector2::Zero, float fScale = 1.f, EmSprite::Effects emEffects = EmSprite::eNone, float fLayerDepth = 0.f) override;
			virtual void Draw(const std::shared_ptr<ITexture>& pTexture, const Math::Vector2& f2Position, const Math::Rect* pSourceRectangle, const Math::Color& color, float fRotation, const Math::Vector2& f2Origin, const Math::Vector2& f2Scale, EmSprite::Effects emEffects = EmSprite::eNone, float fLayerDepth = 0.f) override;

			virtual void Draw(const std::shared_ptr<ITexture>& pTexture, const Math::Rect& destinationRectangle, const Math::Color& color = Math::Color::White) override;
			virtual void Draw(const std::shared_ptr<ITexture>& pTexture, const Math::Rect& destinationRectangle, const Math::Rect* pSourceRectangle, const Math::Color& color = Math::Color::White, float fRotation = 0.f, const Math::Vector2& f2Origin = Math::Vector2::Zero, EmSprite::Effects emEffects = EmSprite::eNone, float fLayerDepth = 0.f) override;

			virtual void __cdecl SetRotation(DXGI_MODE_ROTATION mode) override;
			virtual DXGI_MODE_ROTATION __cdecl GetRotation() const override;

			virtual void __cdecl SetViewport(const Math::Viewport& viewPort) override;

		public:
			virtual DirectX::SpriteBatch* GetInterface() override { return m_pSpriteBatch; }

		private:
			DirectX::SpriteBatch* m_pSpriteBatch;
		};
	}
}