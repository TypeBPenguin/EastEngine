#pragma once

#include "UIButton.h"

namespace EastEngine
{
	namespace UI
	{
		struct UIComboBoxItem
		{
			std::string strText;
			void* pData;

			Math::Rect rcActive;
			bool bVisible;
		};

		typedef std::vector<UIComboBoxItem> VecComboBoxItem;

		class CUIScrollBar;

		class CUIComboBox : public CUIButton
		{
		public:
			CUIComboBox(IUIPanel* pPanel, String::StringID strID, EmUI::Type emUIType = EmUI::eComboBox);
			virtual ~CUIComboBox();

			virtual void Update(float fElapsedTime) override;

			virtual bool HandleKeyboard(uint32_t uMsg, WPARAM wParam, LPARAM lParam) override;
			virtual bool HandleMouse(uint32_t uMsg, POINT pt, WPARAM wParam, LPARAM lParam) override;
			virtual void UpdateRects();

			virtual void OnHotkey();

			virtual bool CanHaveFocus() { return (m_bVisible && m_bEnable); }
			virtual void OnFocusOut();

		public:
			bool AddItem(const char* strText, void* pData);
			void RemoveAllItems();
			void RemoveItem(uint32_t nIdx);

			bool ContainsItem(const char* strText, uint32_t nStart = 0) { return (-1 != FindItem(strText, nStart)); }

			int FindItem(const char* strText, uint32_t nStart = 0);

			void* GetItemData(const char* strText);
			void* GetItemData(int nIdx);

			void SetDropHeight(uint32_t nHeight) { m_nDropHeight = nHeight; UpdateRects(); }

			int GetScrollBarWidth() const { return m_nSBWidth; }
			void SetScrollBarWidth(int nWidth) { m_nSBWidth = nWidth; UpdateRects(); }

			int GetSelectedIndex() const { return m_nSelected; }
			void* GetSelectedData();
			UIComboBoxItem* GetSelectedItem();

			uint32_t GetNumItems() { return m_vecItems.size(); }
			UIComboBoxItem* GetItem(uint32_t nIdx) { return &m_vecItems[nIdx]; }

			void SetSelectedByIndex(uint32_t nIdx);
			void SetSelectedByText(const char* strText);
			void SetSelectedByData(void* pData);

			Math::Rect GetRectDropDown() { return m_rcDropdown; }

		protected:
			int m_nSelected;
			int m_nFocused;
			int m_nDropHeight;
			int m_nPrevDropHeight;
			int m_nMaxDropHeight;
			int m_nSBWidth;
			CUIScrollBar* m_pScrollBar;

			bool m_bOpened;
			bool m_bScrollBarInit;

			Math::Rect m_rcText;
			Math::Rect m_rcButton;
			Math::Rect m_rcDropdown;
			Math::Rect m_rcDropdownText;

			VecComboBoxItem m_vecItems;
		};
	}
}