#pragma once

#include "CommonLib/Singleton.h"

#include "InputInterface.h"

namespace est
{
	namespace input
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
			void Initialize(HINSTANCE hInstance, HWND hWnd, DWORD keyboardCoopFlag = eNoneExclusive | eForeGround, DWORD mouseCoopFlag = eNoneExclusive | eForeGround);

		public:
			void Update(float elapsedTime);

			bool HandleMessage(HWND hWnd, uint32_t nMsg, WPARAM wParam, LPARAM lParam);

		public:
			MouseInstance* GetMouse();
			KeyboardInstance* GetKeyboard();
			GamePadInstance* GetGamePad();

		private:
			class Impl;
			std::unique_ptr<Impl> m_pImpl;
		};
	}
}