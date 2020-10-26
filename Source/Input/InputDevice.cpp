#include "stdafx.h"
#include "InputDevice.h"

#include "Mouse.h"
#include "Keyboard.h"
#include "GamePad.h"

namespace est
{
	namespace input
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
			void Initialize(HINSTANCE hInstance, HWND hWnd, DWORD keyboardCoopFlag = eNoneExclusive | eForeGround, DWORD mouseCoopFlag = eNoneExclusive | eForeGround);

		public:
			void Update(float elapsedTime);

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
		};

		Device::Impl::Impl()
		{
		}

		Device::Impl::~Impl()
		{
			SafeRelease(m_pInput);
		}

		void Device::Impl::Initialize(HINSTANCE hInstance, HWND hWnd, DWORD keyboardCoopFlag, DWORD mouseCoopFlag)
		{
			if (m_isInitialized == true)
				return;

			HRESULT hr = DirectInput8Create(hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, reinterpret_cast<void**>(&m_pInput), 0);
			if (FAILED(hr))
			{
				throw_line("failed to create input device");
			}

			m_mouse.Initialize(hWnd, m_pInput, mouseCoopFlag);
			m_keyboard.Initialize(hWnd, m_pInput, keyboardCoopFlag);

			m_isInitialized = true;
		}

		void Device::Impl::Update(float elapsedTime)
		{
			TRACER_EVENT(L"InputDevice::Update");
			m_mouse.Update();
			m_keyboard.Update();
			m_gamePad.Update(elapsedTime);
		}

		bool Device::Impl::HandleMessage(HWND hWnd, uint32_t nMsg, WPARAM wParam, LPARAM lParam)
		{
			switch (nMsg)
			{
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

		void Device::Initialize(HINSTANCE hInstance, HWND hWnd, DWORD keyboardCoopFlag, DWORD mouseCoopFlag)
		{
			m_pImpl->Initialize(hInstance, hWnd, keyboardCoopFlag, mouseCoopFlag);
		}

		void Device::Update(float elapsedTime)
		{
			m_pImpl->Update(elapsedTime);
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