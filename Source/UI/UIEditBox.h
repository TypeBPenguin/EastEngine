#pragma once

#include "ImeUi.h"

#include "UIObject.h"
#include "UniBuffer.h"

#define MAX_COMPSTRING_SIZE 256

namespace EastEngine
{
	namespace UI
	{
		class CUIEditBox : public CUIObject
		{
		public:
			CUIEditBox(IUIPanel* pPanel, String::StringID strID, EmUI::Type emUIType = EmUI::eEditBox);
			virtual ~CUIEditBox();

			static void IME_Initialize(_In_ HWND hWnd);
			static void IME_Uninitialize();

			void SetImeEnableFlag(bool bFlag) { m_bImeFlag = bFlag; }

			virtual void	Update(float fElapsedTime);
			virtual void	UpdateRects();

			virtual bool	HandleMsg(HWND hWnd, uint32_t nMsg, WPARAM wParam, LPARAM lParam) override;
			bool MsgProc(HWND hWnd, uint32_t nMsg, WPARAM wParam, LPARAM lParam);
			virtual bool	HandleKeyboard(uint32_t nMsg, WPARAM wParam, LPARAM lParam) override;
			virtual bool	HandleMouse(uint32_t nMsg, POINT pt, WPARAM wParam, LPARAM lParam) override;

			virtual bool	CanHaveFocus() { return (m_bVisible && m_bEnable); }
			virtual void	OnFocusIn();
			virtual void	OnFocusOut();

		public:

			void            SetText(const char* wszText, bool bSelected = false);
			const char*		GetText();
			int             GetTextLength() { return m_Buffer.GetTextSize(); }  // Returns text length in chars excluding NULL.

			HRESULT         GetTextCopy(__out_ecount(bufferCount) char* strDest, uint32_t bufferCount);
			void            ClearText();

			virtual void    SetTextColor(Math::Color& Color) { m_TextColor = Color; }  // Text color
			void            SetSelectedTextColor(Math::Color& Color) { m_SelTextColor = Color; }  // Selected text color
			void            SetSelectedBackColor(Math::Color& Color) { m_SelBkColor = Color; }  // Selected background color
			void            SetCaretColor(Math::Color& Color) { m_CaretColor = Color; }  // Caret color
			void            SetBorderWidth(int nBorder) { m_nBorder = nBorder; UpdateRects(); }  // Border of the window
			void            SetSpacing(int nSpacing) { m_nSpacing = nSpacing; UpdateRects(); }

			void            ParseFloatArray(float* pNumbers, int nCount);
			void            SetTextFloatArray(const float* pNumbers, int nCount);

		protected:
			void            PlaceCaret(int nCP);
			void            DeleteSelectionText();
			void            ResetCaretBlink();
			void            CopyToClipboard();
			void            PasteFromClipboard();

		protected:
			CUniBuffer m_Buffer;     // Buffer to hold text
			int m_nBorder;      // Border of the window
			int m_nSpacing;     // Spacing between the text and the edge of border
			Math::Rect m_rcText;       // Bounding rectangle for the text
			Math::Rect m_rcRender[9];  // Convenient rectangles for rendering elements
			double m_dfBlink;      // Caret blink time in milliseconds
			double m_dfLastBlink;  // Last timestamp of caret blink
			bool m_bCaretOn;     // Flag to indicate whether caret is currently visible
			int m_nCaret;       // Caret position, in characters
			bool m_bInsertMode;  // If true, control is in insert mode. Else, overwrite mode.
			int m_nSelStart;    // Starting position of the selection. The caret marks the end.
			int m_nFirstVisible;// First visible character in the edit control
			int m_nLastVisible;
			Math::Color m_TextColor;    // Text color
			Math::Color m_SelTextColor; // Selected text color
			Math::Color m_SelBkColor;   // Selected background color
			Math::Color m_CaretColor;   // Caret color

			// Mouse-specific
			bool m_bMouseDrag;       // True to indicate drag in progress

			std::wstring m_strComposition;
			std::string m_strPrevUniCode;
			std::string m_strBuffer;

			bool m_bHideCaret;   // If true, we don't render the caret.
			bool m_bImeFlag;			  // Is ime enabled
		};
	}
}