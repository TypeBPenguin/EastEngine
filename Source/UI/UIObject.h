#pragma once

#include "IUIObject.h"

namespace EastEngine
{
	namespace UI
	{
		class IUIPanel;

		class CUIObject : public IUIObject
		{
		public:
			CUIObject(IUIPanel* pUIPanel, const String::StringID& strID, EmUI::Type emType);
			virtual ~CUIObject() override;

			virtual void Release() override;

			virtual void Update(float fElapsedTime) = 0;

			virtual bool HandleMsg(HWND hWnd, uint32_t nMsg, WPARAM wParam, LPARAM lParam) { return false; }
			virtual bool HandleKeyboard(uint32_t nMsg, WPARAM wParam, LPARAM lParam) = 0;
			virtual bool HandleMouse(uint32_t nMsg, POINT pt, WPARAM wParam, LPARAM lParam) = 0;

			virtual IUIElement* CreateElement(const String::StringID& strID, EmUI::ElementType emType) override;
			virtual IUIElement* GetElement(const String::StringID& strID) override;

			virtual void UpdateRects() override { SetRect(&m_rcBoundingBox, m_nPosX, m_nPosY, m_nPosX + m_nWidth, m_nPosY + m_nHeight); }

			virtual void Invalidate() override;

		public:
			virtual void OnHotKey() override {}

			virtual void OnMouseEnter() override { m_bMouseOver = true; }
			virtual void OnMouseLeave() override { m_bMouseOver = false; }

			virtual bool IsMousePressed() override { return m_bPressed; }
			virtual void OnMousePressedIn() override { m_bPressed = true; }
			virtual void OnMousePressedOut() override { m_bPressed = false; }

			virtual void OnKeyboardTyping(Input::EmKeyboard::Button emKeyboardButton) override {}

			virtual uint32_t GetHotKey() override { return 0; }

			virtual bool IsFocus() override { return m_bHasFocus; }
			virtual void OnFocusIn() override { m_bHasFocus = true; }
			virtual void OnFocusOut() override { m_bHasFocus = false; }

			virtual bool IsContainsPoint(const POINT& pt) { return PtInRect(&m_rcBoundingBox, pt) == TRUE; }
			virtual bool CanHaveFocus() { return (m_bVisible && m_bEnable); }

		public:
			virtual IUIObject* GetParent() override { return m_pParent; }
			virtual IUIObject* GetChildByID(const String::StringID& strID) override;
			virtual void AddChild(const String::StringID& strID, IUIObject* pUIObject) override { m_umapChildUI.emplace(strID, pUIObject); }

			virtual IUIObject* GetMouseOverUI() override { return nullptr; }

		public:
			virtual void SetEnterSound(const String::StringID& strSoundName) override { m_strEnterSound = strSoundName; }
			virtual void SetLeaveSound(const String::StringID& strSoundName) override { m_strLeaveSound = strSoundName; }

			virtual void SetKeyTypingSound(const String::StringID& strSoundName) override { m_strKeyTypingSound = strSoundName; }

			virtual void SetEnterAnim() override {}
			virtual void SetLeaveAnim() override {}
			virtual void SetStayAnim() override {}

			virtual void SetMouseEnterScript(const String::StringID& strScpFuncName) override { m_strMouseEnterScp = strScpFuncName; }
			virtual void SetMouseLeaveScript(const String::StringID& strScpFuncName) override { m_strMouseLeaveScp = strScpFuncName; }
			virtual void SetMouseClickScript(Input::EmMouse::Button emMouseButton, const String::StringID& strScpFuncName) override { m_strMouseClickScp[emMouseButton] = strScpFuncName; }
			virtual void SetMouseDoubleClickScript(Input::EmMouse::Button emMouseButton, const String::StringID& strScpFuncName) override { m_strMouseDoubleClickScp[emMouseButton] = strScpFuncName; }
			virtual void SetMouseDragScript(Input::EmMouse::Button emMouseButton, const String::StringID& strScpFuncName) override { m_strMouseDragScp[emMouseButton] = strScpFuncName; }

		public:
			virtual const String::StringID& GetID() override { return m_strID; }
			virtual EmUI::Type GetType() override { return m_emType; }

			virtual void SetPosition(int x, int y) override { m_nPosX = x; m_nPosY = y; UpdateRects(); }
			virtual int GetPositionX() override { return m_nPosX; }
			virtual int GetPositionY() override { return m_nPosY; }

			virtual void SetSize(uint32_t nWidth, uint32_t nHeight) override { m_nWidth = nWidth; m_nHeight = nHeight; UpdateRects(); }
			virtual uint32_t GetWidth() override { return m_nWidth; }
			virtual uint32_t GetHeight() override { return m_nHeight; }

			virtual bool IsVisible() override { return m_bVisible; }
			virtual void SetVisible(bool bVisible) override { m_bVisible = bVisible; }

			virtual bool IsEnable() override { return m_bEnable; }
			virtual void SetEnable(bool bEnable) override { m_bEnable = bEnable; }

			virtual bool IsMoveAble() override { return m_bMoveAble; }
			virtual void SetMoveAble(bool bMoveAble) override { m_bMoveAble = bMoveAble; }

			virtual bool IsResizeAble() override { return m_bResizeAble; }
			virtual void SetResizeAble(bool bResizeAble) override { m_bResizeAble = bResizeAble; }

		protected:
			IUIPanel* m_pUIPanel;

			bool m_bVisible;
			bool m_bMouseOver;
			bool m_bHasFocus;
			bool m_bDefault;

			bool m_bEnable;
			bool m_bMoveAble;
			bool m_bPressed;
			bool m_bResizeAble;

			uint32_t m_nHotKey;

			Math::Rect m_rcBoundingBox;

			int m_nPosX, m_nPosY;
			int m_nWidth, m_nHeight;

			std::vector<IUIElement*> m_vecElements;

			IUIObject* m_pParent;
			std::unordered_map<String::StringID, IUIObject*> m_umapChildUI;

		protected:
			String::StringID m_strEnterSound;
			String::StringID m_strLeaveSound;
			String::StringID m_strKeyTypingSound;

			String::StringID m_strMouseEnterScp;
			String::StringID m_strMouseLeaveScp;
			String::StringID m_strMouseClickScp[Input::EmMouse::eCount];
			String::StringID m_strMouseDoubleClickScp[Input::EmMouse::eCount];
			String::StringID m_strMouseDragScp[Input::EmMouse::eCount];

		private:
			String::StringID m_strID;
			EmUI::Type m_emType;
		};
	}
}