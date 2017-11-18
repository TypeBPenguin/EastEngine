#pragma once

#include "CommonLib/Singleton.h"

#include "KeyState.h"

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
			bool Init(HWND hWnd, IDirectInput8A* pInput, DWORD keyboardCoopFlag);
			void Release();
			void Update();

		public:
			bool IsKeyEvent(EmKeyboard::Button emKeyButton) { return IsCurKeyDown(emKeyButton); }
			bool IsKeyDown(EmKeyboard::Button emKeyButton) { return IsOldKeyDown(emKeyButton) == false && IsCurKeyDown(emKeyButton); }
			bool IsKeyPress(EmKeyboard::Button emKeyButton) { return IsOldKeyDown(emKeyButton) && IsCurKeyDown(emKeyButton); }
			bool IsKeyUp(EmKeyboard::Button emKeyButton) { return IsOldKeyDown(emKeyButton) && IsCurKeyDown(emKeyButton) == false; }

		private:
			bool IsCurKeyDown(EmKeyboard::Button emKeyButton) { return (m_curKeyState[emKeyButton] & 0x80) != 0; }
			bool IsOldKeyDown(EmKeyboard::Button emKeyButton) { return (m_oldKeyState[emKeyButton] & 0x80) != 0; }

		private:
			IDirectInputDevice8A* m_pKeyboard;		// 키보드 디바이스
			std::array<byte, 256> m_oldKeyState;	// 각 키들의 이전 상태
			std::array<byte, 256> m_curKeyState;	// 각 키들의 현재 상태
			byte* m_pCurKeyState;
		};
	}
}