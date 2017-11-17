#include "stdafx.h"
#include "UIButton.h"

#include "IUIPanel.h"

namespace EastEngine
{
	namespace UI
	{
		CUIButton::CUIButton(IUIPanel* pPanel, String::StringID strID, EmUI::Type emUIType)
			: CUIStatic(pPanel, strID, emUIType)
		{
		}

		CUIButton::~CUIButton()
		{
		}

		void CUIButton::Update(float fElapsedTime)
		{
			if (m_vecElements.empty())
				return;

			float fFocusDepth = m_bHasFocus ? -0.5f : 0.f;

			int nOffsetX = 0;
			int nOffsetY = 0;

			bool bNeedInvalidate = false;

			EmUI::State emState = EmUI::eNormal;

			if (m_bVisible == false)
			{
				emState = EmUI::eHidden;
			}
			else if (m_bEnable == false)
			{
				emState = EmUI::eDisabled;
			}
			else if (m_bPressed)
			{
				emState = EmUI::ePressed;

				nOffsetX = 1;
				nOffsetY = 2;
			}
			else if (m_bMouseOver)
			{
				emState = EmUI::eMouseOver;

				nOffsetX = -1;
				nOffsetY = -2;
			}
			else if (m_bHasFocus)
			{
				emState = EmUI::eFocus;
			}

			Math::Rect rcWindow = m_rcBoundingBox;
			OffsetRect(&rcWindow, nOffsetX, nOffsetY);
			Math::Rect rcText = rcWindow;
			OffsetRect(&rcText, 5, 5);

			float fBlendRate = (emState == EmUI::ePressed) ? 0.0f : 0.8f;

			IUIElement* pElement = m_vecElements[2];
			bNeedInvalidate = pElement->BlendColor(emState, fElapsedTime, fBlendRate) == false || bNeedInvalidate;
			m_pUIPanel->DrawSprite(pElement, rcWindow, UI_FAR_DEPTH + fFocusDepth);

			// Main Button
			pElement = m_vecElements[0];
			bNeedInvalidate = pElement->BlendColor(emState, fElapsedTime, fBlendRate) == false || bNeedInvalidate;
			m_pUIPanel->DrawText(pElement, m_strText, rcText, UI_NEAR_DEPTH + fFocusDepth);

			pElement = m_vecElements[1];
			bNeedInvalidate = pElement->BlendColor(emState, fElapsedTime, fBlendRate) == false || bNeedInvalidate;
			m_pUIPanel->DrawSprite(pElement, rcWindow, UI_NEAR_DEPTH + fFocusDepth);

			if (bNeedInvalidate)
			{
				Invalidate();
			}
		}

		bool CUIButton::HandleKeyboard(uint32_t nMsg, WPARAM wParam, LPARAM lParam)
		{
			if (!m_bEnable || !m_bVisible)
				return false;

			switch (nMsg)
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
			break;
			case WM_KEYUP:
			{
				switch (wParam)
				{
				case VK_SPACE:
					if (m_bPressed == true)
					{
						m_bPressed = false;
						Invalidate();
					}
					return true;
				}
			}
			break;
			}
			return false;
		}

		bool CUIButton::HandleMouse(uint32_t nMsg, POINT pt, WPARAM wParam, LPARAM lParam)
		{
			if (!m_bEnable || !m_bVisible)
				return false;

			switch (nMsg)
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

					// Button click
					if (!m_bPressed)
					{
						m_pUIPanel->RequestMousePressed(this);
					}

					OnMousePressedIn();

					Invalidate();

					return true;
				}
				break;
			}
			case WM_LBUTTONUP:
			{
				if (m_bPressed)
				{
					ReleaseCapture();

					m_pUIPanel->ClearFocus();

					// Button click
					OnMousePressedOut();

					m_pUIPanel->ClearMousePressed();

					Invalidate();

					return true;
				}
				break;
			}
			};

			return false;
		}
	}
}