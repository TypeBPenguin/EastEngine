#include "stdafx.h"
#include "UIElement.h"

namespace EastEngine
{
	namespace UI
	{
		IUIElement::IUIElement(String::StringID strID)
		{
		}

		IUIElement::~IUIElement()
		{
		}

		CUIElementFont::CUIElementFont(String::StringID strID)
			: IUIElement(strID)
			, m_pFontNode(nullptr)
			, m_emAlignHorizontal(EmUI::eLeft)
			, m_emAlignVertical(EmUI::eTop)
			, m_fRotation(0.f)
			, m_fScale(1.f)
			, m_emSpriteEffect(Graphics::EmSprite::eNone)
		{
		}

		CUIElementFont::~CUIElementFont()
		{
		}

		IUIElement* CUIElementFont::operator = (IUIElement* copyObject)
		{
			if (copyObject->GetType() != EmUI::eFont)
				return nullptr;

			m_pFontNode = copyObject->GetFontNode();
			m_emAlignHorizontal = copyObject->GetAlignHorizontal();
			m_emAlignVertical = copyObject->GetAlignVertical();
			m_fRotation = copyObject->GetRotation();
			m_fScale = copyObject->GetScale();
			m_emSpriteEffect = copyObject->GetSpriteEffect();
			m_emFontEffect = copyObject->GetFontEffect();
			m_effectColor = *copyObject->GetEffectColor();

			m_fontColor = copyObject->GetBlendColor();

			return this;
		}

		void CUIElementFont::Init(CreateUIElementFontInfo* pFontInfo)
		{
			if (pFontInfo == nullptr)
				return;

			m_emAlignHorizontal = pFontInfo->emAlignHorizontal;
			m_emAlignVertical = pFontInfo->emAlignVertical;
			m_fRotation = pFontInfo->fRotation;
			m_fScale = pFontInfo->fScale;
			m_emSpriteEffect = pFontInfo->emSpriteEffect;
			m_emFontEffect = pFontInfo->emFontEffect;
			m_effectColor = pFontInfo->effectColor;
			m_fontColor = pFontInfo->fontColor;
		}

		CUIElementTexture::CUIElementTexture(String::StringID strID)
			: IUIElement(strID)
			, m_pTextureNode(nullptr)
			, m_emSpriteEffect(Graphics::EmSprite::eNone)
		{
			SetRectEmpty(&m_rcTexture);
		}

		CUIElementTexture::~CUIElementTexture()
		{
		}

		IUIElement* CUIElementTexture::operator = (IUIElement* copyObject)
		{
			if (copyObject->GetType() != EmUI::eTexture)
				return nullptr;

			m_rcTexture = *copyObject->GetTextureRect();
			m_pTextureNode = copyObject->GetTextureNode();

			m_textureColor = copyObject->GetBlendColor();

			return this;
		}

		void CUIElementTexture::Init(CreateUIElementTextureInfo* pTextureInfo)
		{
			if (pTextureInfo == nullptr)
				return;

			m_emSpriteEffect = pTextureInfo->emSpriteEffect;
			m_textureColor = pTextureInfo->textureColor;
		}
	}
}