#include "stdafx.h"
#include "UIEditBox.h"

#include "CommonLib/Timer.h"

#include "IUIPanel.h"

#define IN_FLOAT_CHARSET( c ) \
    ( (c) == L'-' || (c) == L'.' || ( (c) >= L'0' && (c) <= L'9' ) )

namespace EastEngine
{
	namespace UI
	{
		CUIEditBox::CUIEditBox(IUIPanel* pPanel, String::StringID strID, EmUI::Type emUIType)
			: CUIObject(pPanel, strID, emUIType)
			, m_nBorder(5)
			, m_nSpacing(4)
			, m_bCaretOn(true)
			, m_dfBlink(GetCaretBlinkTime() * 0.001)
			, m_nFirstVisible(0)
			, m_nLastVisible(0)
			, m_TextColor(0.0627f, 0.0627f, 0.0627f, 1.f)
			, m_SelTextColor(Math::Color::White)
			, m_SelBkColor(0.1569f, 0.1961f, 0.3608f, 1.f)
			, m_CaretColor(Math::Color::Black)
			, m_nCaret(0)
			, m_nSelStart(0)
			, m_bInsertMode(true)
			, m_bMouseDrag(false)
			, m_bHideCaret(false)
			, m_bImeFlag(true)
		{
			m_dfLastBlink = Timer::GetInstance()->GetGameTime();
		}

		CUIEditBox::~CUIEditBox()
		{
		}

		void CUIEditBox::IME_Initialize(_In_ HWND hWnd)
		{
			ImeUiCallback_DrawRect = nullptr;
			ImeUiCallback_Malloc = malloc;
			ImeUiCallback_Free = free;
			ImeUiCallback_DrawFans = nullptr;

			ImeUi_Initialize(hWnd);

			ImeUi_EnableIme(true);
		}

		void CUIEditBox::IME_Uninitialize()
		{
			ImeUi_Uninitialize();

			ImeUi_EnableIme(false);
		}

		void CUIEditBox::Update(float fElapsedTime)
		{
			if (m_bVisible == false)
				return;

			if (m_vecElements.empty())
				return;

			//HRESULT hr;
			int nSelStartX = 0;
			int nCaretX = 0;  // Left and right X cordinates of the selection region

			bool bNeedInvalidate = false;

			IUIElement* pElement = m_vecElements[0];

			m_Buffer.SetFont(pElement->GetFontNode());

			//PlaceCaret(m_nCaret);  // Call PlaceCaret now that we have the font info (node),
			// so that scrolling can be handled.

			// Render the control graphics
			for (int i = 0; i < 9; ++i)
			{
				pElement = m_vecElements[i + 1];
				bNeedInvalidate = pElement->BlendColor(EmUI::eNormal, fElapsedTime) == false || bNeedInvalidate;

				m_pUIPanel->DrawSprite(pElement, m_rcRender[i], UI_FAR_DEPTH);
			}

			//
			// Compute the X coordinates of the first visible character.
			//
			int nXFirst = m_Buffer.GetPosXByIndex(m_nFirstVisible);
			//m_Buffer.CPtoX(m_nFirstVisible, false, &nXFirst);

			//
			// Compute the X coordinates of the selection rectangle
			//
			nCaretX = m_Buffer.GetPosXByIndex(m_nCaret);
			//m_Buffer.CPtoX(m_nCaret, false, &nCaretX);
			if (m_nCaret != m_nSelStart)
			{
				//m_Buffer.CPtoX(m_nSelStart, false, &nSelStartX);
				nSelStartX = m_Buffer.GetPosXByIndex(m_nSelStart);
			}
			else
			{
				nSelStartX = nCaretX;
			}

			//
			// Render the selection rectangle
			//
			Math::Rect rcSelection;  // Make this available for rendering selected text
			if (m_nCaret != m_nSelStart)
			{
				int nSelLeftX = nCaretX, nSelRightX = nSelStartX;
				// Swap if left is bigger than right
				if (nSelLeftX > nSelRightX)
				{
					int nTemp = nSelLeftX; nSelLeftX = nSelRightX; nSelRightX = nTemp;
				}

				SetRect(&rcSelection, nSelLeftX, m_rcText.top, nSelRightX, m_rcText.bottom);
				OffsetRect(&rcSelection, m_rcText.left - nXFirst, 0);
				IntersectRect(&rcSelection, &m_rcText, &rcSelection);

				m_vecElements[10]->SetColorCurrent(m_SelBkColor);

				m_pUIPanel->DrawSprite(m_vecElements[10], rcSelection, UI_NEAR_DEPTH);
			}

			//
			// Render the text
			//
			// Element 0 for text
			if (m_Buffer.GetTextSize() != 0)
			{
				pElement = m_vecElements[0];
				pElement->SetColorCurrent(m_TextColor);

				//IFontNode* pFont = pElement->GetFontNode();
				m_pUIPanel->DrawText(pElement, m_Buffer.GetBuffer() + m_nFirstVisible, m_rcText, UI_NEAR_DEPTH);

				// Render the selected text
				if (m_nCaret != m_nSelStart)
				{
					int nFirstToRender = std::max(m_nFirstVisible, std::min(m_nSelStart, m_nCaret));
					//int nNumChatToRender = std::max(m_nSelStart, m_nCaret) - nFirstToRender;
					pElement->SetColorCurrent(m_SelTextColor);
					m_pUIPanel->DrawText(pElement, m_Buffer.GetBuffer() + nFirstToRender, rcSelection, UI_NEAR_DEPTH);
				}
			}

			//
			// Blink the caret
			//
			double dGameTime = Timer::GetInstance()->GetGameTime();
			if (dGameTime - m_dfLastBlink >= m_dfBlink)
			{
				m_bCaretOn = !m_bCaretOn;
				m_dfLastBlink = dGameTime;
			}

			//
			// Render the caret if this control has the focus
			//
			if (m_bHasFocus && m_bCaretOn && !m_bHideCaret)
			{
				// Start the rectangle with insert mode caret
				Math::Rect rcCaret =
				{
					m_rcText.left - nXFirst + nCaretX - 1, m_rcText.top,
					m_rcText.left - nXFirst + nCaretX + 1, m_rcText.bottom
				};

				// If we are in overwrite mode, adjust the caret rectangle
				// to fill the entire character.
				if (!m_bInsertMode)
				{
					// Obtain the right edge X coord of the current character
					int nRightEdgeX = m_Buffer.GetPosXByIndex(m_nCaret);
					//m_Buffer.CPtoX(m_nCaret, true, &nRightEdgeX);
					rcCaret.right = m_rcText.left - nXFirst + nRightEdgeX;
				}

				m_vecElements[10]->SetColorCurrent(m_CaretColor);

				m_pUIPanel->DrawSprite(m_vecElements[10], rcCaret, UI_NEAR_DEPTH);
			}

			if (bNeedInvalidate)
			{
				Invalidate();
			}
		}

		void CUIEditBox::UpdateRects()
		{
			CUIObject::UpdateRects();

			// Update the text rectangle
			m_rcText = m_rcBoundingBox;
			// First inflate by m_nBorder to compute render rects
			InflateRect(&m_rcText, -m_nBorder, -m_nBorder);

			// Update the render rectangles
			m_rcRender[0] = m_rcText;
			SetRect(&m_rcRender[1], m_rcBoundingBox.left, m_rcBoundingBox.top, m_rcText.left, m_rcText.top);
			SetRect(&m_rcRender[2], m_rcText.left, m_rcBoundingBox.top, m_rcText.right, m_rcText.top);
			SetRect(&m_rcRender[3], m_rcText.right, m_rcBoundingBox.top, m_rcBoundingBox.right, m_rcText.top);
			SetRect(&m_rcRender[4], m_rcBoundingBox.left, m_rcText.top, m_rcText.left, m_rcText.bottom);
			SetRect(&m_rcRender[5], m_rcText.right, m_rcText.top, m_rcBoundingBox.right, m_rcText.bottom);
			SetRect(&m_rcRender[6], m_rcBoundingBox.left, m_rcText.bottom, m_rcText.left, m_rcBoundingBox.bottom);
			SetRect(&m_rcRender[7], m_rcText.left, m_rcText.bottom, m_rcText.right, m_rcBoundingBox.bottom);
			SetRect(&m_rcRender[8], m_rcText.right, m_rcText.bottom, m_rcBoundingBox.right, m_rcBoundingBox.bottom);

			// Inflate further by m_nSpacing
			InflateRect(&m_rcText, -m_nSpacing, -m_nSpacing);
		}

		bool CUIEditBox::HandleMsg(HWND hWnd, uint32_t nMsg, WPARAM wParam, LPARAM lParam)
		{
			if (!m_bEnable || !m_bVisible)
				return false;

			if (!ImeUi_IsEnabled())
				return MsgProc(hWnd, nMsg, wParam, lParam);

			switch (nMsg)
			{
			case WM_INPUTLANGCHANGE:
				return false;
			case WM_IME_SETCONTEXT:
				return false;
			case WM_IME_STARTCOMPOSITION:
			{
				m_bHideCaret = true;
				m_strComposition.clear();
				return true;
			}
			case WM_IME_ENDCOMPOSITION:
			{
				m_bHideCaret = false;
				return false;
			}
			case WM_IME_COMPOSITION:
			{
				HIMC himc;
				himc = ImmGetContext(hWnd);
				if (himc)
				{
					LONG lRet;
					WCHAR szCompStr[256];

					if (lParam & GCS_RESULTSTR)
					{
						lRet = (LONG)ImmGetCompositionStringW(himc, GCS_RESULTSTR, szCompStr, 256) / sizeof(wchar_t);
						szCompStr[lRet] = 0;

						if (m_strComposition.empty() == false)
						{
							MsgProc(hWnd, WM_CHAR, 8, 1);
						}

						m_strPrevUniCode = String::WideToMulti(szCompStr, CP_ACP);

						if (lRet != 0)
						{
							MsgProc(hWnd, WM_CHAR, static_cast<WPARAM>(szCompStr[0]), lParam);
						}

						m_strComposition.clear();
					}

					if (lParam & GCS_COMPSTR)
					{
						lRet = (LONG)ImmGetCompositionStringW(himc, GCS_COMPSTR, szCompStr, 256) / sizeof(wchar_t);
						szCompStr[lRet] = 0;

						if (m_strComposition.empty() == false)
						{
							MsgProc(hWnd, WM_CHAR, 8, 1);
						}

						m_strComposition = szCompStr;

						if (lRet != 0)
						{
							MsgProc(hWnd, WM_CHAR, static_cast<WPARAM>(szCompStr[0]), lParam);
						}
					}
				}
				ImmReleaseContext(hWnd, himc);
				return true;
			}
			case WM_IME_NOTIFY:
				switch (wParam)
				{
				case IMN_SETCONVERSIONMODE:
				case IMN_SETOPENSTATUS:
					return false;
				case IMN_OPENCANDIDATE:
				case IMN_CHANGECANDIDATE:
				case IMN_CLOSECANDIDATE:
					return true;
				default:
					return true;
				}
				break;
			default:
			{
				static int nIdx = 0;
				if (nMsg == WM_CHAR && m_strPrevUniCode.empty() == false)
				{
					static WPARAM temp = 0;
					if (m_strPrevUniCode[nIdx] == static_cast<char>(wParam))
					{
						if (nIdx == 0)
						{
							temp = wParam;
							++nIdx;
						}
						else
						{
							m_strPrevUniCode.clear();
							nIdx = 0;
						}
					}
					else
					{
						m_strPrevUniCode.clear();

						if (nIdx != 0)
						{
							MsgProc(hWnd, WM_CHAR, temp, lParam);

							temp = 0;
							nIdx = 0;
						}

						return MsgProc(hWnd, nMsg, wParam, lParam);
					}
				}
				else
				{
					MsgProc(hWnd, nMsg, wParam, lParam);
				}
				break;
			}
			}

			return false;
		}

		bool CUIEditBox::MsgProc(HWND hWnd, uint32_t nMsg, WPARAM wParam, LPARAM lParam)
		{
			if (!m_bEnable || !m_bVisible)
				return false;

			switch (nMsg)
			{
				// Make sure that while editing, the keyup and keydown messages associated with 
				// WM_CHAR messages don't go to any non-focused controls or cameras
			case WM_KEYUP:
			case WM_KEYDOWN:
				return true;

			case WM_CHAR:
			{
				switch ((wchar_t)wParam)
				{
					// Backspace
				case VK_BACK:
				{
					// If there's a selection, treat this
					// like a delete key.
					if (m_nCaret != m_nSelStart)
					{
						DeleteSelectionText();
					}
					else if (m_nCaret > 0)
					{
						// Move the caret, then delete the char.
						PlaceCaret(m_nCaret - 1);
						m_nSelStart = m_nCaret;
						m_Buffer.RemoveChar(m_nCaret);
					}
					ResetCaretBlink();

					break;
				}

				case 24:        // Ctrl-X Cut
				case VK_CANCEL: // Ctrl-C Copy
				{
					CopyToClipboard();

					// If the key is Ctrl-X, delete the selection too.
					if ((wchar_t)wParam == 24)
					{
						DeleteSelectionText();
					}

					break;
				}

				// Ctrl-V Paste
				case 22:
				{
					PasteFromClipboard();

					break;
				}

				// Ctrl-A Select All
				case 1:
					if (m_nSelStart == m_nCaret)
					{
						m_nSelStart = 0;
						PlaceCaret(m_Buffer.GetTextSize());
					}
					break;

				case VK_RETURN:
					// Invoke the callback when the user presses Enter.
					break;

					// Junk characters we don't want in the string
				case 26:  // Ctrl Z
				case 2:   // Ctrl B
				case 14:  // Ctrl N
				case 19:  // Ctrl S
				case 4:   // Ctrl D
				case 6:   // Ctrl F
				case 7:   // Ctrl G
				case 10:  // Ctrl J
				case 11:  // Ctrl K
				case 12:  // Ctrl L
				case 17:  // Ctrl Q
				case 23:  // Ctrl W
				case 5:   // Ctrl E
				case 18:  // Ctrl R
				case 20:  // Ctrl T
				case 25:  // Ctrl Y
				case 21:  // Ctrl U
				case 9:   // Ctrl I
				case 15:  // Ctrl O
				case 16:  // Ctrl P
				case 27:  // Ctrl [
				case 29:  // Ctrl ]
				case 28:  // Ctrl \ 
					break;

				default:
				{
					// If there's a selection and the user
					// starts to type, the selection should
					// be deleted.
					if (m_nCaret != m_nSelStart)
						DeleteSelectionText();

					// If we are in overwrite mode and there is already
					// a char at the caret's position, simply replace it.
					// Otherwise, we insert the char as normal.
					if (!m_bInsertMode && m_nCaret < m_Buffer.GetTextSize())
					{
						m_Buffer[m_nCaret] = (wchar_t)wParam;
						PlaceCaret(m_nCaret + 1);
						m_nSelStart = m_nCaret;
					}
					else
					{
						// Insert the char
						if (m_Buffer.InsertChar(m_nCaret, (wchar_t)wParam))
						{
							PlaceCaret(m_nCaret + 1);
							m_nSelStart = m_nCaret;
						}
					}
					ResetCaretBlink();
				}
				}
				return true;
			}
			}
			return false;
		}

		bool CUIEditBox::HandleKeyboard(uint32_t nMsg, WPARAM wParam, LPARAM lParam)
		{
			if (!m_bEnable || !m_bVisible)
				return false;

			bool bHandled = false;

			switch (nMsg)
			{
			case WM_KEYDOWN:
			{
				switch (wParam)
				{
				case VK_TAB:
					// We don't process Tab in case keyboard input is enabled and the user
					// wishes to Tab to other controls.
					break;

				case VK_HOME:
					PlaceCaret(0);
					if (GetKeyState(VK_SHIFT) >= 0)
						// Shift is not down. Update selection
						// start along with the caret.
						m_nSelStart = m_nCaret;
					ResetCaretBlink();
					bHandled = true;
					break;

				case VK_END:
					PlaceCaret(m_Buffer.GetTextSize());
					if (GetKeyState(VK_SHIFT) >= 0)
						// Shift is not down. Update selection
						// start along with the caret.
						m_nSelStart = m_nCaret;
					ResetCaretBlink();
					bHandled = true;
					break;

				case VK_INSERT:
					if (GetKeyState(VK_CONTROL) < 0)
					{
						// Control Insert. Copy to clipboard
						CopyToClipboard();
					}
					else if (GetKeyState(VK_SHIFT) < 0)
					{
						// Shift Insert. Paste from clipboard
						PasteFromClipboard();
					}
					else
					{
						// Toggle caret insert mode
						m_bInsertMode = !m_bInsertMode;
					}
					break;

				case VK_DELETE:
					// Check if there is a text selection.
					if (m_nCaret != m_nSelStart)
					{
						DeleteSelectionText();
					}
					else
					{
						// Deleting one character
						m_Buffer.RemoveChar(m_nCaret);
					}
					ResetCaretBlink();
					bHandled = true;
					break;

				case VK_LEFT:
					if (GetKeyState(VK_CONTROL) < 0)
					{
						// Control is down. Move the caret to a new item
						// instead of a character.
						m_nCaret = m_Buffer.GetPriorTextPos(m_nCaret);
						PlaceCaret(m_nCaret);
					}
					else if (m_nCaret > 0)
						PlaceCaret(m_nCaret - 1);
					if (GetKeyState(VK_SHIFT) >= 0)
						// Shift is not down. Update selection
						// start along with the caret.
						m_nSelStart = m_nCaret;
					ResetCaretBlink();
					bHandled = true;
					break;

				case VK_RIGHT:
					if (GetKeyState(VK_CONTROL) < 0)
					{
						// Control is down. Move the caret to a new item
						// instead of a character.
						m_nCaret = m_Buffer.GetNextTextPos(m_nCaret);
						PlaceCaret(m_nCaret);
					}
					else if (m_nCaret < m_Buffer.GetTextSize())
						PlaceCaret(m_nCaret + 1);
					if (GetKeyState(VK_SHIFT) >= 0)
						// Shift is not down. Update selection
						// start along with the caret.
						m_nSelStart = m_nCaret;
					ResetCaretBlink();
					bHandled = true;
					break;

				case VK_UP:
				case VK_DOWN:
					// Trap up and down arrows so that the dialog
					// does not switch focus to another control.
					bHandled = true;
					break;

				default:
					bHandled = wParam != VK_ESCAPE;  // Let the application handle Esc.
				}
			}
			}
			return bHandled;
		}

		bool CUIEditBox::HandleMouse(uint32_t nMsg, POINT pt, WPARAM wParam, LPARAM lParam)
		{
			if (!m_bEnable || !m_bVisible)
				return false;

			switch (nMsg)
			{
			case WM_LBUTTONDOWN:
			case WM_LBUTTONDBLCLK:
			{
				if (!m_bHasFocus)
					m_pUIPanel->RequestFocus(this);

				if (!IsContainsPoint(pt))
					return false;

				m_bMouseDrag = true;
				SetCapture(m_pUIPanel->GetHWND());
				// Determine the character corresponding to the coordinates.
				int nX1st = m_Buffer.GetPosXByIndex(m_nFirstVisible);
				int nCP = m_Buffer.GetIndexByPosX(pt.x - m_rcText.left + nX1st);

				// Cap at the nullptr character.
				if (nCP < m_Buffer.GetTextSize())
					PlaceCaret(nCP + 1);
				else
					PlaceCaret(nCP);
				m_nSelStart = m_nCaret;
				ResetCaretBlink();

				return true;
			}

			case WM_LBUTTONUP:
				ReleaseCapture();
				m_bMouseDrag = false;
				Invalidate();
				break;

			case WM_MOUSEMOVE:
				if (m_bMouseDrag)
				{
					// Determine the character corresponding to the coordinates.
					int nX1st = m_Buffer.GetPosXByIndex(m_nFirstVisible);
					int nCP = m_Buffer.GetIndexByPosX(pt.x - m_rcText.left + nX1st);

					// Cap at the nullptr character.
					if (nCP < m_Buffer.GetTextSize())
					{
						PlaceCaret(nCP + 1);
					}
					else
					{
						PlaceCaret(nCP);
					}
				}
				break;
			}

			return false;
		}

		void CUIEditBox::OnFocusIn()
		{
			ImeUi_EnableIme(true);

			CUIObject::OnFocusIn();

			ResetCaretBlink();
		}

		void CUIEditBox::OnFocusOut()
		{
			ImeUi_FinalizeString();
			ImeUi_EnableIme(false);
			CUIObject::OnFocusOut();
		}

		void CUIEditBox::SetText(const char* wszText, bool bSelected)
		{
			assert(wszText != nullptr);

			m_Buffer.SetText(String::MultiToWide(wszText, CP_ACP).c_str());
			m_nFirstVisible = 0;
			// Move the caret to the end of the text
			PlaceCaret(m_Buffer.GetTextSize());
			m_nSelStart = bSelected ? 0 : m_nCaret;
		}

		const char* CUIEditBox::GetText()
		{
			m_strBuffer = String::WideToMulti(m_Buffer.GetBuffer(), CP_ACP);
			return m_strBuffer.c_str();
		}

		HRESULT CUIEditBox::GetTextCopy(__out_ecount(bufferCount) char* strDest, uint32_t bufferCount)
		{
			assert(strDest);

			String::Copy(strDest, bufferCount, String::WideToMulti(m_Buffer.GetBuffer(), CP_ACP).c_str());

			return S_OK;
		}

		void CUIEditBox::ClearText()
		{
			m_Buffer.Clear();
			m_nFirstVisible = 0;
			PlaceCaret(0);
			m_nSelStart = 0;
		}

		void CUIEditBox::ParseFloatArray(float* pNumbers, int nCount)
		{
			int nWritten = 0;  // Number of floats written
			const wchar_t* pToken, *pEnd;
			wchar_t wszToken[60];

			pToken = m_Buffer.GetBuffer();
			while (nWritten < nCount && *pToken != L'\0')
			{
				// Skip leading spaces
				while (*pToken == L' ')
					++pToken;

				if (*pToken == L'\0')
					break;

				// Locate the end of number
				pEnd = pToken;
				while (IN_FLOAT_CHARSET(*pEnd))
					++pEnd;

				// Copy the token to our buffer
				int nTokenLen = __min(sizeof(wszToken) / sizeof(wszToken[0]) - 1, int(pEnd - pToken));
				wcscpy_s(wszToken, nTokenLen, pToken);
				*pNumbers = (float)wcstod(wszToken, nullptr);
				++nWritten;
				++pNumbers;
				pToken = pEnd;
			}
		}

		void CUIEditBox::SetTextFloatArray(const float* pNumbers, int nCount)
		{
			char wszBuffer[512] =
			{
				0
			};
			char wszTmp[64];

			if (pNumbers == nullptr)
				return;

			for (int i = 0; i < nCount; ++i)
			{
				sprintf_s(wszTmp, 64, "%.4f ", pNumbers[i]);
				strcat_s(wszBuffer, 512, wszTmp);
			}

			// Don't want the last space
			if (nCount > 0 && strlen(wszBuffer) > 0)
				wszBuffer[strlen(wszBuffer) - 1] = 0;

			SetText(wszBuffer);
		}

		void CUIEditBox::PlaceCaret(int nCP)
		{
			m_nCaret = nCP;

			// Obtain the X offset of the character.
			int nX1st = m_Buffer.GetPosXByIndex(m_nFirstVisible);
			int nX = m_Buffer.GetPosXByIndex(nCP);
			int nX2 = 0;
			// If nCP is the nullptr terminator, get the leading edge instead of trailing.
			if (nCP == m_Buffer.GetTextSize())
			{
				nX2 = nX;
			}
			else
			{
				nX2 = m_Buffer.GetPosXByIndex(nCP);
			}

			// If the left edge of the char is smaller than the left edge of the 1st visible char,
			// we need to scroll left until this char is visible.
			if (nX < nX1st)
			{
				// Simply make the first visible character the char at the new caret position.
				m_nFirstVisible = nCP;
			}
			else // If the right of the character is bigger than the offset of the control's
			{
				// right edge, we need to scroll right to this character.
				if (nX2 > nX1st + m_rcText.GetWidth())
				{
					// Compute the X of the new left-most pixel
					int nXNewLeft = nX2 - m_rcText.GetWidth();

					// Compute the char position of this character
					int nCPNew1st = m_Buffer.GetIndexByPosX(nXNewLeft);

					// If this coordinate is not on a character border,
					// start from the next character so that the caret
					// position does not fall outside the text rectangle.
					int nXNew1st = m_Buffer.GetPosXByIndex(nCPNew1st);
					if (nXNew1st < nXNewLeft)
						++nCPNew1st;

					m_nFirstVisible = nCPNew1st;
				}
			}

			Invalidate();
		}

		void CUIEditBox::DeleteSelectionText()
		{
			int nFirst = std::min(m_nCaret, m_nSelStart);
			int nLast = std::max(m_nCaret, m_nSelStart);
			// Update caret and selection
			PlaceCaret(nFirst);
			m_nSelStart = m_nCaret;
			// Remove the characters
			for (int i = nFirst; i < nLast; ++i)
			{
				m_Buffer.RemoveChar(nFirst);
			}

			Invalidate();
		}

		void CUIEditBox::ResetCaretBlink()
		{
			m_bCaretOn = true;
			m_dfLastBlink = Timer::GetInstance()->GetElapsedTime();

			Invalidate();
		}

		void CUIEditBox::CopyToClipboard()
		{
			// Copy the selection text to the clipboard
			if (m_nCaret != m_nSelStart && OpenClipboard(nullptr))
			{
				EmptyClipboard();

				HGLOBAL hBlock = GlobalAlloc(GMEM_MOVEABLE, sizeof(wchar_t) * (m_Buffer.GetTextSize() + 1));
				if (hBlock)
				{
					char* pwszText = (char*)GlobalLock(hBlock);
					if (pwszText)
					{
						int nFirst = std::min(m_nCaret, m_nSelStart);
						int nLast = std::max(m_nCaret, m_nSelStart);
						if (nLast - nFirst > 0)
							CopyMemory(pwszText, m_Buffer.GetBuffer() + nFirst, (nLast - nFirst) * sizeof(wchar_t));
						pwszText[nLast - nFirst] = '\0';  // Terminate it
						GlobalUnlock(hBlock);
					}
					SetClipboardData(CF_UNICODETEXT, hBlock);
				}
				CloseClipboard();
				// We must not free the object until CloseClipboard is called.
				if (hBlock)
				{
					GlobalFree(hBlock);
				}

				Invalidate();
			}
		}

		void CUIEditBox::PasteFromClipboard()
		{
			DeleteSelectionText();

			if (OpenClipboard(nullptr))
			{
				HANDLE handle = GetClipboardData(CF_UNICODETEXT);
				if (handle)
				{
					// Convert the ANSI string to Unicode, then
					// insert to our buffer.
					wchar_t* pwszText = (wchar_t*)GlobalLock(handle);
					if (pwszText)
					{
						// Copy all characters up to null.
						if (m_Buffer.InsertString(m_nCaret, pwszText))
							PlaceCaret(m_nCaret + wcslen(pwszText));
						m_nSelStart = m_nCaret;
						GlobalUnlock(handle);
					}
				}
				CloseClipboard();

				Invalidate();
			}
		}
	}
}