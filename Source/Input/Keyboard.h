#pragma once

#include "InputInterface.h"

namespace eastengine
{
	namespace input
	{
		class KeyboardInstance
		{
		public:
			KeyboardInstance();
			~KeyboardInstance();

		public:
			void Initialize(HWND hWnd, struct IDirectInput8A* pInput, DWORD keyboardCoopFlag);
			void Release();
			void Update();

		public:
			bool IsKeyEvent(Keyboard::KeyCode emKeyCode) const { return IsCurKeyDown(emKeyCode); }
			bool IsKeyDown(Keyboard::KeyCode emKeyCode) const { return IsOldKeyDown(emKeyCode) == false && IsCurKeyDown(emKeyCode); }
			bool IsKeyPressed(Keyboard::KeyCode emKeyCode) const { return IsOldKeyDown(emKeyCode) && IsCurKeyDown(emKeyCode); }
			bool IsKeyUp(Keyboard::KeyCode emKeyCode) const { return IsOldKeyDown(emKeyCode) && IsCurKeyDown(emKeyCode) == false; }

		private:
			bool IsCurKeyDown(Keyboard::KeyCode emKeyCode) const { return (m_curKeyState[emKeyCode] & 0x80) != 0; }
			bool IsOldKeyDown(Keyboard::KeyCode emKeyCode) const { return (m_oldKeyState[emKeyCode] & 0x80) != 0; }

		private:
			struct IDirectInputDevice8A* m_pKeyboard;		// 키보드 디바이스
			std::array<byte, 256> m_oldKeyState;	// 각 키들의 이전 상태
			std::array<byte, 256> m_curKeyState;	// 각 키들의 현재 상태
			byte* m_pCurKeyState;
		};
	}
}