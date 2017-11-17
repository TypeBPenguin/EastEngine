#pragma once

#include "UIObject.h"

namespace EastEngine
{
	namespace UI
	{
		const int ScrollBar_MinThumbSize = 8;
		const float ScrollBar_ArrowClick_Delay = 0.33f;
		const float ScrollBar_ArrowClick_Repeat = 0.05f;

		class CUIScrollBar : public CUIObject
		{
		public:
			CUIScrollBar(IUIPanel* pPanel, String::StringID strID, EmUI::Type emUIType = EmUI::eScrollBar);
			virtual ~CUIScrollBar();

			virtual void Update(float fElapsedTime) override;

			virtual bool HandleMsg(HWND hWnd, uint32_t nMsg, WPARAM wParam, LPARAM lParam) override;
			virtual bool HandleKeyboard(uint32_t nMsg, WPARAM wParam, LPARAM lParam) override;
			virtual bool HandleMouse(uint32_t nMsg, POINT pt, WPARAM wParam, LPARAM lParam) override;

			virtual void UpdateRects();

			void SetTrackRange(int nStart, int nEnd);

			int GetTrackPos() { return m_nPosition; }
			void SetTrackPos(int nPosition) { m_nPosition = nPosition; cap(); updateThumbRect(); }

			int GetPageSize() { return m_nPageSize; }
			void SetPageSize(int nPageSize) { m_nPageSize = nPageSize; cap(); updateThumbRect(); }

			void Scroll(int nDelta);    // Scroll by nDelta items (plus or minus)
			void ShowItem(int nIndex);  // Ensure that item nIndex is displayed, scroll if necessary

		protected:
			void updateThumbRect();
			void cap();  // Clips position at boundaries. Ensures it stays within legal range.

		protected:
			enum EmArrowState		// 화살표 버튼 상태
			{
				eClear,			// 화살표 버튼 없음
				eClickedUp,		// 위쪽 화살표 버튼 클릭
				eClickedDown,	// 아래쪽 화살표 버튼 클릭
				eHeldUp,		// 위쪽 화살표 버튼 누르고 있음
				eHeldDown,		// 아래쪽 화살표 버튼 누르고 있음
				eCount,
			};

			bool m_bShowThumb;
			bool m_bDrag;
			Math::Rect m_rcUpButton;
			Math::Rect m_rcDownButton;
			Math::Rect m_rcTrack;
			Math::Rect m_rcThumb;
			int m_nPosition;  // Position of the first displayed item
			int m_nPageSize;  // How many items are displayable in one page
			int m_nStart;     // First item
			int m_nEnd;       // The index after the last item

			POINT m_LastMouse;// Last mouse position

			EmArrowState m_emArrow; // State of the arrows

			float m_fArrowTime;  // Timestamp of last arrow event.
			float m_fTime;
		};
	}
}