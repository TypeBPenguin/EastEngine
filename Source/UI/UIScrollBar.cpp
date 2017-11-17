#include "stdafx.h"
#include "UIScrollBar.h"

#include "IUIPanel.h"

namespace EastEngine
{
	namespace UI
	{
		CUIScrollBar::CUIScrollBar(IUIPanel* pPanel, String::StringID strID, EmUI::Type emUIType)
			: CUIObject(pPanel, strID, emUIType)
			, m_bShowThumb(true)
			, m_bDrag(false)
			, m_nPosition(0)
			, m_nPageSize(1)
			, m_nStart(0)
			, m_nEnd(1)
			, m_emArrow(EmArrowState::eClear)
			, m_fArrowTime(0.f)
			, m_fTime(0.f)
		{
			ZeroMemory(&m_rcUpButton, sizeof(m_rcUpButton));
			ZeroMemory(&m_rcDownButton, sizeof(m_rcDownButton));
			ZeroMemory(&m_rcTrack, sizeof(m_rcTrack));
			ZeroMemory(&m_rcThumb, sizeof(m_rcThumb));
			ZeroMemory(&m_LastMouse, sizeof(m_LastMouse));
		}

		CUIScrollBar::~CUIScrollBar()
		{
		}

		void CUIScrollBar::Update(float fElapsedTime)
		{
			if (m_vecElements.empty())
				return;

			float fFocusDepth = m_bHasFocus ? -0.5f : 0.f;

			bool bNeedInvalidate = false;

			// Check if the arrow button has been held for a while.
			// If so, update the thumb position to simulate repeated
			// scroll.
			if (m_emArrow != EmArrowState::eClear)
			{
				if (PtInRect(&m_rcUpButton, m_LastMouse))
				{
					switch (m_emArrow)
					{
					case EmArrowState::eClickedUp:
						if (ScrollBar_ArrowClick_Delay < m_fTime - m_fArrowTime)
						{
							Scroll(-1);
							m_emArrow = EmArrowState::eHeldUp;
							m_fArrowTime = m_fTime;
						}
						break;
					case EmArrowState::eHeldUp:
						if (ScrollBar_ArrowClick_Repeat < m_fTime - m_fArrowTime)
						{
							Scroll(-1);
							m_fArrowTime = m_fTime;
						}
						break;
					}
				}
				else if (PtInRect(&m_rcDownButton, m_LastMouse))
				{
					switch (m_emArrow)
					{
					case EmArrowState::eClickedDown:
						if (ScrollBar_ArrowClick_Delay < m_fTime - m_fArrowTime)
						{
							Scroll(1);
							m_emArrow = EmArrowState::eHeldDown;
							m_fArrowTime = m_fTime;
						}
						break;
					case EmArrowState::eHeldDown:
						if (ScrollBar_ArrowClick_Repeat < m_fTime - m_fArrowTime)
						{
							Scroll(1);
							m_fArrowTime = m_fTime;
						}
						break;
					}
				}
			}

			EmUI::State emState = EmUI::eNormal;

			if (m_bVisible == false)
				emState = EmUI::eHidden;
			else if (m_bEnable == false || m_bShowThumb == false)
				emState = EmUI::eDisabled;
			else if (m_bMouseOver)
				emState = EmUI::eMouseOver;
			else if (m_bHasFocus)
				emState = EmUI::eFocus;


			float fBlendRate = (emState == EmUI::ePressed) ? 0.0f : 0.8f;

			// Background track layer
			IUIElement* pElement = m_vecElements[0];

			// Blend current color
			bNeedInvalidate = pElement->BlendColor(emState, fElapsedTime, fBlendRate) == false || bNeedInvalidate;
			m_pUIPanel->DrawSprite(pElement, m_rcTrack, UI_FAR_DEPTH + fFocusDepth);

			// Up Arrow
			pElement = m_vecElements[1];

			// Blend current color
			bNeedInvalidate = pElement->BlendColor(emState, fElapsedTime, fBlendRate) == false || bNeedInvalidate;
			m_pUIPanel->DrawSprite(pElement, m_rcUpButton, UI_NEAR_DEPTH + fFocusDepth);

			// Down Arrow
			pElement = m_vecElements[2];

			// Blend current color
			bNeedInvalidate = pElement->BlendColor(emState, fElapsedTime, fBlendRate) == false || bNeedInvalidate;
			m_pUIPanel->DrawSprite(pElement, m_rcDownButton, UI_NEAR_DEPTH + fFocusDepth);

			// Thumb button
			pElement = m_vecElements[3];

			// Blend current color
			bNeedInvalidate = pElement->BlendColor(emState, fElapsedTime, fBlendRate) == false || bNeedInvalidate;
			m_pUIPanel->DrawSprite(pElement, m_rcThumb, UI_NEAR_DEPTH + fFocusDepth);

			if (bNeedInvalidate)
			{
				Invalidate();
			}
		}

		bool CUIScrollBar::HandleMsg(HWND hWnd, uint32_t nMsg, WPARAM wParam, LPARAM lParam)
		{
			if (WM_CAPTURECHANGED == nMsg)
			{
				// The application just lost mouse capture. We may not have gotten
				// the WM_MOUSEUP message, so reset m_bDrag here.
				if (reinterpret_cast<HWND>(lParam) != m_pUIPanel->GetHWND())
					m_bDrag = false;
			}

			return false;
		}

		bool CUIScrollBar::HandleKeyboard(uint32_t uMsg, WPARAM wParam, LPARAM lParam)
		{
			return false;
		}

		bool CUIScrollBar::HandleMouse(uint32_t uMsg, POINT pt, WPARAM wParam, LPARAM lParam)
		{
			static int ThumbOffsetY;

			m_LastMouse = pt;
			switch (uMsg)
			{
			case WM_LBUTTONDOWN:
			case WM_LBUTTONDBLCLK:
			{
				if (m_bShowThumb == false)
					return false;

				// Check for click on up button
				if (PtInRect(&m_rcUpButton, pt))
				{
					SetCapture(m_pUIPanel->GetHWND());
					if (m_nPosition > m_nStart)
					{
						--m_nPosition;
					}

					updateThumbRect();
					m_emArrow = EmArrowState::eClickedUp;
					m_fArrowTime = m_fTime;

					Invalidate();

					return true;
				}

				// Check for click on down button

				if (PtInRect(&m_rcDownButton, pt))
				{
					SetCapture(m_pUIPanel->GetHWND());
					if (m_nPosition + m_nPageSize <= m_nEnd)
						++m_nPosition;
					updateThumbRect();
					m_emArrow = EmArrowState::eClickedDown;
					m_fArrowTime = m_fTime;

					Invalidate();

					return true;
				}

				// Check for click on thumb

				if (PtInRect(&m_rcThumb, pt))
				{
					SetCapture(m_pUIPanel->GetHWND());
					m_bDrag = true;
					ThumbOffsetY = pt.y - m_rcThumb.top;

					Invalidate();

					return true;
				}

				// Check for click on track

				if (m_rcThumb.left <= pt.x &&
					m_rcThumb.right > pt.x)
				{
					SetCapture(m_pUIPanel->GetHWND());
					if (m_rcThumb.top > pt.y &&
						m_rcTrack.top <= pt.y)
					{
						Scroll(-(m_nPageSize - 1));
						return true;
					}
					else if (m_rcThumb.bottom <= pt.y &&
						m_rcTrack.bottom > pt.y)
					{
						Scroll(m_nPageSize - 1);
						return true;
					}
				}

				break;
			}

			case WM_LBUTTONUP:
			{
				m_bDrag = false;
				ReleaseCapture();
				updateThumbRect();
				m_emArrow = EmArrowState::eClear;

				Invalidate();

				break;
			}

			case WM_MOUSEMOVE:
			{
				if (m_bDrag)
				{
					m_rcThumb.bottom += pt.y - ThumbOffsetY - m_rcThumb.top;
					m_rcThumb.top = pt.y - ThumbOffsetY;
					if (m_rcThumb.top < m_rcTrack.top)
						OffsetRect(&m_rcThumb, 0, m_rcTrack.top - m_rcThumb.top);
					else if (m_rcThumb.bottom > m_rcTrack.bottom)
						OffsetRect(&m_rcThumb, 0, m_rcTrack.bottom - m_rcThumb.bottom);

					// Compute first item index based on thumb position

					int nMaxFirstItem = m_nEnd - m_nStart - m_nPageSize + 1;  // Largest possible index for first item
					int nMaxThumb = m_rcTrack.GetHeight() - m_rcThumb.GetHeight();  // Largest possible thumb position from the top

					m_nPosition = m_nStart +
						(m_rcThumb.top - m_rcTrack.top +
							nMaxThumb / (nMaxFirstItem * 2)) * // Shift by half a row to avoid last row covered by only one pixel
						nMaxFirstItem / nMaxThumb;

					Invalidate();

					return true;
				}

				break;
			}
			}

			return false;
		}

		void CUIScrollBar::UpdateRects()
		{
			CUIObject::UpdateRects();

			// Make the buttons square
			m_rcUpButton.Set(m_rcBoundingBox.left, m_rcBoundingBox.top,
				m_rcBoundingBox.right, m_rcBoundingBox.top + m_rcBoundingBox.GetWidth());

			m_rcDownButton.Set(m_rcBoundingBox.left, m_rcBoundingBox.bottom - m_rcBoundingBox.GetWidth(),
				m_rcBoundingBox.right, m_rcBoundingBox.bottom);

			m_rcTrack.Set(m_rcUpButton.left, m_rcUpButton.bottom,
				m_rcDownButton.right, m_rcDownButton.top);
			m_rcThumb.left = m_rcUpButton.left;
			m_rcThumb.right = m_rcUpButton.right;

			updateThumbRect();
		}

		void CUIScrollBar::SetTrackRange(int nStart, int nEnd)
		{
			m_nStart = nStart; m_nEnd = nEnd;
			cap();
			updateThumbRect();

			Invalidate();
		}

		void CUIScrollBar::Scroll(int nDelta)
		{
			// Perform scroll
			m_nPosition += nDelta;

			// Cap position
			cap();

			// Update thumb position
			updateThumbRect();

			Invalidate();
		}

		void CUIScrollBar::ShowItem(int nIndex)
		{
			// Cap the index

			if (nIndex < 0)
				nIndex = 0;

			if (nIndex >= m_nEnd)
				nIndex = m_nEnd - 1;

			// Adjust position

			if (m_nPosition > nIndex)
				m_nPosition = nIndex;
			else if (m_nPosition + m_nPageSize <= nIndex)
				m_nPosition = nIndex - m_nPageSize + 1;

			updateThumbRect();

			Invalidate();
		}

		void CUIScrollBar::updateThumbRect()
		{
			if (m_nEnd - m_nStart > m_nPageSize)
			{
				int nThumbHeight = std::max(static_cast<int>(m_rcTrack.GetHeight()) * m_nPageSize / (m_nEnd - m_nStart), ScrollBar_MinThumbSize);
				int nMaxPosition = m_nEnd - m_nStart - m_nPageSize;
				m_rcThumb.top = m_rcTrack.top + (m_nPosition - m_nStart) * (m_rcTrack.GetHeight() - nThumbHeight) / nMaxPosition;
				m_rcThumb.bottom = m_rcThumb.top + nThumbHeight;
				m_bShowThumb = true;

			}
			else
			{
				// No content to scroll
				m_rcThumb.bottom = m_rcThumb.top;
				m_bShowThumb = false;
			}
		}

		void CUIScrollBar::cap()
		{
			if (m_nPosition < m_nStart ||
				m_nEnd - m_nStart <= m_nPageSize)
			{
				m_nPosition = m_nStart;
			}
			else if (m_nPosition + m_nPageSize > m_nEnd)
				m_nPosition = m_nEnd - m_nPageSize + 1;
		}
	}
}