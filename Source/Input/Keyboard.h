#pragma once

#include "CommonLib/Singleton.h"

struct IDirectInput8A;
struct IDirectInputDevice8A;

namespace EastEngine
{
	namespace Input
	{
		class Keyboard : public Singleton<Keyboard>
		{
			friend Singleton<Keyboard>;
		private:
			Keyboard();
			virtual ~Keyboard();

		public:
#include "KeyState.inl"

		public:
			bool Init(HWND hWnd, IDirectInput8A* pInput, DWORD keyboardCoopFlag);
			void Release();
			void Update();

		public:
			bool IsKeyEvent(Button emKeyButton) { return IsCurKeyDown(emKeyButton); }
			bool IsKeyDown(Button emKeyButton) { return IsOldKeyDown(emKeyButton) == false && IsCurKeyDown(emKeyButton); }
			bool IsKeyPress(Button emKeyButton) { return IsOldKeyDown(emKeyButton) && IsCurKeyDown(emKeyButton); }
			bool IsKeyUp(Button emKeyButton) { return IsOldKeyDown(emKeyButton) && IsCurKeyDown(emKeyButton) == false; }

		private:
			bool IsCurKeyDown(Button emKeyButton) { return (m_curKeyState[emKeyButton] & 0x80) != 0; }
			bool IsOldKeyDown(Button emKeyButton) { return (m_oldKeyState[emKeyButton] & 0x80) != 0; }

		private:
			IDirectInputDevice8A* m_pKeyboard;		// Ű���� ����̽�
			std::array<byte, 256> m_oldKeyState;	// �� Ű���� ���� ����
			std::array<byte, 256> m_curKeyState;	// �� Ű���� ���� ����
			byte* m_pCurKeyState;
		};
	}
}