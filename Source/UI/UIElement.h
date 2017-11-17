#pragma once

#include "UIDefine.h"

namespace EastEngine
{
	namespace Graphics
	{
		class IVertexBuffer;
		class IndexBuffer;
	}

	namespace UI
	{
		namespace EmUI
		{
			enum FontEffect
			{
				eNone = 0,
				eShadow,
				eOutline,

				eFontCount,
			};

			enum ElementType
			{
				eFont = 0,
				eTexture,

				eElementCount,
			};
		}

		struct CreateUIElementFontInfo
		{
			String::StringID strElementName;

			String::StringID strFontName;
			EmUI::AlignHorizontal emAlignHorizontal = EmUI::eLeft;
			EmUI::AlignVertical emAlignVertical = EmUI::eTop;
			float fRotation = 0.f;
			float fScale = 1.f;
			Graphics::EmSprite::Effects emSpriteEffect = Graphics::EmSprite::eNone;
			EmUI::FontEffect emFontEffect = EmUI::eNone;
			Math::Color effectColor = Math::Color::Black;
			UIBlendColor fontColor;
		};

		struct CreateUIElementTextureInfo
		{
			String::StringID strElementName;

			String::StringID strTextureName;
			String::StringID strRectName;
			Graphics::EmSprite::Effects emSpriteEffect = Graphics::EmSprite::eNone;
			UIBlendColor textureColor;
		};

		class IUIElement
		{
		public:
			IUIElement(String::StringID strID);
			virtual ~IUIElement() = 0;

			virtual IUIElement* operator = (IUIElement* copyObject) = 0;

			virtual void Init(CreateUIElementFontInfo* pFontInfo) {}
			virtual void Init(CreateUIElementTextureInfo* pTextureInfo) {}

			virtual void Refresh() = 0;
			virtual bool BlendColor(EmUI::State emState, float fElapsedTime, float fRate = 0.7f) = 0;

		public:
			virtual String::StringID GetID() { return m_strID; }
			virtual EmUI::ElementType GetType() = 0;

			virtual UIBlendColor& GetBlendColor() = 0;
			virtual Math::Color& GetColor() = 0;
			virtual Math::Color& GetColor(EmUI::State emUIState) = 0;
			virtual void SetColor(const Math::Color& defaultColor, const Math::Color& disableColor = Math::Color::Gray, const Math::Color& hiddenColor = Math::Color::Transparent) = 0;
			virtual void SetColorCurrent(const Math::Color& currentColor) = 0;

			virtual Graphics::EmSprite::Effects GetSpriteEffect() = 0;
			virtual void SetSpriteEffect(Graphics::EmSprite::Effects emSpriteEffect) = 0;

			virtual UIFontNode* GetFontNode() = 0;
			virtual void SetFontNode(UIFontNode* pFontNode) = 0;

			virtual EmUI::AlignHorizontal GetAlignHorizontal() = 0;
			virtual void SetAlignHorizontal(EmUI::AlignHorizontal emHorizontal) = 0;

			virtual EmUI::AlignVertical GetAlignVertical() = 0;
			virtual void SetAlignVertical(EmUI::AlignVertical emVertical) = 0;

			virtual float GetRotation() = 0;
			virtual void SetRotation(float fRot) = 0;

			virtual float GetScale() = 0;
			virtual void SetScale(float fScale) = 0;

			virtual EmUI::FontEffect GetFontEffect() = 0;
			virtual void SetFontEffect(EmUI::FontEffect emFontEffects) = 0;

			virtual Math::Color* GetEffectColor() = 0;
			virtual void SetEffectColor(Math::Color& color) = 0;

			virtual Math::Rect* GetTextureRect() = 0;
			virtual void SetTextureRect(Math::Rect& rcTexture) = 0;

			virtual UITextureNode* GetTextureNode() = 0;
			virtual void SetTextureNode(UITextureNode* pTextureNode) = 0;

		private:
			String::StringID m_strID;
		};

		class CUIElementFont : public IUIElement
		{
		public:
			CUIElementFont(String::StringID strID);
			virtual ~CUIElementFont();

			virtual IUIElement* operator = (IUIElement* copyObject) override;

			virtual void Init(CreateUIElementFontInfo* pFontInfo) override;

			virtual void Refresh() override { m_fontColor.current = m_fontColor.states[EmUI::eHidden]; }
			virtual bool BlendColor(EmUI::State emState, float fElapsedTime, float fRate = 0.7f) override { return m_fontColor.Blend(emState, fElapsedTime, fRate); }

		public:
			virtual EmUI::ElementType GetType() override { return EmUI::eFont; }

			virtual UIBlendColor& GetBlendColor() override { return m_fontColor; }
			virtual Math::Color& GetColor() override { return m_fontColor.current; }
			virtual Math::Color& GetColor(EmUI::State emUIState) override { return m_fontColor.states[emUIState]; }
			virtual void SetColor(const Math::Color& defaultColor, const Math::Color& disableColor = Math::Color::Gray, const Math::Color& hiddenColor = Math::Color::Transparent) override { m_fontColor.Init(defaultColor, disableColor, hiddenColor); }
			virtual void SetColorCurrent(const Math::Color& currentColor) override { m_fontColor.current = currentColor; }

			virtual Graphics::EmSprite::Effects GetSpriteEffect() override { return m_emSpriteEffect; }
			virtual void SetSpriteEffect(Graphics::EmSprite::Effects emSpriteEffect) override { m_emSpriteEffect = emSpriteEffect; }

			virtual UIFontNode* GetFontNode() override { return m_pFontNode; }
			virtual void SetFontNode(UIFontNode* pFontNode) override { m_pFontNode = pFontNode; }

			virtual EmUI::AlignHorizontal GetAlignHorizontal() override { return m_emAlignHorizontal; }
			virtual void SetAlignHorizontal(EmUI::AlignHorizontal emAlignHorizontal) override { m_emAlignHorizontal = emAlignHorizontal; }

			virtual EmUI::AlignVertical GetAlignVertical() override { return m_emAlignVertical; }
			virtual void SetAlignVertical(EmUI::AlignVertical emAlignVertical) override { m_emAlignVertical = emAlignVertical; }

			virtual float GetRotation() override { return m_fRotation; }
			virtual void SetRotation(float fRot) override { m_fRotation = fRot; }

			virtual float GetScale() override { return m_fScale; }
			virtual void SetScale(float fScale) override { m_fScale = fScale; }

			virtual EmUI::FontEffect GetFontEffect() override { return m_emFontEffect; }
			virtual void SetFontEffect(EmUI::FontEffect emFontEffects) override { m_emFontEffect = emFontEffects; }

			virtual Math::Color* GetEffectColor() override { return &m_effectColor; }
			virtual void SetEffectColor(Math::Color& color) override { m_effectColor = color; }

			virtual Math::Rect* GetTextureRect() override { return nullptr; }
			virtual void SetTextureRect(Math::Rect& rcTexture) override {}

			virtual UITextureNode* GetTextureNode() override { return nullptr; }
			virtual void SetTextureNode(UITextureNode* pTextureNode) override {}

		private:
			UIFontNode* m_pFontNode;
			EmUI::AlignHorizontal m_emAlignHorizontal;
			EmUI::AlignVertical m_emAlignVertical;
			float m_fRotation;
			float m_fScale;
			Graphics::EmSprite::Effects m_emSpriteEffect;
			EmUI::FontEffect m_emFontEffect;
			Math::Color m_effectColor;

			UIBlendColor m_fontColor;
		};

		class CUIElementTexture : public IUIElement
		{
		public:
			CUIElementTexture(String::StringID strID);
			virtual ~CUIElementTexture();

			virtual IUIElement* operator = (IUIElement* copyObject) override;

			virtual void Init(CreateUIElementTextureInfo* pTextureInfo) override;

			virtual void Refresh() override { m_textureColor.current = m_textureColor.states[(uint32_t)(EmUI::eHidden)]; }
			virtual bool BlendColor(EmUI::State emState, float fElapsedTime, float fRate = 0.7f) override { return m_textureColor.Blend(emState, fElapsedTime, fRate); }

		public:
			virtual EmUI::ElementType GetType() override { return EmUI::eTexture; }

			virtual UIBlendColor& GetBlendColor() override { return m_textureColor; }
			virtual Math::Color& GetColor() override { return m_textureColor.current; }
			virtual Math::Color& GetColor(EmUI::State emUIState) override { return m_textureColor.states[emUIState]; }
			virtual void SetColor(const Math::Color& defaultColor, const Math::Color& disableColor = Math::Color::Gray, const Math::Color& hiddenColor = Math::Color::Transparent) override { m_textureColor.Init(defaultColor, disableColor, hiddenColor); }
			virtual void SetColorCurrent(const Math::Color& currentColor) override { m_textureColor.current = currentColor; }

			virtual Graphics::EmSprite::Effects GetSpriteEffect() override { return m_emSpriteEffect; }
			virtual void SetSpriteEffect(Graphics::EmSprite::Effects emSpriteEffect) override { m_emSpriteEffect = emSpriteEffect; }

			virtual UIFontNode* GetFontNode() override { return nullptr; }
			virtual void SetFontNode(UIFontNode* pFontNode) override {}

			virtual EmUI::AlignHorizontal GetAlignHorizontal() override { return EmUI::eLeft; }
			virtual void SetAlignHorizontal(EmUI::AlignHorizontal emAlignHorizontal) override {}

			virtual EmUI::AlignVertical GetAlignVertical() override { return EmUI::eTop; }
			virtual void SetAlignVertical(EmUI::AlignVertical emAlignVertical) override {}

			virtual float GetRotation() override { return 0.f; }
			virtual void SetRotation(float fRot) override {}

			virtual float GetScale() override { return 0.f; }
			virtual void SetScale(float fScale) override {}

			virtual EmUI::FontEffect GetFontEffect() override { return EmUI::eNone; }
			virtual void SetFontEffect(EmUI::FontEffect emFontEffects) override {}

			virtual Math::Color* GetEffectColor() override { return nullptr; }
			virtual void SetEffectColor(Math::Color& color) override {}

			virtual Math::Rect* GetTextureRect() override { return &m_rcTexture; }
			virtual void SetTextureRect(Math::Rect& rcTexture) override { m_rcTexture = rcTexture; }

			virtual UITextureNode* GetTextureNode() override { return m_pTextureNode; }
			virtual void SetTextureNode(UITextureNode* pTextureNode) override { m_pTextureNode = pTextureNode; }

		private:
			Math::Rect m_rcTexture;
			UITextureNode* m_pTextureNode;

			Graphics::EmSprite::Effects m_emSpriteEffect;

			UIBlendColor m_textureColor;
		};
	}
}