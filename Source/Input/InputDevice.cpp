#include "stdafx.h"
#include "Mouse.h"
#include "Keyboard.h"
#include "InputDevice.h"

namespace EastEngine
{
	namespace Input
	{
		Device::Device()
			: m_pInput(nullptr)
			, m_pMouse(nullptr)
			, m_pKeyboard(nullptr)
			, m_pGamePad(nullptr)
			, m_isInit(false)
			, m_isFocus(false)
		{
		}

		Device::~Device()
		{
			Release();
		}

		bool Device::Init(HINSTANCE hInstance, HWND hWnd, DWORD keyboardCoopFlag, DWORD mouseCoopFlag)
		{
			if (m_isInit == true)
				return true;

			m_isInit = true;

			HRESULT hr = DirectInput8Create(hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, reinterpret_cast<void**>(&m_pInput), 0);
			if (FAILED(hr))
				return false;

			if (m_pInput == nullptr)
				return false;

			m_pMouse = Mouse::GetInstance();
			if (m_pMouse->Init(hWnd, m_pInput, mouseCoopFlag) == false)
			{
				Release();
				return false;
			}

			m_pKeyboard = Keyboard::GetInstance();
			if (m_pKeyboard->Init(hWnd, m_pInput, keyboardCoopFlag) == false)
			{
				Release();
				return false;
			}

			m_pGamePad = GamePad::GetInstance();

			return true;
		}

		void Device::Release()
		{
			SafeRelease(m_pMouse);
			Mouse::DestroyInstance();

			SafeRelease(m_pKeyboard);
			Keyboard::DestroyInstance();

			GamePad::DestroyInstance();

			SafeRelease(m_pInput);

			m_isInit = false;
		}

		void Device::Update(float fElapsedTime)
		{
			m_pMouse->Update();
			m_pKeyboard->Update();
			m_pGamePad->Update(fElapsedTime);
		}

		bool Device::HandleMsg(HWND hWnd, uint32_t nMsg, WPARAM wParam, LPARAM lParam)
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
				m_pMouse->HandleMouse(hWnd, nMsg, wParam, lParam);
				break;
			}

			return true;
		}
	}
}