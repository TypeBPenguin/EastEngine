#include "stdafx.h"
#include "InputDevice.h"

#include "Mouse.h"
#include "Keyboard.h"
#include "GamePad.h"

namespace EastEngine
{
	namespace Input
	{
		static_assert(eExclusive == DISCL_EXCLUSIVE, "dinput.h mismatch");
		static_assert(eNoneExclusive == DISCL_NONEXCLUSIVE, "dinput.h mismatch");
		static_assert(eForeGround == DISCL_FOREGROUND, "dinput.h mismatch");
		static_assert(eBackGround == DISCL_BACKGROUND, "dinput.h mismatch");
		static_assert(eNoWinKey == DISCL_NOWINKEY, "dinput.h mismatch");

		class Device::Impl
		{
		public:
			Impl();
			~Impl();

		public:
			bool Initialize(HINSTANCE hInstance, HWND hWnd, DWORD keyboardCoopFlag = eNoneExclusive | eForeGround, DWORD mouseCoopFlag = eNoneExclusive | eForeGround);

		public:
			void Update(float fElapsedTime);

			bool HandleMessage(HWND hWnd, uint32_t nMsg, WPARAM wParam, LPARAM lParam);

		public:
			MouseInstance* GetMouse() { return &m_mouse; }
			KeyboardInstance* GetKeyboard() { return &m_keyboard; }
			GamePadInstance* GetGamePad() { return &m_gamePad; }

		private:
			IDirectInput8A * m_pInput{ nullptr };

			MouseInstance m_mouse;
			KeyboardInstance m_keyboard;
			GamePadInstance m_gamePad;

			bool m_isInitialized{ false };
			bool m_isFocus{ false };
		};

		Device::Impl::Impl()
		{
		}

		Device::Impl::~Impl()
		{
			SafeRelease(m_pInput);
		}

		bool Device::Impl::Initialize(HINSTANCE hInstance, HWND hWnd, DWORD keyboardCoopFlag, DWORD mouseCoopFlag)
		{
			if (m_isInitialized == true)
				return true;

			HRESULT hr = DirectInput8Create(hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, reinterpret_cast<void**>(&m_pInput), 0);
			if (FAILED(hr))
				return false;

			if (m_pInput == nullptr)
				return false;

			if (m_mouse.Init(hWnd, m_pInput, mouseCoopFlag) == false)
				return false;

			if (m_keyboard.Init(hWnd, m_pInput, keyboardCoopFlag) == false)
				return false;

			m_isInitialized = true;

			return true;
		}

		void Device::Impl::Update(float fElapsedTime)
		{
			m_mouse.Update();
			m_keyboard.Update();
			m_gamePad.Update(fElapsedTime);
		}

		bool Device::Impl::HandleMessage(HWND hWnd, uint32_t nMsg, WPARAM wParam, LPARAM lParam)
		{
			switch (nMsg)
			{
			case WM_ACTIVATEAPP:
				if (wParam)
				{
					m_isFocus = true;
				}
				else
				{
					m_isFocus = false;
				}
				break;
			case WM_MOUSEMOVE:
				m_mouse.HandleMouse(hWnd, nMsg, wParam, lParam);
				break;
			}

			return true;
		}

		Device::Device()
			: m_pImpl{ std::make_unique<Impl>() }
		{
		}

		Device::~Device()
		{
		}

		bool Device::Initialize(HINSTANCE hInstance, HWND hWnd, DWORD keyboardCoopFlag, DWORD mouseCoopFlag)
		{
			return m_pImpl->Initialize(hInstance, hWnd, keyboardCoopFlag, mouseCoopFlag);
		}

		void Device::Update(float fElapsedTime)
		{
			m_pImpl->Update(fElapsedTime);
		}

		bool Device::HandleMessage(HWND hWnd, uint32_t nMsg, WPARAM wParam, LPARAM lParam)
		{
			return m_pImpl->HandleMessage(hWnd, nMsg, wParam, lParam);
		}

		MouseInstance* Device::GetMouse()
		{
			return m_pImpl->GetMouse();
		}

		KeyboardInstance* Device::GetKeyboard()
		{
			return m_pImpl->GetKeyboard();
		}

		GamePadInstance* Device::GetGamePad()
		{
			return m_pImpl->GetGamePad();
		}
	}
}