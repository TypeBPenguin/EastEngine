#pragma once

#include "UIElement.h"

namespace EastEngine
{
	namespace Input
	{
		class IKeyboard;
		class IMouse;
	}

	namespace UI
	{
		class IUIObject
		{
		public:
			IUIObject();
			virtual ~IUIObject() = 0;

			virtual void Release() = 0;

			virtual void Update(float fElapsedTime) = 0;

			virtual bool HandleMsg(HWND hWnd, uint32_t nMsg, WPARAM wParam, LPARAM lParam) = 0;
			virtual bool HandleKeyboard(uint32_t nMsg, WPARAM wParam, LPARAM lParam) = 0;
			virtual bool HandleMouse(uint32_t nMsg, POINT pt, WPARAM wParam, LPARAM lParam) = 0;

			virtual IUIElement* CreateElement(String::StringID strID, EmUI::ElementType emType) = 0;
			virtual IUIElement* GetElement(String::StringID strID) = 0;

			virtual void UpdateRects() = 0;

			virtual void Invalidate() = 0;

		public:
			virtual void OnHotKey() = 0;

			virtual void OnMouseEnter() = 0;
			virtual void OnMouseLeave() = 0;

			virtual bool IsMousePressed() = 0;
			virtual void OnMousePressedIn() = 0;
			virtual void OnMousePressedOut() = 0;

			virtual void OnKeyboardTyping(Input::EmKeyboard::Button emKeyboardButton) = 0;

			virtual uint32_t GetHotKey() = 0;

			virtual bool IsFocus() = 0;
			virtual void OnFocusIn() = 0;
			virtual void OnFocusOut() = 0;

			virtual bool IsContainsPoint(const POINT& pt) = 0;
			virtual bool CanHaveFocus() = 0;

		public:
			virtual IUIObject* GetParent() = 0;
			virtual IUIObject* GetChildByID(String::StringID strID) = 0;
			virtual void AddChild(String::StringID strID, IUIObject* pUIObject) = 0;

			virtual IUIObject* GetMouseOverUI() = 0;

		public:
			virtual void SetEnterSound(String::StringID strSoundName) = 0;
			virtual void SetLeaveSound(String::StringID strSoundName) = 0;

			virtual void SetKeyTypingSound(String::StringID strSoundName) = 0;

			virtual void SetEnterAnim() = 0;
			virtual void SetLeaveAnim() = 0;
			virtual void SetStayAnim() = 0;

			virtual void SetMouseEnterScript(String::StringID strScpFuncName) = 0;
			virtual void SetMouseLeaveScript(String::StringID strScpFuncName) = 0;
			virtual void SetMouseClickScript(Input::EmMouse::Button emMouseButton, String::StringID strScpFuncName) = 0;
			virtual void SetMouseDoubleClickScript(Input::EmMouse::Button emMouseButton, String::StringID strScpFuncName) = 0;
			virtual void SetMouseDragScript(Input::EmMouse::Button emMouseButton, String::StringID strScpFuncName) = 0;

		public:
			virtual String::StringID GetID() = 0;
			virtual EmUI::Type GetType() = 0;

			virtual void SetPosition(int x, int y) = 0;
			virtual int GetPositionX() = 0;
			virtual int GetPositionY() = 0;

			virtual void SetSize(uint32_t nWidth, uint32_t nHeight) = 0;
			virtual uint32_t GetWidth() = 0;
			virtual uint32_t GetHeight() = 0;

			virtual bool IsVisible() = 0;
			virtual void SetVisible(bool bVisible) = 0;

			virtual bool IsEnable() = 0;
			virtual void SetEnable(bool bEnable) = 0;

			virtual bool IsMoveAble() = 0;
			virtual void SetMoveAble(bool bMoveAble) = 0;

			virtual bool IsResizeAble() = 0;
			virtual void SetResizeAble(bool bResizeAble) = 0;
		};
	}
}