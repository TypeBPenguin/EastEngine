#pragma once

#include "CommonLib/Singleton.h"

#include "InputInterface.h"

namespace EastEngine
{
	namespace Input
	{
		class MouseInstance;
		class KeyboardInstance;
		class GamePadInstance;

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
			MouseInstance* GetMouse() const { return m_pMouse; }
			KeyboardInstance* GetKeyboard() const { return m_pKeyboard; }
			GamePadInstance* GetGamePad() const { return m_pGamePad; }

		private:
			struct IDirectInput8A* m_pInput;	// ¥Ÿ¿Ã∑∫∆Æ ¿Œ«≤ ∞¥√º

			MouseInstance* m_pMouse;
			KeyboardInstance* m_pKeyboard;
			GamePadInstance* m_pGamePad;

			bool m_isInit;
			bool m_isFocus;
		};
	}
}