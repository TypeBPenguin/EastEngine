#pragma once

#include "UIObject.h"

namespace EastEngine
{
	namespace UI
	{
		class CUIStatic : public CUIObject
		{
		public:
			CUIStatic(IUIPanel* pUIPanel, String::StringID strID, EmUI::Type emUIType = EmUI::eStatic);
			virtual ~CUIStatic();

			virtual void Update(float fElapsedTime) override;

			virtual bool HandleKeyboard(uint32_t nMsg, WPARAM wParam, LPARAM lParam) override { return false; }
			virtual bool HandleMouse(uint32_t nMsg, POINT pt, WPARAM wParam, LPARAM lParam) override { return false; }

		public:
			void SetText(const char* strText) { if (m_strText != strText) { Invalidate(); } m_strText = strText; }
			const char* GetText() { return m_strText.c_str(); }

		public:
			void SetFontColor(const Math::Color& defaultColor, const Math::Color& disableColor = Math::Color::Gray, const Math::Color& hiddenColor = Math::Color::Transparent) { m_vecElements[0]->SetColor(defaultColor, disableColor, hiddenColor); }
			void SetFontStyle(Graphics::EmSprite::Effects emSpriteEffect) {}

		protected:
			std::string m_strText;
		};
	}
}