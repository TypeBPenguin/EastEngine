#include "stdafx.h"
#include "UICheckBox.h"

#include "IUIPanel.h"

namespace EastEngine
{
	namespace UI
	{
		CUICheckBox::CUICheckBox(IUIPanel* pPanel, String::StringID strID, EmUI::Type emUIType)
			: CUIButton(pPanel, strID, emUIType)
			, m_bChecked(false)
		{
			Memory::Clear(&m_rcButton, sizeof(m_rcButton));
			Memory::Clear(&m_rcText, sizeof(m_rcText));
		}

		CUICheckBox::~CUICheckBox()
		{
		}

		void CUICheckBox::Update(float fElapsedTime)
		{
			if (m_bVisible == false)
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
				nOffsetY = 1;
			}
			else if (m_bMouseOver)
			{
				emState = EmUI::eMouseOver;

				nOffsetX = -1;
				nOffsetY = -1;
			}
			else if (m_bHasFocus)
			{
				emState = EmUI::eFocus;
			}

			Math::Rect rcButton = m_rcButton;
			OffsetRect(&rcButton, nOffsetX, nOffsetY);
			Math::Rect rcText = m_rcText;
			OffsetRect(&rcText, nOffsetX, nOffsetY);

			float fBlendRate = (emState == EmUI::ePressed) ? 0.0f : 0.8f;

			IUIElement* pElement = m_vecElements[0];
			bNeedInvalidate = pElement->BlendColor(emState, fElapsedTime, fBlendRate) == false || bNeedInvalidate;
			m_pUIPanel->DrawText(pElement, m_strText, rcText, UI_NEAR_DEPTH + fFocusDepth);

			pElement = m_vecElements[1];
			bNeedInvalidate = pElement->BlendColor(emState, fElapsedTime, fBlendRate) == false || bNeedInvalidate;
			m_pUIPanel->DrawSprite(pElement, rcButton, UI_NEAR_DEPTH + fFocusDepth);

			if (!m_bChecked)
				emState = EmUI::eHidden;

			pElement = m_vecElements[2];
			bNeedInvalidate = pElement->BlendColor(emState, fElapsedTime, fBlendRate) == false || bNeedInvalidate;
			m_pUIPanel->DrawSprite(pElement, rcButton, UI_FAR_DEPTH + fFocusDepth);

			if (bNeedInvalidate)
			{
				Invalidate();
			}
		}

		bool CUICheckBox::HandleKeyboard(uint32_t nMsg, WPARAM wParam, LPARAM lParam)
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

			case WM_KEYUP:
			{
				switch (wParam)
				{
				case VK_SPACE:
					if (m_bPressed == true)
					{
						m_bPressed = false;
						setCheckedInternal(!m_bChecked, true);
						Invalidate();
					}
					return true;
				}
			}
			}
			return false;
		}

		bool CUICheckBox::HandleMouse(uint32_t nMsg, POINT pt, WPARAM wParam, LPARAM lParam)
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
						setCheckedInternal(!m_bChecked, true);
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

		void CUICheckBox::UpdateRects()
		{
			CUIButton::UpdateRects();

			m_rcButton = m_rcBoundingBox;
			m_rcButton.right = m_rcButton.left + m_rcButton.GetHeight();

			m_rcText = m_rcBoundingBox;
			m_rcText.left += (int)(1.25f * m_rcButton.GetWidth());
		}

		void CUICheckBox::setCheckedInternal(bool bChecked, bool bFromInput)
		{
			m_bChecked = bChecked;

			Invalidate();

			//m_pUIManager->SendEvent(UI_EVENT_CHECKBOX_CHANGED, bFromInput, this);
		}
	}
}