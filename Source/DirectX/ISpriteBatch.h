#pragma once

#include "d3dDefine.h"

namespace DirectX
{
	class SpriteBatch;
}

namespace EastEngine
{
	namespace Graphics
	{
		class ITexture;
		class IBlendState;
		class ISamplerState;
		class IDepthStencilState;
		class IRasterizerState;

		class ISpriteBatch
		{
		public:
			ISpriteBatch();
			virtual ~ISpriteBatch() = 0;

		public:
			static ISpriteBatch* Create(ID3D11DeviceContext* pd3dDeviceContext);

		public:
			virtual void Begin(EmSprite::SortMode emSortMode = EmSprite::eDeferred, IBlendState* pBlendState = nullptr, ISamplerState* pSamplerState = nullptr, IDepthStencilState* pDepthStencilState = nullptr, IRasterizerState* pRasterizerState = nullptr,
				std::function<void()> setCustomShaders = nullptr, const Math::Matrix& matTransformMatrix = Math::Matrix::Identity) = 0;
			virtual void __cdecl End() = 0;

			virtual void Draw(const std::shared_ptr<ITexture>& pTexture, const Math::Vector2& f2Position, const Math::Color& color = Math::Color::White) = 0;
			virtual void Draw(const std::shared_ptr<ITexture>& pTexture, const Math::Vector2& f2Position, const Math::Rect* pSourceRectangle, const Math::Color& color = Math::Color::White, float fRotation = 0.f, const Math::Vector2& f2Origin = Math::Vector2::Zero, float fScale = 1.f, EmSprite::Effects emEffects = EmSprite::eNone, float fLayerDepth = 0.f) = 0;
			virtual void Draw(const std::shared_ptr<ITexture>& pTexture, const Math::Vector2& f2Position, const Math::Rect* pSourceRectangle, const Math::Color& color, float fRotation, const Math::Vector2& f2Origin, const Math::Vector2& f2Scale, EmSprite::Effects emEffects = EmSprite::eNone, float fLayerDepth = 0.f) = 0;

			virtual void Draw(const std::shared_ptr<ITexture>& pTexture, const Math::Rect& destinationRectangle, const Math::Color& color = Math::Color::White) = 0;
			virtual void Draw(const std::shared_ptr<ITexture>& pTexture, const Math::Rect& destinationRectangle, const Math::Rect* pSourceRectangle, const Math::Color& color = Math::Color::White, float fRotation = 0.f, const Math::Vector2& origin = Math::Vector2::Zero, EmSprite::Effects emEffects = EmSprite::eNone, float fLayerDepth = 0.f) = 0;

			virtual void __cdecl SetRotation(DXGI_MODE_ROTATION mode) = 0;
			virtual DXGI_MODE_ROTATION __cdecl GetRotation() const = 0;

			virtual void __cdecl SetViewport(const Math::Viewport& viewPort) = 0;

		public:
			virtual DirectX::SpriteBatch* GetInterface() = 0;
		};
	}
}