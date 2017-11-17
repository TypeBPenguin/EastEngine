#pragma once

#include "UIButton.h"

namespace EastEngine
{
	namespace UI
	{
		class CUICheckBox : public CUIButton
		{
		public:
			CUICheckBox(IUIPanel* pPanel, String::StringID strID, EmUI::Type emUIType = EmUI::eCheckBox);
			virtual ~CUICheckBox();

			virtual void Update(float fElapsedTime) override;

			virtual bool HandleKeyboard(uint32_t nMsg, WPARAM wParam, LPARAM lParam) override;
			virtual bool HandleMouse(uint32_t nMsg, POINT pt, WPARAM wParam, LPARAM lParam) override;

		public:
			bool IsChecked() { return m_bChecked; }
			void SetChecked(bool bChecked) { setCheckedInternal(bChecked, false); }

			virtual void UpdateRects();

		protected:
			virtual void setCheckedInternal(bool bChecked, bool bFromInput);

		protected:
			bool m_bChecked;

			Math::Rect m_rcButton;
			Math::Rect m_rcText;
		};
	}
}