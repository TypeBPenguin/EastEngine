#pragma once

#include "ISpriteBatch.h"

namespace DirectX
{
	class SpriteFont;
}

namespace EastEngine
{
	namespace Graphics
	{
		class ITexture;
		class ISpriteBatch;

		class ISpriteFont
		{
		public:
			struct Glyph
			{
				uint32_t Character;
				Math::Rect Subrect;
				float XOffset;
				float YOffset;
				float XAdvance;
			};

		public:
			ISpriteFont();
			virtual ~ISpriteFont() = 0;

		public:
			static ISpriteFont* Create(const char* strFileName);
			static ISpriteFont* Create(const uint8_t* pDataBlob, uint32_t nDataSize);
			static ISpriteFont* Create(const std::shared_ptr<ITexture>& pTexture, const Glyph* pGlyphs, uint32_t nGlyphCount, float fLineSpacing);

		public:
			virtual void DrawString(ISpriteBatch* pSpriteBatch, const wchar_t* strText, const Math::Vector2& f2Pos, const Math::Color& color = Math::Color::White, float fRotation = 0.f, const Math::Vector2& f2Origin = Math::Vector2::Zero, float fScale = 1.f, EmSprite::Effects emEffects = EmSprite::eNone, float fLayerDepth = 0.f) const = 0;
			virtual void DrawString(ISpriteBatch* pSpriteBatch, const wchar_t* strText, const Math::Vector2& f2Pos, const Math::Color& color, float fRotation, const Math::Vector2& f2Origin, const Math::Vector2& f2Scale, EmSprite::Effects emEffects = EmSprite::eNone, float fLayerDepth = 0.f) const = 0;
			virtual void DrawString(ISpriteBatch* pSpriteBatch, const wchar_t* strText, const Math::Rect& rect, const Math::Color& color = Math::Color::White, float fRotation = 0.f, const Math::Vector2& f2Origin = Math::Vector2::Zero, float fScale = 1.f, EmSprite::Effects emEffects = EmSprite::eNone, float fLayerDepth = 0.f) const = 0;
			virtual void DrawString(ISpriteBatch* pSpriteBatch, const wchar_t* strText, const Math::Rect& rect, const Math::Color& color, float fRotation, const Math::Vector2& f2Origin, const Math::Vector2& f2Scale, EmSprite::Effects emEffects, float fLayerDepth) const = 0;

			virtual Math::Vector2 MeasureString(const wchar_t* strText) const = 0;

			virtual float __cdecl GetLineSpacing() const = 0;
			virtual void __cdecl SetLineSpacing(float spacing) = 0;

			virtual wchar_t __cdecl GetDefaultCharacter() const = 0;
			virtual void __cdecl SetDefaultCharacter(wchar_t character) = 0;

			virtual bool __cdecl ContainsCharacter(wchar_t character) const = 0;

			virtual const Glyph* __cdecl FindGlyph(wchar_t character) const = 0;
			virtual void GetSpriteSheet(const std::shared_ptr<ITexture>* ppTexture) const = 0;
		};
	}
}