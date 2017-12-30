#include "stdafx.h"
#include "UIComboBox.h"

#include "UIScrollBar.h"
#include "IUIPanel.h"

namespace EastEngine
{
	namespace UI
	{
		CUIComboBox::CUIComboBox(IUIPanel* pPanel, String::StringID strID, EmUI::Type emUIType)
			: CUIButton(pPanel, strID, emUIType)
			, m_nDropHeight(0)
			, m_nPrevDropHeight(0)
			, m_nMaxDropHeight(40)
			, m_nSBWidth(16)
			, m_bOpened(false)
			, m_bScrollBarInit(false)
			, m_nSelected(-1)
			, m_nFocused(-1)
		{
			std::string strSubID = strID.c_str();
			strSubID.append("_scrollbar");

			m_pScrollBar = pPanel->AddScrollBar(this, strSubID.c_str(), 0, 0, 0, 0);
			m_pScrollBar->SetVisible(false);
		}

		CUIComboBox::~CUIComboBox()
		{
			m_pScrollBar = nullptr;
			RemoveAllItems();
		}

		void CUIComboBox::Update(float fElapsedTime)
		{
			if (m_vecElements.empty())
				return;

			if (m_pScrollBar == nullptr)
				return;

			float fFocusDepth = m_bHasFocus ? -0.5f : 0.f;

			bool bNeedInvalidate = false;

			EmUI::State emState = EmUI::eNormal;

			if (!m_bOpened)
				emState = EmUI::eHidden;

			// Dropdown box Font
			IUIElement* pElement_DropdownFont = m_vecElements[2];

			// If we have not initialized the scrolbSBInitl bar page size,
			// do that now.
			if (!m_bScrollBarInit)
			{
				// Update the page size of the scroll bar
				UIFontNode* pFontNode = pElement_DropdownFont->GetFontNode();
				if (Math::IsZero(pFontNode->fHeight) == false)
				{
					m_pScrollBar->SetPageSize(m_rcDropdown.GetHeight() / (int)(pFontNode->fHeight));
				}
				else
				{
					m_pScrollBar->SetPageSize(m_rcDropdownText.GetHeight());
				}
				m_bScrollBarInit = true;
			}

			IUIElement* pElement_SelectionFont = m_vecElements[4];
			pElement_SelectionFont->SetColorCurrent(pElement_SelectionFont->GetColor(EmUI::eNormal));
			int curY = m_rcDropdownText.top;
			int nRemainingHeight = m_rcDropdownText.GetHeight();

			UIFontNode* pFontNode = pElement_SelectionFont->GetFontNode();
			int nFontHeight = (int)(pFontNode->fHeight);

			int nCount = m_vecItems.size() - m_pScrollBar->GetTrackPos();

			m_nDropHeight = std::max(nFontHeight, std::min(nCount * nFontHeight, m_nMaxDropHeight));
			if (m_nPrevDropHeight != m_nDropHeight)
			{
				m_nPrevDropHeight = m_nDropHeight;
				UpdateRects();
			}

			// Scroll bar
			m_pScrollBar->Update(fElapsedTime);
			m_pScrollBar->SetVisible(m_bOpened);

			// Blend current color
			bNeedInvalidate = pElement_DropdownFont->BlendColor(emState, fElapsedTime) == false || bNeedInvalidate;


			IUIElement* pElement_DropdownSpirte = m_vecElements[3];
			bNeedInvalidate = pElement_DropdownSpirte->BlendColor(emState, fElapsedTime) == false || bNeedInvalidate;

			m_pUIPanel->DrawSprite(pElement_DropdownSpirte, m_rcDropdown, UI_NEAR_DEPTH + fFocusDepth);

			// Selection outline
			IUIElement* pElement_SelectionSprite = m_vecElements[5];
			pElement_SelectionSprite->SetColorCurrent(pElement_DropdownSpirte->GetColor());

			size_t nSize = m_vecItems.size();
			for (size_t i = m_pScrollBar->GetTrackPos(); i < nSize; ++i)
			{
				UIComboBoxItem& item = m_vecItems[i];

				// Make sure there's room left in the dropdown
				nRemainingHeight -= nFontHeight;
				if (nRemainingHeight < 0)
				{
					item.bVisible = false;
					continue;
				}

				SetRect(&item.rcActive, m_rcDropdownText.left, curY, m_rcDropdownText.right, curY + nFontHeight);
				curY += nFontHeight + 2;

				item.bVisible = true;

				if (m_bOpened)
				{
					if ((int)(i) == m_nFocused)
					{
						Math::Rect rc;
						SetRect(&rc, m_rcDropdown.left, item.rcActive.top - 2, m_rcDropdown.right,
							item.rcActive.bottom + 2);

						m_pUIPanel->DrawSprite(pElement_SelectionSprite, rc, UI_NEAR_DEPTH + fFocusDepth);
						m_pUIPanel->DrawText(pElement_SelectionFont, item.strText, item.rcActive, UI_NEAR_DEPTH + fFocusDepth);
					}
					else
					{
						m_pUIPanel->DrawText(pElement_DropdownFont, item.strText, item.rcActive, UI_NEAR_DEPTH + fFocusDepth);
					}
				}
			}

			int nOffsetX = 0;
			int nOffsetY = 0;

			emState = EmUI::eNormal;

			if (m_bVisible == false)
				emState = EmUI::eHidden;
			else if (m_bEnable == false)
				emState = EmUI::eDisabled;
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
				emState = EmUI::eFocus;

			float fBlendRate = (emState == EmUI::ePressed) ? 0.0f : 0.8f;

			// Button
			IUIElement* pElement_Button = m_vecElements[6];

			// Blend current color
			bNeedInvalidate = pElement_Button->BlendColor(emState, fElapsedTime, fBlendRate) == false || bNeedInvalidate;

			Math::Rect rcWindow = m_rcButton;
			OffsetRect(&rcWindow, nOffsetX, nOffsetY);
			m_pUIPanel->DrawSprite(pElement_Button, rcWindow, UI_FAR_DEPTH + fFocusDepth);

			if (m_bOpened)
			{
				emState = EmUI::ePressed;
			}

			// Main text box
			IUIElement* pElement_MainSprite = m_vecElements[1];

			// Blend current color
			bNeedInvalidate = pElement_MainSprite->BlendColor(emState, fElapsedTime, fBlendRate) == false || bNeedInvalidate;

			m_pUIPanel->DrawSprite(pElement_MainSprite, m_rcText, UI_NEAR_DEPTH + fFocusDepth);

			IUIElement* pElement_MainFont = m_vecElements[0];
			bNeedInvalidate = pElement_MainFont->BlendColor(emState, fElapsedTime, fBlendRate) == false || bNeedInvalidate;

			if (m_nSelected >= 0 && m_nSelected < (int)(m_vecItems.size()))
			{
				UIComboBoxItem& item = m_vecItems[m_nSelected];

				Math::Rect rcText = m_rcText;
				OffsetRect(&rcText, 5, 5);

				m_pUIPanel->DrawText(pElement_MainFont, item.strText, rcText, UI_NEAR_DEPTH + fFocusDepth);
			}

			if (bNeedInvalidate)
			{
				Invalidate();
			}
		}

		bool CUIComboBox::HandleKeyboard(uint32_t uMsg, WPARAM wParam, LPARAM lParam)
		{
			const DWORD REPEAT_MASK = (0x40000000);

			if (!m_bEnable || !m_bVisible)
				return false;

			// Let the scroll bar have a chance to handle it first
			if (m_pScrollBar->HandleKeyboard(uMsg, wParam, lParam))
				return true;

			switch (uMsg)
			{
			case WM_KEYDOWN:
			{
				switch (wParam)
				{
				case VK_RETURN:
					if (m_bOpened)
					{
						if (m_nSelected != m_nFocused)
						{
							m_nSelected = m_nFocused;
						}
						m_bOpened = false;

						m_pUIPanel->ClearFocus();

						Invalidate();

						return true;
					}
					break;
				case VK_F4:
					// Filter out auto-repeats
					if (lParam & REPEAT_MASK)
						return true;

					m_bOpened = !m_bOpened;

					if (!m_bOpened)
					{
						m_pUIPanel->ClearFocus();
					}

					Invalidate();

					return true;
				case VK_LEFT:
				case VK_UP:
					if (m_nFocused > 0)
					{
						m_nFocused--;
						m_nSelected = m_nFocused;
					}

					Invalidate();

					return true;
				case VK_RIGHT:
				case VK_DOWN:
					if (m_nFocused + 1 < (int)(m_vecItems.size()))
					{
						m_nFocused++;
						m_nSelected = m_nFocused;
					}

					Invalidate();

					return true;
				}
				break;
			}
			}

			return false;
		}

		bool CUIComboBox::HandleMouse(uint32_t uMsg, POINT pt, WPARAM wParam, LPARAM lParam)
		{
			if (!m_bEnable || !m_bVisible)
				return false;

			// Let the scroll bar handle it first.
			if (m_pScrollBar->HandleMouse(uMsg, pt, wParam, lParam))
				return true;

			switch (uMsg)
			{
			case WM_MOUSEMOVE:
			{
				if (m_bOpened && PtInRect(&m_rcDropdown, pt))
				{
					// Determine which item has been selected
					size_t nSize = m_vecItems.size();
					for (size_t i = 0; i < nSize; ++i)
					{
						UIComboBoxItem& item = m_vecItems[i];

						if (item.bVisible && PtInRect(&item.rcActive, pt))
						{
							m_nFocused = i;

							Invalidate();

							break;
						}
					}
					return true;
				}
				break;
			}
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

					// Toggle dropdown
					if (m_bHasFocus)
					{
						m_bOpened = !m_bOpened;

						if (!m_bOpened)
						{
							m_pUIPanel->ClearFocus();
						}
					}

					Invalidate();

					return true;
				}

				// Perhaps this click is within the dropdown
				if (m_bOpened && PtInRect(&m_rcDropdown, pt))
				{
					// Determine which item has been selected
					size_t nSize = m_vecItems.size();
					for (size_t i = m_pScrollBar->GetTrackPos(); i < nSize; ++i)
					{
						UIComboBoxItem& item = m_vecItems[i];
						if (item.bVisible && PtInRect(&item.rcActive, pt))
						{
							m_nFocused = m_nSelected = i;
							m_bOpened = false;

							m_pUIPanel->ClearFocus();

							Invalidate();

							break;
						}
					}

					return true;
				}

				// Mouse click not on main control or in dropdown, fire an event if needed
				if (m_bOpened)
				{
					m_nFocused = m_nSelected;

					m_bOpened = false;
				}

				// Make sure the control is no longer in a pressed state
				m_bPressed = false;

				// Release focus if appropriate
				m_pUIPanel->ClearFocus();

				Invalidate();

				break;
			}
			case WM_LBUTTONUP:
			{
				if (m_bPressed && IsContainsPoint(pt))
				{
					// Button click
					m_bPressed = false;

					m_pUIPanel->ClearMousePressed();

					ReleaseCapture();

					Invalidate();

					return true;
				}
				break;
			}
			case WM_MOUSEWHEEL:
			{
				int zDelta = (short)HIWORD(wParam) / WHEEL_DELTA;
				if (m_bOpened)
				{
					uint32_t uLines;
					SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &uLines, 0);
					m_pScrollBar->Scroll(-zDelta * uLines);
				}
				else
				{
					if (zDelta > 0)
					{
						if (m_nFocused > 0)
						{
							m_nFocused--;
							m_nSelected = m_nFocused;
						}
					}
					else
					{
						if (m_nFocused + 1 < (int)(m_vecItems.size()))
						{
							m_nFocused++;
							m_nSelected = m_nFocused;
						}
					}
				}

				Invalidate();

				return true;
			}
			};

			return false;
		}

		void CUIComboBox::OnHotkey()
		{
			if (m_bOpened)
				return;

			if (m_nSelected == -1)
				return;

			if (m_pUIPanel->IsKeyboardInput())
				m_pUIPanel->RequestFocus(this);

			m_nSelected++;

			if (m_nSelected >= (int)(m_vecElements.size()))
				m_nSelected = 0;

			m_nFocused = m_nSelected;
		}

		void CUIComboBox::OnFocusOut()
		{
			CUIButton::OnFocusOut();

			m_bOpened = false;
		}

		bool CUIComboBox::AddItem(const char* strText, void* pData)
		{
			// Validate parameters
			if (strText == nullptr)
				return false;

			UIComboBoxItem item;
			item.strText = strText;
			item.pData = pData;

			m_vecItems.push_back(item);

			// Update the scroll bar with new range
			m_pScrollBar->SetTrackRange(0, m_vecItems.size());

			// If this is the only item in the list, it's selected
			if (m_vecItems.size() == 1)
			{
				m_nSelected = 0;
				m_nFocused = 0;
			}

			return true;
		}

		void CUIComboBox::RemoveAllItems()
		{
			m_vecItems.clear();

			if (m_pScrollBar != nullptr)
			{
				m_pScrollBar->SetTrackRange(0, 1);
			}
			m_nFocused = m_nSelected = -1;
		}

		void CUIComboBox::RemoveItem(size_t nIdx)
		{
			if (nIdx >= m_vecItems.size())
				return;

			m_vecItems.erase(m_vecItems.begin() + nIdx);

			m_pScrollBar->SetTrackRange(0, m_vecItems.size());

			if (m_nSelected >= (int)(m_vecItems.size()))
			{
				m_nSelected = m_vecItems.size() - 1;
			}
		}

		int CUIComboBox::FindItem(const char* strText, size_t nStart)
		{
			if (nStart >= m_vecItems.size())
				return -1;

			if (strText == nullptr)
				return -1;

			size_t nSize = m_vecItems.size();
			for (; nStart < nSize; ++nStart)
			{
				if (m_vecItems[nStart].strText == strText)
					return nStart;
			}

			return -1;
		}

		void* CUIComboBox::GetItemData(const char* strText)
		{
			int nIdx = FindItem(strText);
			if (nIdx == -1)
				return nullptr;

			return GetItemData(nIdx);
		}

		void* CUIComboBox::GetItemData(int nIdx)
		{
			if (nIdx < 0 || nIdx >= (int)(m_vecItems.size()))
				return nullptr;

			return m_vecItems[nIdx].pData;
		}

		void* CUIComboBox::GetSelectedData()
		{
			return GetItemData(m_nSelected);
		}

		UIComboBoxItem* CUIComboBox::GetSelectedItem()
		{
			if (m_nSelected < 0 || m_nSelected >= (int)(m_vecItems.size()))
				return nullptr;

			return &m_vecItems[m_nSelected];
		}

		void CUIComboBox::SetSelectedByIndex(size_t nIdx)
		{
			if (nIdx >= m_vecItems.size())
				return;

			m_nFocused = m_nSelected = nIdx;
		}

		void CUIComboBox::SetSelectedByText(const char* strText)
		{
			int nIdx = FindItem(strText);
			if (nIdx == -1)
				return;

			m_nFocused = m_nSelected = nIdx;
		}

		void CUIComboBox::SetSelectedByData(void* pData)
		{
			size_t nSize = m_vecItems.size();
			for (size_t i = 0; i < nSize; ++i)
			{
				UIComboBoxItem& item = m_vecItems[i];

				if (item.pData == pData)
				{
					m_nFocused = m_nSelected = i;
					break;
				}
			}
		}

		void CUIComboBox::UpdateRects()
		{
			CUIButton::UpdateRects();

			m_rcButton = m_rcBoundingBox;
			m_rcButton.left = m_rcButton.right - m_rcButton.GetHeight();

			m_rcText = m_rcBoundingBox;
			m_rcText.right = m_rcButton.left;

			m_rcDropdown = m_rcText;
			OffsetRect(&m_rcDropdown, 0, m_rcText.GetHeight());
			m_rcDropdown.bottom += m_nDropHeight;
			//m_rcDropdown.right += m_nSBWidth;

			m_rcDropdownText = m_rcDropdown;
			m_rcDropdownText.left += (int)(0.1f * m_rcDropdown.GetWidth());
			m_rcDropdownText.right -= (int)(0.1f * m_rcDropdown.GetWidth());
			m_rcDropdownText.top += (int)(0.1f * m_rcDropdown.GetHeight());
			m_rcDropdownText.bottom -= (int)(0.1f * m_rcDropdown.GetHeight());

			// Update the scrollbar's rects
			m_pScrollBar->SetPosition(m_rcDropdown.right, m_rcDropdown.top + 2);
			m_pScrollBar->SetSize(m_nSBWidth, m_rcDropdown.GetHeight() - 2);

			if (m_vecElements.empty())
				return;

			UIFontNode* pFontNode = m_vecElements[3]->GetFontNode();
			if (pFontNode && Math::IsZero(pFontNode->fHeight) == false)
			{
				m_pScrollBar->SetPageSize(m_rcDropdownText.GetHeight() / (int)(pFontNode->fHeight));

				m_pScrollBar->ShowItem(m_nSelected);
			}

			m_pScrollBar->UpdateRects();
		}
	}
}