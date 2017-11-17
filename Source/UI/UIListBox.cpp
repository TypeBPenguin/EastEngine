#include "stdafx.h"
#include "UIListBox.h"

#include "UIScrollBar.h"
#include "IUIPanel.h"

namespace EastEngine
{
	namespace UI
	{
		CUIListBox::CUIListBox(IUIPanel* pPanel, String::StringID strID, EmUI::Type emUIType)
			: CUIObject(pPanel, strID, emUIType)
			, m_nSBWidth(16)
			, m_nBorder(6)
			, m_nMargin(5)
			, m_nTextHeight(0)
			, m_nSelected(-1)
			, m_nSelStart(0)
			, m_bDrag(false)
			, m_bScrollBarInit(false)
		{
			ZeroMemory(&m_rcText, sizeof(m_rcText));
			ZeroMemory(&m_rcSelection, sizeof(m_rcSelection));

			std::string strSubID = strID.c_str();
			strSubID.append("_scrollbar");

			m_pScrollBar = pPanel->AddScrollBar(this, strSubID.c_str(), 0, 0, 0, 0);
			m_pScrollBar->SetVisible(false);
		}

		CUIListBox::~CUIListBox()
		{
			m_pScrollBar = nullptr;
			RemoveAllItems();
		}

		void CUIListBox::Update(float fElapsedTime)
		{
			if (m_bVisible == false)
				return;

			bool bNeedInvalidate = false;

			IUIElement* pElement_MainFont = m_vecElements[0];
			bNeedInvalidate = pElement_MainFont->BlendColor(EmUI::eNormal, fElapsedTime) == false || bNeedInvalidate;

			IUIElement* pElement = m_vecElements[1];
			bNeedInvalidate = pElement->BlendColor(EmUI::eNormal, fElapsedTime) == false || bNeedInvalidate;

			m_pUIPanel->DrawSprite(pElement, m_rcBoundingBox, UI_FAR_DEPTH);

			IUIElement* pSelElement_Font = m_vecElements[2];
			bNeedInvalidate = pSelElement_Font->BlendColor(EmUI::eNormal, fElapsedTime) == false || bNeedInvalidate;

			IUIElement* pSelElement = m_vecElements[3];
			bNeedInvalidate = pSelElement->BlendColor(EmUI::eNormal, fElapsedTime) == false || bNeedInvalidate;

			// Render the text
			if (m_vecItems.empty() == false)
			{
				UIFontNode* pFontNode = pElement_MainFont->GetFontNode();

				// Find out the height of a single line of text
				Math::Rect rc = m_rcText;
				Math::Rect rcSel = m_rcSelection;
				rc.bottom = rc.top + (int)(pFontNode->fHeight);

				// Update the line height formation
				m_nTextHeight = rc.bottom - rc.top;

				if (m_bScrollBarInit == false)
				{
					// Update the page size of the scroll bar
					if (m_nTextHeight)
					{
						m_pScrollBar->SetPageSize(m_rcText.GetHeight() / m_nTextHeight);
					}
					else
					{
						m_pScrollBar->SetPageSize(m_rcText.GetHeight());
					}
					m_bScrollBarInit = true;
				}

				rc.right = m_rcText.right;
				int nSize = (int)(m_vecItems.size());
				for (int i = m_pScrollBar->GetTrackPos(); i < nSize; ++i)
				{
					if (rc.bottom > m_rcText.bottom)
						break;

					UIListBoxItem& item = m_vecItems[i];

					// Determine if we need to render this item with the
					// selected element.
					bool bSelectedStyle = false;

					if (m_emStyle != EmListBox::eMultiSelection && i == m_nSelected)
					{
						bSelectedStyle = true;
					}
					else if (m_emStyle == EmListBox::eMultiSelection)
					{
						if (m_bDrag &&
							((i >= m_nSelected && i < m_nSelStart) ||
							(i <= m_nSelected && i > m_nSelStart)))
						{
							bSelectedStyle = m_vecItems[m_nSelStart].bSelected;
						}
						else if (item.bSelected)
						{
							bSelectedStyle = true;
						}
					}

					if (bSelectedStyle)
					{
						rcSel.top = rc.top;
						rcSel.bottom = rc.bottom;

						m_pUIPanel->DrawSprite(pSelElement, rcSel, UI_NEAR_DEPTH);

						m_pUIPanel->DrawText(pSelElement_Font, item.strText, rc, UI_NEAR_DEPTH);
					}
					else
					{
						m_pUIPanel->DrawText(pElement_MainFont, item.strText, rc, UI_NEAR_DEPTH);
					}

					OffsetRect(&rc, 0, m_nTextHeight);
				}
			}

			// Render the scroll bar
			m_pScrollBar->Update(fElapsedTime);

			if (bNeedInvalidate)
			{
				Invalidate();
			}
		}

		bool CUIListBox::HandleMsg(HWND hWnd, uint32_t uMsg, WPARAM wParam, LPARAM lParam)
		{
			if (WM_CAPTURECHANGED == uMsg)
			{
				// The application just lost mouse capture. We may not have gotten
				// the WM_MOUSEUP message, so reset m_bDrag here.
				if (reinterpret_cast<HWND>(lParam) != m_pUIPanel->GetHWND())
					m_bDrag = false;
			}

			return false;
		}

		bool CUIListBox::HandleKeyboard(uint32_t uMsg, WPARAM wParam, LPARAM lParam)
		{
			if (!m_bEnable || !m_bVisible)
				return false;

			// Let the scroll bar have a chance to handle it first
			if (m_pScrollBar->HandleKeyboard(uMsg, wParam, lParam))
				return true;

			switch (uMsg)
			{
			case WM_KEYDOWN:
				switch (wParam)
				{
				case VK_UP:
				case VK_DOWN:
				case VK_NEXT:
				case VK_PRIOR:
				case VK_HOME:
				case VK_END:
				{
					// If no item exists, do nothing.
					if (m_vecItems.empty())
						return true;

					int nOldSelected = m_nSelected;

					// Adjust m_nSelected
					switch (wParam)
					{
					case VK_UP:
						--m_nSelected; break;
					case VK_DOWN:
						++m_nSelected; break;
					case VK_NEXT:
						m_nSelected += m_pScrollBar->GetPageSize() - 1; break;
					case VK_PRIOR:
						m_nSelected -= m_pScrollBar->GetPageSize() - 1; break;
					case VK_HOME:
						m_nSelected = 0; break;
					case VK_END:
						m_nSelected = (int)(m_vecItems.size()) - 1; break;
					}

					// Perform capping
					if (m_nSelected < 0)
						m_nSelected = 0;
					if (m_nSelected >= (int)(m_vecItems.size()))
						m_nSelected = (int)(m_vecItems.size()) - 1;

					if (nOldSelected != m_nSelected)
					{
						if (m_emStyle == EmListBox::eMultiSelection)
						{
							// Multiple selection

							// Clear all selection
							uint32_t nSize = m_vecItems.size();
							for (uint32_t i = 0; i < nSize; ++i)
							{
								UIListBoxItem& item = m_vecItems[i];
								item.bSelected = false;
							}

							if (GetKeyState(VK_SHIFT) < 0)
							{
								// Select all items from m_nSelStart to
								// m_nSelected
								int nEnd = std::max(m_nSelStart, m_nSelected);

								for (int n = std::min(m_nSelStart, m_nSelected); n <= nEnd; ++n)
								{
									m_vecItems[n].bSelected = true;
								}
							}
							else
							{
								m_vecItems[m_nSelected].bSelected = true;

								// Update selection start
								m_nSelStart = m_nSelected;
							}
						}
						else
						{
							m_nSelStart = m_nSelected;
						}

						// Adjust scroll bar

						m_pScrollBar->ShowItem(m_nSelected);

						Invalidate();
					}
					return true;
				}

				// Space is the hotkey for double-clicking an item.
				//
				case VK_SPACE:
					return true;
				}
				break;
			}

			return false;
		}

		bool CUIListBox::HandleMouse(uint32_t uMsg, POINT pt, WPARAM wParam, LPARAM lParam)
		{
			if (!m_bEnable || !m_bVisible)
				return false;

			// First acquire focus
			if (WM_LBUTTONDOWN == uMsg && m_bHasFocus == false)
			{
				m_pUIPanel->RequestFocus(this);
			}

			// Let the scroll bar handle it first.
			if (m_pScrollBar->HandleMouse(uMsg, pt, wParam, lParam))
				return true;

			switch (uMsg)
			{
			case WM_LBUTTONDOWN:
			case WM_LBUTTONDBLCLK:
				// Check for clicks in the text area
				if (m_vecItems.empty() == false && PtInRect(&m_rcSelection, pt))
				{
					// Compute the index of the clicked item

					int nClicked = -1;
					if (m_nTextHeight)
					{
						nClicked = m_pScrollBar->GetTrackPos() + (pt.y - m_rcText.top) / m_nTextHeight;
					}

					// Only proceed if the click falls on top of an item.

					if (nClicked >= m_pScrollBar->GetTrackPos() &&
						nClicked < (int)(m_vecItems.size()) &&
						nClicked < m_pScrollBar->GetTrackPos() + m_pScrollBar->GetPageSize())
					{
						SetCapture(m_pUIPanel->GetHWND());
						m_bDrag = true;

						// If this is a double click, fire off an event and exit
						// since the first click would have taken care of the selection
						// updating.
						if (uMsg == WM_LBUTTONDBLCLK)
							return true;

						m_nSelected = nClicked;
						if (!(wParam & MK_SHIFT))
						{
							m_nSelStart = m_nSelected;
						}

						// If this is a multi-selection listbox, update per-item
						// selection data.

						if (m_emStyle == EmListBox::eMultiSelection)
						{
							// Determine behavior based on the state of Shift and Ctrl

							UIListBoxItem& selItem = m_vecItems[m_nSelected];
							if ((wParam & (MK_SHIFT | MK_CONTROL)) == MK_CONTROL)
							{
								// Control click. Reverse the selection of this item.

								selItem.bSelected = !selItem.bSelected;
							}
							else if ((wParam & (MK_SHIFT | MK_CONTROL)) == MK_SHIFT)
							{
								// Shift click. Set the selection for all items
								// from last selected item to the current item.
								// Clear everything else.

								int nBegin = std::min(m_nSelStart, m_nSelected);
								int nEnd = std::max(m_nSelStart, m_nSelected);

								for (int i = 0; i < nBegin; ++i)
								{
									UIListBoxItem& item = m_vecItems[i];
									item.bSelected = false;
								}

								uint32_t nSize = m_vecItems.size();
								for (uint32_t i = nEnd + 1; i < nSize; ++i)
								{
									UIListBoxItem& item = m_vecItems[i];
									item.bSelected = false;
								}

								for (int i = nBegin; i <= nEnd; ++i)
								{
									UIListBoxItem& item = m_vecItems[i];
									item.bSelected = false;
								}
							}
							else if ((wParam & (MK_SHIFT | MK_CONTROL)) == (MK_SHIFT | MK_CONTROL))
							{
								// Control-Shift-click.

								// The behavior is:
								//   Set all items from m_nSelStart to m_nSelected to
								//     the same state as m_nSelStart, not including m_nSelected.
								//   Set m_nSelected to selected.

								int nBegin = std::min(m_nSelStart, m_nSelected);
								int nEnd = std::max(m_nSelStart, m_nSelected);

								// The two ends do not need to be set here.

								bool bLastSelected = m_vecItems[m_nSelStart].bSelected;
								for (int i = nBegin + 1; i < nEnd; ++i)
								{
									UIListBoxItem& item = m_vecItems[i];
									item.bSelected = bLastSelected;
								}

								selItem.bSelected = true;

								// Restore m_nSelected to the previous value
								// This matches the Windows behavior

								m_nSelected = m_nSelStart;
							}
							else
							{
								// Simple click.  Clear all items and select the clicked
								// item.

								uint32_t nSize = m_vecItems.size();
								for (uint32_t i = 0; i < nSize; ++i)
								{
									UIListBoxItem& item = m_vecItems[i];
									item.bSelected = false;
								}

								selItem.bSelected = true;
							}
						}  // End of multi-selection case

						Invalidate();
					}

					return true;
				}
				break;
			case WM_LBUTTONUP:
			{
				ReleaseCapture();
				m_bDrag = false;

				if (m_nSelected != -1)
				{
					// Set all items between m_nSelStart and m_nSelected to
					// the same state as m_nSelStart
					int nEnd = std::max(m_nSelStart, m_nSelected);

					for (int n = std::min(m_nSelStart, m_nSelected) + 1; n < nEnd; ++n)
					{
						m_vecItems[n].bSelected = m_vecItems[m_nSelStart].bSelected;
					}
					m_vecItems[m_nSelected].bSelected = m_vecItems[m_nSelStart].bSelected;

					Invalidate();
				}
				return false;
			}
			case WM_MOUSEMOVE:
				if (m_bDrag)
				{
					// Compute the index of the item below cursor

					int nItem;
					if (m_nTextHeight)
					{
						nItem = m_pScrollBar->GetTrackPos() + (pt.y - m_rcText.top) / m_nTextHeight;
					}
					else
					{
						nItem = -1;
					}

					// Only proceed if the cursor is on top of an item.

					if (nItem >= m_pScrollBar->GetTrackPos() &&
						nItem < (int)(m_vecItems.size()) &&
						nItem < m_pScrollBar->GetTrackPos() + m_pScrollBar->GetPageSize())
					{
						m_nSelected = nItem;
					}
					else if (nItem < (int)(m_pScrollBar->GetTrackPos()))
					{
						// User drags the mouse above window top
						m_pScrollBar->Scroll(-1);
						m_nSelected = m_pScrollBar->GetTrackPos();
					}
					else if (nItem >= m_pScrollBar->GetTrackPos() + m_pScrollBar->GetPageSize())
					{
						// User drags the mouse below window bottom
						m_pScrollBar->Scroll(1);
						m_nSelected = std::min((int)(m_vecItems.size()), m_pScrollBar->GetTrackPos() + m_pScrollBar->GetPageSize()) - 1;
					}

					Invalidate();
				}
				break;
			case WM_MOUSEWHEEL:
			{
				uint32_t uLines = 0;
				if (SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &uLines, 0) == false)
				{
					uLines = 0;
				}
				int nScrollAmount = (int)((short)(HIWORD(wParam))) / WHEEL_DELTA * uLines;
				m_pScrollBar->Scroll(-nScrollAmount);

				Invalidate();
				return true;
			}
			}

			return false;
		}

		void CUIListBox::UpdateRects()
		{
			CUIObject::UpdateRects();

			m_rcSelection = m_rcBoundingBox;
			m_rcSelection.right -= m_nSBWidth;
			InflateRect(&m_rcSelection, -m_nBorder, -m_nBorder);
			m_rcText = m_rcSelection;
			InflateRect(&m_rcText, -m_nMargin, 0);

			// Update the scrollbar's rects
			m_pScrollBar->SetPosition(m_rcBoundingBox.right - m_nSBWidth, m_rcBoundingBox.top);
			m_pScrollBar->SetSize(m_nSBWidth, m_nHeight);

			if (m_vecElements.empty())
				return;

			UIFontNode* pFontNode = m_vecElements[0]->GetFontNode();
			if (pFontNode != nullptr)
			{
				m_pScrollBar->SetPageSize(m_rcText.GetHeight() / (int)(pFontNode->fHeight));

				m_pScrollBar->ShowItem(m_nSelected);
			}
		}

		bool CUIListBox::AddItem(const char* strText, void* pData)
		{
			UIListBoxItem newItem;
			newItem.strText = strText;
			newItem.pData = pData;
			SetRect(&newItem.rcActive, 0, 0, 0, 0);
			newItem.bSelected = false;

			m_vecItems.push_back(newItem);
			m_pScrollBar->SetTrackRange(0, (int)(m_vecItems.size()));

			Invalidate();

			return true;
		}

		bool CUIListBox::InsertItem(int nIndex, const char* strText, void* pData)
		{
			UIListBoxItem newItem;
			newItem.strText = strText;
			newItem.pData = pData;
			SetRect(&newItem.rcActive, 0, 0, 0, 0);
			newItem.bSelected = false;

			m_vecItems[nIndex] = newItem;
			m_pScrollBar->SetTrackRange(0, (int)(m_vecItems.size()));

			Invalidate();

			return true;
		}

		void CUIListBox::RemoveItem(int nIndex)
		{
			int nSize = (int)(m_vecItems.size());
			if (nIndex < 0 || nIndex >= nSize)
				return;

			auto iter = m_vecItems.begin() + nIndex;
			m_vecItems.erase(iter);

			m_pScrollBar->SetTrackRange(0, nSize);
			if (m_nSelected >= nSize)
			{
				m_nSelected = nSize - 1;
			}

			Invalidate();
		}

		void CUIListBox::RemoveAllItems()
		{
			m_vecItems.clear();

			if (m_pScrollBar != nullptr)
			{
				m_pScrollBar->SetTrackRange(0, 1);
			}

			Invalidate();
		}

		UIListBoxItem* CUIListBox::GetItem(int nIndex)
		{
			if (nIndex < 0 || nIndex >= (int)(m_vecItems.size()))
				return nullptr;

			return &m_vecItems[nIndex];
		}

		int CUIListBox::GetSelectedIndex(int nPreviousSelected)
		{
			if (nPreviousSelected < -1)
				return -1;

			if (m_emStyle == EmListBox::eMultiSelection)
			{
				// Multiple selection enabled. Search for the next item with the selected flag.
				uint32_t nSize = m_vecItems.size();
				for (uint32_t i = nPreviousSelected + 1; i < nSize; ++i)
				{
					UIListBoxItem& item = m_vecItems[i];

					if (item.bSelected)
						return i;
				}

				return -1;
			}
			else
			{
				// Single selection
				return m_nSelected;
			}
		}

		void CUIListBox::SelectItem(int nNewIndex)
		{
			if (m_vecItems.empty())
				return;

			int nOldSelected = m_nSelected;

			// Adjust m_nSelected
			m_nSelected = nNewIndex;

			// Perform capping
			if (m_nSelected < 0)
				m_nSelected = 0;
			if (m_nSelected >= (int)(m_vecItems.size()))
				m_nSelected = (int)(m_vecItems.size()) - 1;

			if (nOldSelected != m_nSelected)
			{
				if (m_emStyle == EmListBox::eMultiSelection)
				{
					m_vecItems[m_nSelected].bSelected = true;
				}

				// Update selection start
				m_nSelStart = m_nSelected;

				// Adjust scroll bar
				m_pScrollBar->ShowItem(m_nSelected);
			}

			Invalidate();
		}
	}
}