#pragma once

#include "ISpriteFont.h"

namespace EastEngine
{
	namespace Graphics
	{
		class ITexture;

		class CSpriteFont : public ISpriteFont
		{
		public:
			CSpriteFont(const char* strFileName);
			CSpriteFont(const uint8_t* pDataBlob, uint32_t nDataSize);
			CSpriteFont(const std::shared_ptr<ITexture>& pTexture, const Glyph* pGlyphs, uint32_t nGlyphCount, float fLineSpacing);
			virtual ~CSpriteFont();

		public:
			virtual void DrawString(ISpriteBatch* pSpriteBatch, const wchar_t* strText, const Math::Vector2& f2Pos, const Math::Color& color = Math::Color::White, float fRotation = 0.f, const Math::Vector2& f2Origin = Math::Vector2::Zero, float fScale = 1.f, EmSprite::Effects emEffects = EmSprite::eNone, float fLayerDepth = 0.f) const override;
			virtual void DrawString(ISpriteBatch* pSpriteBatch, const wchar_t* strText, const Math::Vector2& f2Pos, const Math::Color& color, float fRotation, const Math::Vector2& f2Origin, const Math::Vector2& f2Scale, EmSprite::Effects emEffects = EmSprite::eNone, float fLayerDepth = 0.f) const override;
			virtual void DrawString(ISpriteBatch* pSpriteBatch, const wchar_t* strText, const Math::Rect& rect, const Math::Color& color = Math::Color::White, float fRotation = 0.f, const Math::Vector2& f2Origin = Math::Vector2::Zero, float fScale = 1.f, EmSprite::Effects emEffects = EmSprite::eNone, float fLayerDepth = 0.f) const override;
			virtual void DrawString(ISpriteBatch* pSpriteBatch, const wchar_t* strText, const Math::Rect& rect, const Math::Color& color, float fRotation, const Math::Vector2& f2Origin, const Math::Vector2& f2Scale, EmSprite::Effects emEffects, float fLayerDepth) const override;

			virtual Math::Vector2 MeasureString(const wchar_t* strText) const override;

			// Spacing properties
			virtual float __cdecl GetLineSpacing() const override;
			virtual void __cdecl SetLineSpacing(float spacing) override;

			// Font properties
			virtual wchar_t __cdecl GetDefaultCharacter() const override;
			virtual void __cdecl SetDefaultCharacter(wchar_t character) override;

			virtual bool __cdecl ContainsCharacter(wchar_t character) const override;

			// Custom layout/rendering
			virtual const Glyph* __cdecl FindGlyph(wchar_t character) const override;
			virtual void GetSpriteSheet(const std::shared_ptr<ITexture>* ppTexture) const override;

		private:
			DirectX::SpriteFont* m_pSpriteFont;
		};
	}
}