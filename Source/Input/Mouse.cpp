#include "stdafx.h"
#include "Mouse.h"

namespace eastengine
{
	namespace input
	{
		MouseInstance::MouseInstance()
			: m_pMouse(nullptr)
			, m_nX(0)
			, m_nY(0)
		{
			memory::Clear(&m_CurMouseState, sizeof(m_CurMouseState));
			memory::Clear(&m_OldMouseState, sizeof(m_OldMouseState));
		}

		MouseInstance::~MouseInstance()
		{
			Release();
		}

		void MouseInstance::Initialize(HWND hWnd, IDirectInput8A* pInput, DWORD mouseCoopFlag)
		{
			if (pInput == nullptr)
				return;

			// 마우스 디바이스 생성
			HRESULT hr = pInput->CreateDevice(GUID_SysMouse, &m_pMouse, 0);
			if (FAILED(hr))
			{
				throw_line("failed to create mouse device");
			}

			m_pMouse->SetDataFormat(&c_dfDIMouse2);
			m_pMouse->SetCooperativeLevel(hWnd, mouseCoopFlag);
			m_pMouse->Acquire();
		}

		void MouseInstance::Release()
		{
			if (m_pMouse != nullptr)
			{
				m_pMouse->Unacquire();
			}
			SafeRelease(m_pMouse);
		}

		void MouseInstance::Update()
		{
			TRACER_EVENT("MouseInstance::Update");
			memory::Copy(&m_OldMouseState, sizeof(m_OldMouseState), &m_CurMouseState, sizeof(m_CurMouseState));

			HRESULT hr = m_pMouse->GetDeviceState(sizeof(m_CurMouseState), reinterpret_cast<void**>(&m_CurMouseState));

			if (FAILED(hr))
			{
				if (hr == DIERR_INPUTLOST || hr == DIERR_NOTACQUIRED)
				{
					memory::Clear(&m_CurMouseState, sizeof(m_CurMouseState));
					m_pMouse->Acquire();
				}
			}
		}

		void MouseInstance::HandleMouse(HWND hWnd, uint32_t nMsg, WPARAM wParam, LPARAM lParam)
		{
			switch (nMsg)
			{
			case WM_MOUSEMOVE:
				m_nX = static_cast<short>(LOWORD(lParam)); // GET_X_LPARAM(lParam);
				m_nY = static_cast<short>(HIWORD(lParam)); // GET_Y_LPARAM(lParam);
				break;
			}
		}
	}
}