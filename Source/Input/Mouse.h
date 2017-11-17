#pragma once

#include "../CommonLib/Singleton.h"

struct IDirectInput8A;
struct IDirectInputDevice8A;

namespace EastEngine
{
	namespace Input
	{
		namespace EmMouse
		{
			enum Button
			{
				eLeft = 0,
				eRight,
				eMiddle,
				eUndefine1,
				eUndefine2,
				eUndefine3,
				eUndefine4,
				eUndefine5,

				eCount,
			};
		}

		struct MouseState
		{
			/*int x;
			int y;*/

			long moveX;
			long moveY;
			long moveWheel;

			union
			{
				struct _rgbButton
				{
					byte leftButton;
					byte rightButton;
					byte middleButton;
					byte undefineButton[5];
				};

				_rgbButton rgbButton;
				byte rgbButtonArr[8];
			};
		};

		class Mouse : public Singleton<Mouse>
		{
			friend Singleton<Mouse>;
		private:
			Mouse();
			virtual ~Mouse();

		public:
			bool Init(HWND hWnd, IDirectInput8A* pInput, DWORD mouseCoopFlag);
			void Release();

			void Update();

			void HandleMouse(HWND hWnd, uint32_t nMsg, WPARAM wParam, LPARAM lParam);

		public:
			bool IsMouseEvent(EmMouse::Button emMouseButton) { return CurMouseDown(emMouseButton); }
			bool IsMouseDown(EmMouse::Button emMouseButton) { return !OldMouseDown(emMouseButton) && CurMouseDown(emMouseButton); }
			bool IsMousePress(EmMouse::Button emMouseButton) { return OldMouseDown(emMouseButton) && CurMouseDown(emMouseButton); }
			bool IsMouseUp(EmMouse::Button emMouseButton) { return OldMouseDown(emMouseButton) && !CurMouseDown(emMouseButton); }

			int	GetPosX() { return m_nX; }
			int GetPosY() { return m_nY; }

			long GetMoveX() { return m_CurMouseState.moveX; }				// 마우스 X 이동거리
			long GetMoveY() { return m_CurMouseState.moveY; }				// 마우스 Y 이동거리
			long GetMoveWheel() { return m_CurMouseState.moveWheel; }	// 휠 이동거리

		private:
			bool CurMouseDown(EmMouse::Button emMouseButton) { return (m_CurMouseState.rgbButtonArr[static_cast<int>(emMouseButton)] & 0x80) != 0; }
			bool OldMouseDown(EmMouse::Button emMouseButton) { return (m_OldMouseState.rgbButtonArr[static_cast<int>(emMouseButton)] & 0x80) != 0; }

		private:
			IDirectInputDevice8A* m_pMouse;	// 마우스 디바이스
			MouseState m_CurMouseState;
			MouseState m_OldMouseState;

			int m_nX;
			int m_nY;
		};
	}
}