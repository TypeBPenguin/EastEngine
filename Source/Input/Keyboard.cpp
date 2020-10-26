#include "stdafx.h"
#include "Keyboard.h"

namespace est
{
	namespace input
	{
		KeyboardInstance::KeyboardInstance()
		{
		}

		KeyboardInstance::~KeyboardInstance()
		{
		}

		void KeyboardInstance::Initialize(HWND hWnd, IDirectInput8A* pInput, DWORD keyboardCoopFlag)
		{
			if (pInput == nullptr)
				return;

			// 키보드 디바이스 생성
			HRESULT hr = pInput->CreateDevice(GUID_SysKeyboard, &m_pKeyboard, 0);
			if (FAILED(hr))
			{
				throw_line("failed to create keyboard device");
			}

			m_pKeyboard->SetDataFormat(&c_dfDIKeyboard);
			m_pKeyboard->SetCooperativeLevel(hWnd, keyboardCoopFlag);
			m_pKeyboard->Acquire();
		}

		void KeyboardInstance::Release()
		{
			if (m_pKeyboard != nullptr)
			{
				m_pKeyboard->Unacquire();
			}
			SafeRelease(m_pKeyboard);
		}

		void KeyboardInstance::Update()
		{
			TRACER_EVENT(L"KeyboardInstance::Update");
			memory::Copy(&m_oldKeyState, sizeof(m_oldKeyState), &m_curKeyState, sizeof(m_curKeyState));

			HRESULT hr = m_pKeyboard->GetDeviceState(sizeof(m_curKeyState), reinterpret_cast<void**>(&m_curKeyState));

			if (FAILED(hr))
			{
				if (hr == DIERR_INPUTLOST || hr == DIERR_NOTACQUIRED)
				{
					m_curKeyState.fill(0);
					m_pKeyboard->Acquire();
				}
			}
		}
	}
}