#pragma once

#include "CommonLib/Singleton.h"

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

		class Device : public Singleton<Device>
		{
			friend Singleton<Device>;
		private:
			Device();
			virtual ~Device();

		public:
			bool Init(HINSTANCE hInstance, HWND hWnd, DWORD keyboardCoopFlag = eNoneExclusive | eForeGround, DWORD mouseCoopFlag = eNoneExclusive | eForeGround);
			void Release();

			void Update(float fElapsedTime);

			bool HandleMsg(HWND hWnd, uint32_t nMsg, WPARAM wParam, LPARAM lParam);

		public:
			bool IsMouseEvent(Mouse::Button emMouseButton) { return m_pMouse->IsButtonEvent(emMouseButton); }
			bool IsMouseDown(Mouse::Button emMouseButton) { return m_pMouse->IsButtonDown(emMouseButton); }
			bool IsMousePress(Mouse::Button emMouseButton) { return m_pMouse->IsButtonPress(emMouseButton); }
			bool IsMouseUp(Mouse::Button emMouseButton) { return m_pMouse->IsButtonUp(emMouseButton); }

			long GetMoveX() { return m_pMouse->GetMoveX(); }
			long GetMoveY() { return m_pMouse->GetMoveY(); }
			long GetMoveWheel() { return m_pMouse->GetMoveWheel(); }

			bool IsKeyEvent(Keyboard::Button emKeyButton) { return m_pKeyboard->IsKeyEvent(emKeyButton); }
			bool IsKeyDown(Keyboard::Button emKeyButton) { return m_pKeyboard->IsKeyDown(emKeyButton); }
			bool IsKeyPress(Keyboard::Button emKeyButton) { return m_pKeyboard->IsKeyPress(emKeyButton); }
			bool IsKeyUp(Keyboard::Button emKeyButton) { return m_pKeyboard->IsKeyUp(emKeyButton); }

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