#pragma once

#include "../CommonLib/Singleton.h"

#include "Mouse.h"
#include "Keyboard.h"
#include "GamePad.h"

struct IDirectInput8A;

namespace EastEngine
{
	namespace Input
	{
		enum EmCooperativeLevel
		{
			eExclusive = 1 << 0,
			eNoneExclusive = 1 << 1,
			eForeGround = 1 << 2,
			eBackGround = 1 << 3,
			eNoWinKey = 1 << 4,
		};

		class InputDevice : public Singleton<InputDevice>
		{
			friend Singleton<InputDevice>;
		private:
			InputDevice();
			virtual ~InputDevice();

		public:
			bool Init(HINSTANCE hInstance, HWND hWnd, DWORD keyboardCoopFlag = eNoneExclusive | eForeGround, DWORD mouseCoopFlag = eNoneExclusive | eForeGround);
			void Release();

			void Update(float fElapsedTime);

			bool HandleMsg(HWND hWnd, uint32_t nMsg, WPARAM wParam, LPARAM lParam);

		public:
			bool IsMouseEvent(EmMouse::Button emMouseButton) { return m_pMouse->IsMouseEvent(emMouseButton); }
			bool IsMouseDown(EmMouse::Button emMouseButton) { return m_pMouse->IsMouseDown(emMouseButton); }
			bool IsMousePress(EmMouse::Button emMouseButton) { return m_pMouse->IsMousePress(emMouseButton); }
			bool IsMouseUp(EmMouse::Button emMouseButton) { return m_pMouse->IsMouseUp(emMouseButton); }

			long GetMoveX() { return m_pMouse->GetMoveX(); }
			long GetMoveY() { return m_pMouse->GetMoveY(); }
			long GetMoveWheel() { return m_pMouse->GetMoveWheel(); }

			bool IsKeyEvent(EmKeyboard::Button emKeyButton) { return m_pKeyboard->IsKeyEvent(emKeyButton); }
			bool IsKeyDown(EmKeyboard::Button emKeyButton) { return m_pKeyboard->IsKeyDown(emKeyButton); }
			bool IsKeyPress(EmKeyboard::Button emKeyButton) { return m_pKeyboard->IsKeyPress(emKeyButton); }
			bool IsKeyUp(EmKeyboard::Button emKeyButton) { return m_pKeyboard->IsKeyUp(emKeyButton); }

			Mouse* GetMouse() { return m_pMouse; }
			Keyboard* GetKeyboard() { return m_pKeyboard; }
			GamePad* GetGamePad() { return m_pGamePad; }

		private:
			IDirectInput8A* m_pInput;	// ¥Ÿ¿Ã∑∫∆Æ ¿Œ«≤ ∞¥√º

			Mouse* m_pMouse;
			Keyboard* m_pKeyboard;
			GamePad* m_pGamePad;

			bool m_isInit;
			bool m_isFocus;
		};
	}
}