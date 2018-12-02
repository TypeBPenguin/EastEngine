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
			bool IsKeyEvent(keyboard::KeyCode emKeyCode) const { return IsCurKeyDown(emKeyCode); }
			bool IsKeyDown(keyboard::KeyCode emKeyCode) const { return IsOldKeyDown(emKeyCode) == false && IsCurKeyDown(emKeyCode); }
			bool IsKeyPressed(keyboard::KeyCode emKeyCode) const { return IsOldKeyDown(emKeyCode) && IsCurKeyDown(emKeyCode); }
			bool IsKeyUp(keyboard::KeyCode emKeyCode) const { return IsOldKeyDown(emKeyCode) && IsCurKeyDown(emKeyCode) == false; }

		private:
			bool IsCurKeyDown(keyboard::KeyCode emKeyCode) const { return (m_curKeyState[emKeyCode] & 0x80) != 0; }
			bool IsOldKeyDown(keyboard::KeyCode emKeyCode) const { return (m_oldKeyState[emKeyCode] & 0x80) != 0; }

		private:
			struct IDirectInputDevice8A* m_pKeyboard;		// Ű���� ����̽�
			std::array<byte, 256> m_oldKeyState;	// �� Ű���� ���� ����
			std::array<byte, 256> m_curKeyState;	// �� Ű���� ���� ����
			byte* m_pCurKeyState;
		};
	}
}