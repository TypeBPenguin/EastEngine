#include "stdafx.h"
#include "_SpriteFont.h"

#include "D3DInterface.h"

#include <SpriteFont.h>
#include <SpriteBatch.h>

namespace EastEngine
{
	namespace Graphics
	{
		CSpriteFont::CSpriteFont(const char* strFileName)
		{
			m_pSpriteFont = new DirectX::SpriteFont(GetDevice()->GetInterface(), String::MultiToWide(strFileName).c_str());
		}

		CSpriteFont::CSpriteFont(const uint8_t* pDataBlob, uint32_t nDataSize)
		{
			m_pSpriteFont = new DirectX::SpriteFont(GetDevice()->GetInterface(), pDataBlob, nDataSize);
		}

		CSpriteFont::CSpriteFont(const std::shared_ptr<ITexture>& pTexture, const Glyph* pGlyphs, uint32_t nGlyphCount, float fLineSpacing)
		{
			m_pSpriteFont = new DirectX::SpriteFont(pTexture->GetShaderResourceView(), reinterpret_cast<const DirectX::SpriteFont::Glyph*>(pGlyphs), nGlyphCount, fLineSpacing);
		}

		CSpriteFont::~CSpriteFont()
		{
			SafeDelete(m_pSpriteFont);
		}

		void CSpriteFont::DrawString(ISpriteBatch* pSpriteBatch, const wchar_t* strText, const Math::Vector2& f2Pos, const Math::Color& color, float fRotation, const Math::Vector2& f2Origin, float fScale, EmSprite::Effects emEffects, float fLayerDepth) const
		{
			m_pSpriteFont->DrawString(pSpriteBatch->GetInterface(), strText, f2Pos, color, fRotation, f2Origin, fScale, static_cast<DirectX::SpriteEffects>(emEffects), fLayerDepth);
		}

		void CSpriteFont::DrawString(ISpriteBatch* pSpriteBatch, const wchar_t* strText, const Math::Vector2& f2Pos, const Math::Color& color, float fRotation, const Math::Vector2& f2Origin, const Math::Vector2& f2Scale, EmSprite::Effects emEffects, float fLayerDepth) const
		{
			m_pSpriteFont->DrawString(pSpriteBatch->GetInterface(), strText, f2Pos, color, fRotation, f2Origin, f2Scale, static_cast<DirectX::SpriteEffects>(emEffects), fLayerDepth);
		}

		void CSpriteFont::DrawString(ISpriteBatch* pSpriteBatch, const wchar_t* strText, const Math::Rect& rect, const Math::Color& color, float fRotation, const Math::Vector2& f2Origin, float fScale, EmSprite::Effects emEffects, float fLayerDepth) const
		{
			m_pSpriteFont->DrawString(pSpriteBatch->GetInterface(), strText, rect, color, fRotation, reinterpret_cast<const DirectX::XMFLOAT2&>(f2Origin), fScale, static_cast<DirectX::SpriteEffects>(emEffects), fLayerDepth);
		}

		void CSpriteFont::DrawString(ISpriteBatch* pSpriteBatch, const wchar_t* strText, const Math::Rect& rect, const Math::Color& color, float fRotation, const Math::Vector2& f2Origin, const Math::Vector2& f2Scale, EmSprite::Effects emEffects, float fLayerDepth) const
		{
			m_pSpriteFont->DrawString(pSpriteBatch->GetInterface(), strText, rect, color, fRotation, f2Origin, f2Scale, static_cast<DirectX::SpriteEffects>(emEffects), fLayerDepth);
		}

		Math::Vector2 CSpriteFont::MeasureString(const wchar_t* strText) const
		{
			return m_pSpriteFont->MeasureString(strText);
		}

		float __cdecl CSpriteFont::GetLineSpacing() const
		{
			return m_pSpriteFont->GetLineSpacing();
		}

		void __cdecl CSpriteFont::SetLineSpacing(float spacing)
		{
			m_pSpriteFont->SetLineSpacing(spacing);
		}

		wchar_t __cdecl CSpriteFont::GetDefaultCharacter() const
		{
			return m_pSpriteFont->GetDefaultCharacter();
		}

		void __cdecl CSpriteFont::SetDefaultCharacter(wchar_t character)
		{
			m_pSpriteFont->SetDefaultCharacter(character);
		}

		bool __cdecl CSpriteFont::ContainsCharacter(wchar_t character) const
		{
			return m_pSpriteFont->ContainsCharacter(character);
		}

		const CSpriteFont::Glyph* __cdecl CSpriteFont::FindGlyph(wchar_t character) const
		{
			return reinterpret_cast<const CSpriteFont::Glyph*>(m_pSpriteFont->FindGlyph(character));;
		}

		void CSpriteFont::GetSpriteSheet(const std::shared_ptr<ITexture>* ppTexture) const
		{
			m_pSpriteFont->GetSpriteSheet((*ppTexture)->GetShaderResourceViewPtr());
		}
	}
}