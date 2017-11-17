#pragma once

#include "UIStatic.h"

namespace EastEngine
{
	namespace UI
	{
		class CUIButton : public CUIStatic
		{
		public:
			CUIButton(IUIPanel* pUIPack, String::StringID strID, EmUI::Type emUIType = EmUI::eButton);
			virtual ~CUIButton();

			virtual void Update(float fElapsedTime) override;

			virtual bool HandleKeyboard(uint32_t nMsg, WPARAM wParam, LPARAM lParam) override;
			virtual bool HandleMouse(uint32_t nMsg, POINT pt, WPARAM wParam, LPARAM lParam) override;
		};
	}
}