#include "stdafx.h"
#include "Keyboard.h"

namespace EastEngine
{
	namespace Input
	{
		Keyboard::Keyboard()
			: m_pKeyboard(nullptr)
		{
		}

		Keyboard::~Keyboard()
		{
		}

		bool Keyboard::Init(HWND hWnd, IDirectInput8A* pInput, DWORD keyboardCoopFlag)
		{
			if (pInput == nullptr)
				return false;

			// 키보드 디바이스 생성
			pInput->CreateDevice(GUID_SysKeyboard, &m_pKeyboard, 0);
			m_pKeyboard->SetDataFormat(&c_dfDIKeyboard);
			m_pKeyboard->SetCooperativeLevel(hWnd, keyboardCoopFlag);
			m_pKeyboard->Acquire();

			return true;
		}

		void Keyboard::Release()
		{
			if (m_pKeyboard != nullptr)
			{
				m_pKeyboard->Unacquire();
			}
			SafeRelease(m_pKeyboard);
		}

		void Keyboard::Update()
		{
			Memory::Copy(&m_oldKeyState, sizeof(m_oldKeyState), &m_curKeyState, sizeof(m_curKeyState));

			HRESULT hr = m_pKeyboard->GetDeviceState(sizeof(m_curKeyState), reinterpret_cast<void**>(&m_curKeyState));

			if (FAILED(hr))
			{
				if (hr == DIERR_INPUTLOST || hr == DIERR_NOTACQUIRED)
				{
					Memory::Clear(&m_curKeyState, sizeof(m_curKeyState));
					m_pKeyboard->Acquire();
				}
			}
		}
	}
}