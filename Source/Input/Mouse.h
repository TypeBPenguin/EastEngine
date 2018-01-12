#pragma once

#include "InputInterface.h"

namespace EastEngine
{
	namespace Input
	{
		class MouseInstance
		{
		public:
			MouseInstance();
			~MouseInstance();

		private:
			struct MouseState
			{
				long moveX = 0;
				long moveY = 0;
				long moveWheel = 0;

				union
				{
					struct
					{
						byte leftButton;
						byte rightButton;
						byte middleButton;
						byte undefineButton[5];
					};

					byte rgbButtonArr[8];
				};
			};

		public:
			bool Init(HWND hWnd, struct IDirectInput8A* pInput, DWORD mouseCoopFlag);
			void Release();

			void Update();

			void HandleMouse(HWND hWnd, uint32_t nMsg, WPARAM wParam, LPARAM lParam);

		public:
			bool IsButtonEvent(Mouse::Button emMouseButton) { return CurMouseDown(emMouseButton); }
			bool IsButtonDown(Mouse::Button emMouseButton) { return !OldMouseDown(emMouseButton) && CurMouseDown(emMouseButton); }
			bool IsButtonPressed(Mouse::Button emMouseButton) { return OldMouseDown(emMouseButton) && CurMouseDown(emMouseButton); }
			bool IsButtonUp(Mouse::Button emMouseButton) { return OldMouseDown(emMouseButton) && !CurMouseDown(emMouseButton); }

			int	GetX() { return m_nX; }
			int GetY() { return m_nY; }

			long GetMoveX() { return m_CurMouseState.moveX; }				// 마우스 X 이동거리
			long GetMoveY() { return m_CurMouseState.moveY; }				// 마우스 Y 이동거리
			long GetMoveWheel() { return m_CurMouseState.moveWheel; }	// 휠 이동거리

		private:
			bool CurMouseDown(Mouse::Button emMouseButton) { return (m_CurMouseState.rgbButtonArr[static_cast<int>(emMouseButton)] & 0x80) != 0; }
			bool OldMouseDown(Mouse::Button emMouseButton) { return (m_OldMouseState.rgbButtonArr[static_cast<int>(emMouseButton)] & 0x80) != 0; }

		private:
			struct IDirectInputDevice8A* m_pMouse;	// 마우스 디바이스
			MouseState m_CurMouseState;
			MouseState m_OldMouseState;

			int m_nX;
			int m_nY;
		};
	}
}