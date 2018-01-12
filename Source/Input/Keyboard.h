#pragma once

#include "InputInterface.h"

namespace EastEngine
{
	namespace Input
	{
		class KeyboardInstance
		{
		public:
			KeyboardInstance();
			~KeyboardInstance();

		public:
			bool Init(HWND hWnd, struct IDirectInput8A* pInput, DWORD keyboardCoopFlag);
			void Release();
			void Update();

		public:
			bool IsKeyEvent(Keyboard::KeyCode emKeyCode) { return IsCurKeyDown(emKeyCode); }
			bool IsKeyDown(Keyboard::KeyCode emKeyCode) { return IsOldKeyDown(emKeyCode) == false && IsCurKeyDown(emKeyCode); }
			bool IsKeyPressed(Keyboard::KeyCode emKeyCode) { return IsOldKeyDown(emKeyCode) && IsCurKeyDown(emKeyCode); }
			bool IsKeyUp(Keyboard::KeyCode emKeyCode) { return IsOldKeyDown(emKeyCode) && IsCurKeyDown(emKeyCode) == false; }

		private:
			bool IsCurKeyDown(Keyboard::KeyCode emKeyCode) { return (m_curKeyState[emKeyCode] & 0x80) != 0; }
			bool IsOldKeyDown(Keyboard::KeyCode emKeyCode) { return (m_oldKeyState[emKeyCode] & 0x80) != 0; }

		private:
			struct IDirectInputDevice8A* m_pKeyboard;		// Ű���� ����̽�
			std::array<byte, 256> m_oldKeyState;	// �� Ű���� ���� ����
			std::array<byte, 256> m_curKeyState;	// �� Ű���� ���� ����
			byte* m_pCurKeyState;
		};
	}
}