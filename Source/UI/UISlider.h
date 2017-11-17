#pragma once

#include "UIObject.h"

namespace EastEngine
{
	namespace UI
	{
		class CUISlider : public CUIObject
		{
		public:
			CUISlider(IUIPanel* pPanel, String::StringID strID, EmUI::Type emUIType = EmUI::eSlider);
			virtual ~CUISlider();

			virtual void Update(float fElapsedTime) override;

			virtual void UpdateRects();

			virtual bool IsContainsPoint(POINT pt) { return (PtInRect(&m_rcBoundingBox, pt) || PtInRect(&m_rcButton, pt)); }
			virtual bool CanHaveFocus() { return (m_bVisible && m_bEnable); }
			virtual bool HandleKeyboard(uint32_t nMsg, WPARAM wParam, LPARAM lParam);
			virtual bool HandleMouse(uint32_t nMsg, POINT pt, WPARAM wParam, LPARAM lParam);

			void SetValue(int nValue) { setValueInternal(nValue, false); }
			int GetValue() const { return m_nValue; };

			void GetRange(int& nMin, int& nMax) const { nMin = m_nMin; nMax = m_nMax; }
			void SetRange(int nMin, int nMax);

		protected:
			void setValueInternal(int nValue, bool bFromInput);
			int valueFromPos(int x);

		protected:
			int m_nValue;

			int m_nMin;
			int m_nMax;

			int m_nDragX;      // Mouse position at start of drag
			int m_nDragOffset; // Drag offset from the center of the button
			int m_nButtonX;

			Math::Rect m_rcButton;
		};
	}
}