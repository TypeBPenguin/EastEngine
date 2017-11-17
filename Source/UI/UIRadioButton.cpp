#include "stdafx.h"
#include "UIRadioButton.h"

#include "IUIPanel.h"

namespace EastEngine
{
	namespace UI
	{
		CUIRadioButton::CUIRadioButton(IUIPanel* pPanel, String::StringID strID, EmUI::Type emUIType)
			: CUICheckBox(pPanel, strID, emUIType)
		{
		}

		CUIRadioButton::~CUIRadioButton()
		{
		}

		bool CUIRadioButton::HandleKeyboard(UINT uMsg, WPARAM wParam, LPARAM lParam)
		{
			if (!m_bEnable || !m_bVisible)
				return false;

			switch (uMsg)
			{
			case WM_KEYDOWN:
			{
				switch (wParam)
				{
				case VK_SPACE:
					m_bPressed = true;
					Invalidate();
					return true;
				}
			}
			case WM_KEYUP:
			{
				switch (wParam)
				{
				case VK_SPACE:
					if (m_bPressed == true)
					{
						m_bPressed = false;

						m_pUIPanel->ClearRadioButtonGroup(m_nButtonGroup);
						m_bChecked = !m_bChecked;
						Invalidate();
					}
					return true;
				}
			}
			}
			return false;
		}

		bool CUIRadioButton::HandleMouse(UINT uMsg, POINT pt, WPARAM wParam, LPARAM lParam)
		{
			if (!m_bEnable || !m_bVisible)
				return false;

			switch (uMsg)
			{
			case WM_LBUTTONDOWN:
			case WM_LBUTTONDBLCLK:
			{
				if (IsContainsPoint(pt))
				{
					// Pressed while inside the control
					SetCapture(m_pUIPanel->GetHWND());

					if (!m_bHasFocus)
					{
						m_pUIPanel->RequestFocus(this);
					}

					if (!m_bPressed)
					{
						m_pUIPanel->RequestMousePressed(this);
					}

					m_bPressed = true;
					Invalidate();
					return true;
				}
				break;
			}
			case WM_LBUTTONUP:
			{
				if (m_bPressed)
				{
					m_bPressed = false;
					ReleaseCapture();

					// Button click
					if (IsContainsPoint(pt))
					{
						m_pUIPanel->ClearRadioButtonGroup(m_nButtonGroup);
						m_bChecked = !m_bChecked;
					}

					m_pUIPanel->ClearMousePressed();

					Invalidate();
					return true;
				}
				break;
			}
			};

			return false;
		}

		void CUIRadioButton::OnHotkey()
		{
			if (m_pUIPanel->IsKeyboardInput())
			{
				m_pUIPanel->RequestFocus(this);
			}
			setCheckedInternal(true, true, true);
		}

		void CUIRadioButton::setCheckedInternal(bool bChecked, bool bClearGroup, bool bFromInput)
		{
			if (bChecked && bClearGroup)
			{
				m_pUIPanel->ClearRadioButtonGroup(m_nButtonGroup);
			}

			m_bChecked = bChecked;

			Invalidate();
		}
	}
}