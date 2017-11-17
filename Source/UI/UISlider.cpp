#include "stdafx.h"
#include "UISlider.h"

#include "IUIPanel.h"

namespace EastEngine
{
	namespace UI
	{
		CUISlider::CUISlider(IUIPanel* pPanel, String::StringID strID, EmUI::Type emUIType)
			: CUIObject(pPanel, strID, emUIType)
			, m_nMin(0)
			, m_nMax(100)
			, m_nValue(50)
		{
		}

		CUISlider::~CUISlider()
		{
		}

		void CUISlider::Update(float fElapsedTime)
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

			float fBlendRate = emState == EmUI::ePressed ? 0.0f : 0.8f;

			IUIElement* pElement = m_vecElements[0];

			// Blend current color
			bNeedInvalidate = pElement->BlendColor(emState, fElapsedTime, fBlendRate) == false || bNeedInvalidate;
			m_pUIPanel->DrawSprite(pElement, m_rcBoundingBox, UI_FAR_DEPTH + fFocusDepth);

			pElement = m_vecElements[1];

			// Blend current color
			bNeedInvalidate = pElement->BlendColor(emState, fElapsedTime, fBlendRate) == false || bNeedInvalidate;
			m_pUIPanel->DrawSprite(pElement, m_rcButton, UI_NEAR_DEPTH + fFocusDepth);

			if (bNeedInvalidate)
			{
				Invalidate();
			}
		}

		bool CUISlider::HandleKeyboard(uint32_t nMsg, WPARAM wParam, LPARAM lParam)
		{
			if (!m_bEnable || !m_bVisible)
				return false;

			switch (nMsg)
			{
			case WM_KEYDOWN:
			{
				switch (wParam)
				{
				case VK_HOME:
					setValueInternal(m_nMin, true);
					return true;

				case VK_END:
					setValueInternal(m_nMax, true);
					return true;

				case VK_LEFT:
				case VK_DOWN:
					setValueInternal(m_nValue - 1, true);
					return true;

				case VK_RIGHT:
				case VK_UP:
					setValueInternal(m_nValue + 1, true);
					return true;

				case VK_NEXT:
					setValueInternal(m_nValue - (10 > (m_nMax - m_nMin) / 10 ? 10 : (m_nMax - m_nMin) / 10),
						true);
					return true;

				case VK_PRIOR:
					setValueInternal(m_nValue + (10 > (m_nMax - m_nMin) / 10 ? 10 : (m_nMax - m_nMin) / 10),
						true);
					return true;
				}
				break;
			}
			}


			return false;
		}

		bool CUISlider::HandleMouse(uint32_t nMsg, POINT pt, WPARAM wParam, LPARAM lParam)
		{
			if (!m_bEnable || !m_bVisible)
				return false;

			switch (nMsg)
			{
			case WM_LBUTTONDOWN:
			case WM_LBUTTONDBLCLK:
			{
				if (PtInRect(&m_rcButton, pt))
				{
					// Pressed while inside the control
					SetCapture(m_pUIPanel->GetHWND());

					m_nDragX = pt.x;
					//m_nDragY = pt.y;
					m_nDragOffset = m_nButtonX - m_nDragX;

					//m_nDragValue = m_nValue;

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

				if (PtInRect(&m_rcBoundingBox, pt))
				{
					m_nDragX = pt.x;
					m_nDragOffset = 0;
					m_bPressed = true;

					if (!m_bHasFocus)
					{
						m_pUIPanel->RequestFocus(this);
					}

					if (pt.x > m_nButtonX + m_nPosX)
					{
						setValueInternal(m_nValue + 1, true);
						return true;
					}

					if (pt.x < m_nButtonX + m_nPosX)
					{
						setValueInternal(m_nValue - 1, true);
						return true;
					}

					Invalidate();
				}

				break;
			}

			case WM_LBUTTONUP:
			{
				if (m_bPressed)
				{
					m_bPressed = false;

					m_pUIPanel->ClearMousePressed();
					ReleaseCapture();
					Invalidate();

					return true;
				}

				break;
			}

			case WM_MOUSEMOVE:
			{
				if (m_bPressed)
				{
					setValueInternal(valueFromPos(m_nPosX + pt.x + m_nDragOffset), true);
					return true;
				}

				break;
			}

			case WM_MOUSEWHEEL:
			{
				int nScrollAmount = (int)((short)(HIWORD(wParam))) / WHEEL_DELTA;
				setValueInternal(m_nValue - nScrollAmount, true);
				return true;
			}
			};

			return false;
		}

		void CUISlider::SetRange(int nMin, int nMax)
		{
			m_nMin = nMin;
			m_nMax = nMax;

			setValueInternal(m_nValue, false);

			Invalidate();
		}

		void CUISlider::UpdateRects()
		{
			CUIObject::UpdateRects();

			m_rcButton = m_rcBoundingBox;
			m_rcButton.right = m_rcButton.left + m_rcButton.GetHeight();
			OffsetRect(&m_rcButton, -m_rcButton.GetWidth() / 2, 0);

			m_nButtonX = (int)((m_nValue - m_nMin) * (float)(m_rcBoundingBox.GetWidth()) / (m_nMax - m_nMin));
			OffsetRect(&m_rcButton, m_nButtonX, 0);
		}

		void CUISlider::setValueInternal(int nValue, bool bFromInput)
		{
			// Clamp to range
			nValue = std::max(m_nMin, nValue);
			nValue = std::min(m_nMax, nValue);

			if (nValue == m_nValue)
				return;

			m_nValue = nValue;
			UpdateRects();

			Invalidate();
		}

		int CUISlider::valueFromPos(int x)
		{
			float fValuePerPixel = (float)(m_nMax - m_nMin) / m_rcBoundingBox.GetWidth();
			return (int)(0.5f + m_nMin + fValuePerPixel * (x - m_rcBoundingBox.left));
		}
	}
}