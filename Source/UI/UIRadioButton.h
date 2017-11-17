#pragma once

#include "UICheckBox.h"

namespace EastEngine
{
	namespace UI
	{
		class CUIRadioButton : public CUICheckBox
		{
		public:
			CUIRadioButton(IUIPanel* pPanel, String::StringID strID, EmUI::Type emUIType = EmUI::eRadioButton);
			virtual ~CUIRadioButton();

			virtual bool HandleKeyboard(uint32_t nMsg, WPARAM wParam, LPARAM lParam);
			virtual bool HandleMouse(uint32_t nMsg, POINT pt, WPARAM wParam, LPARAM lParam);
			virtual void OnHotkey();

			void SetChecked(bool bChecked, bool bClearGroup = true) { setCheckedInternal(bChecked, bClearGroup, false); }
			void SetButtonGroup(uint32_t nButtonGroup) { m_nButtonGroup = nButtonGroup; }
			uint32_t GetButtonGroup() { return m_nButtonGroup; }

		protected:
			virtual void setCheckedInternal(bool bChecked, bool bClearGroup, bool bFromInput);

		protected:
			uint32_t m_nButtonGroup;
		};
	}
}