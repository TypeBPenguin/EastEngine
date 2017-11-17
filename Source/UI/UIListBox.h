#pragma once

#include "UIObject.h"

namespace EastEngine
{
	namespace UI
	{
		struct UIListBoxItem
		{
			std::string strText;
			void* pData;

			Math::Rect rcActive;
			bool bSelected;
		};

		namespace EmListBox
		{
			enum Style
			{
				eSingleSelection = 0,
				eMultiSelection,

				eCount,
			};
		}

		class CUIScrollBar;

		class CUIListBox : public CUIObject
		{
		public:
			CUIListBox(IUIPanel* pPanel, String::StringID strID, EmUI::Type emUIType = EmUI::eStatic);
			virtual ~CUIListBox();

			virtual void Update(float fElapsedTime) override;

			virtual bool CanHaveFocus() override { return m_bVisible && m_bEnable; }

			virtual bool HandleMsg(HWND hWnd, uint32_t uMsg, WPARAM wParam, LPARAM lParam)  override;
			virtual bool HandleKeyboard(uint32_t uMsg, WPARAM wParam, LPARAM lParam)  override;
			virtual bool HandleMouse(uint32_t uMsg, POINT pt, WPARAM wParam, LPARAM lParam)  override;

			virtual void UpdateRects() override;

			size_t GetSize() const { return m_vecItems.size(); }

			int GetScrollBarWidth() const { return m_nSBWidth; }
			void SetScrollBarWidth(int nWidth) { m_nSBWidth = nWidth; UpdateRects(); }

			void SetBorder(int nBorder, int nMargin) { m_nBorder = nBorder; m_nMargin = nMargin; }

			bool AddItem(const char* strText, void* pData);
			bool InsertItem(int nIndex, const char* strText, void* pData);
			void RemoveItem(int nIndex);
			void RemoveAllItems();

			UIListBoxItem* GetItem(int nIndex);
			int GetSelectedIndex(int nPreviousSelected = -1);
			UIListBoxItem* GetSelectedItem(int nPreviousSelected = -1) { return GetItem(GetSelectedIndex(nPreviousSelected)); }
			void SelectItem(int nNewIndex);

		public:
			void SetStyle(EmListBox::Style emStyle) { m_emStyle = emStyle; }
			EmListBox::Style GetStyle() { return m_emStyle; }

		protected:
			Math::Rect m_rcText;
			Math::Rect m_rcSelection;
			CUIScrollBar* m_pScrollBar;
			int m_nSBWidth;
			int m_nBorder;
			int m_nMargin;
			int m_nTextHeight;
			EmListBox::Style m_emStyle;
			int m_nSelected;
			int m_nSelStart;
			bool m_bDrag;
			bool m_bScrollBarInit;

			std::vector<UIListBoxItem> m_vecItems;
		};
	}
}