#include "stdafx.h"
#include "Windows.h"

namespace EastEngine
{
	namespace Windows
	{
		MessageData::MessageData(HWND _hWnd, uint32_t _nMsg, WPARAM _wParam, LPARAM _lParam)
			: hWnd(_hWnd)
			, nMsg(_nMsg)
			, wParam(_wParam)
			, lParam(_lParam)
		{
		}

		WindowsManager::WindowsManager()
			: m_hInstance(nullptr)
			, m_hWnd(nullptr)
			, m_nScreenWidth(0)
			, m_nScreenHeight(0)
			, m_isFullScreen(false)
			, m_isInit(false)
		{
		}

		WindowsManager::~WindowsManager()
		{
			Release();
		}

		bool WindowsManager::Init(const char* strApplicationName, uint32_t nScreenWidth, uint32_t nScreenHeight, bool isFullScreen)
		{
			if (m_isInit == true)
				return true;

			m_isFullScreen = isFullScreen;

			m_nScreenWidth = nScreenWidth;
			m_nScreenHeight = nScreenHeight;

			m_strApplicationName = strApplicationName;

			DEVMODE dmScreenSettings;
			int posX, posY;

			// 이 어플리케이션의 인스턴스를 가져옵니다.
			m_hInstance = GetModuleHandle(nullptr);

			// 어플리케이션의 이름을 설정
			m_strApplicationName = strApplicationName;

			// 윈도우 클래스를 기본 설정으로 맞춤
			WNDCLASSEX wc;
			wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
			wc.lpfnWndProc = WndProc;
			wc.cbClsExtra = 0;
			wc.cbWndExtra = 0;
			wc.hInstance = m_hInstance;
			wc.hIcon = LoadIcon(nullptr, IDI_WINLOGO);
			wc.hIconSm = wc.hIcon;
			wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
			wc.hbrBackground = static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
			wc.lpszMenuName = nullptr;
			wc.lpszClassName = strApplicationName;
			wc.cbSize = sizeof(WNDCLASSEX);

			// 윈도우 클래스를 등록
			RegisterClassEx(&wc);

			// 풀스크린 모드 변수의 값에 따라 화면 설정을 한다.
			if (m_isFullScreen)
			{
				// 만약 풀스크린 모드라면 화면 크기를 데스크톱 크기에 맞추고 색상을 32bit로 합니다.
				memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
				dmScreenSettings.dmSize = sizeof(dmScreenSettings);
				dmScreenSettings.dmPelsWidth = GetSystemMetrics(SM_CXSCREEN);
				dmScreenSettings.dmPelsHeight = GetSystemMetrics(SM_CYSCREEN);
				dmScreenSettings.dmBitsPerPel = 32;
				dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

				// 풀스크린에 맞는 디스플레이 설정을 합니다.
				ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);

				// 윈도우의 위치를 화면의 왼쪽 위로 맞춥니다.
				posX = posY = 0;
			}
			else
			{
				// 창을 모니터의 중앙에 오도록 합니다.
				posX = static_cast<int>((GetSystemMetrics(SM_CXSCREEN) - nScreenWidth) * 0.5f);
				posY = static_cast<int>((GetSystemMetrics(SM_CYSCREEN) - nScreenHeight) * 0.5f);
			}

			Math::Rect rtWindowSize;
			rtWindowSize.right = nScreenWidth;
			rtWindowSize.bottom = nScreenHeight;
			::AdjustWindowRect(&rtWindowSize, WS_OVERLAPPEDWINDOW | WS_VISIBLE, false);

			// 설정을 가지고 창을 만들고 핸들을 가져온다.
			m_hWnd = CreateWindowEx(WS_EX_APPWINDOW, strApplicationName, strApplicationName,
				//WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP,
				WS_OVERLAPPEDWINDOW | WS_VISIBLE,
				posX, posY, rtWindowSize.GetWidth(), rtWindowSize.GetHeight(), nullptr, nullptr, m_hInstance, nullptr);

			if (m_hWnd == nullptr)
				return false;

			// 윈도우를 화면에 표시
			ShowWindow(m_hWnd, SW_SHOW);
			UpdateWindow(m_hWnd);

			m_isInit = true;

			return true;
		}

		void WindowsManager::Release()
		{
			if (m_isInit == false)
				return;

			// 풀스크린 모드를 빠져나올 때 디스플레이 설정 바꿈
			if (m_isFullScreen)
			{
				ChangeDisplaySettings(NULL, 0);
			}

			// 창을 제거
			DestroyWindow(m_hWnd);

			// 어플리케이션 인스턴스를 제거
			UnregisterClass(m_strApplicationName.c_str(), m_hInstance);

			m_hInstance = nullptr;
			m_hWnd = nullptr;

			m_vecFuncMessageHandler.clear();

			m_isInit = true;
		}

		void WindowsManager::ProcessMessages()
		{
			if (m_vecFuncMessageHandler.empty() || m_queueMessageData.empty())
				return;

			uint32_t nSize = m_vecFuncMessageHandler.size();

			while (m_queueMessageData.empty() == false)
			{
				MessageData& messageData = m_queueMessageData.front();

				for (uint32_t i = 0; i < nSize; ++i)
				{
					m_vecFuncMessageHandler[i](messageData.hWnd, messageData.nMsg, messageData.wParam, messageData.lParam);
				}

				m_queueMessageData.pop();
			}
		}

		bool WindowsManager::AddMessageHandler(FuncMessageHandler funcMessageHandler)
		{
			if (funcMessageHandler == nullptr)
				return false;

			m_vecFuncMessageHandler.push_back(funcMessageHandler);

			return true;
		}

		LRESULT WindowsManager::HandleMsg(HWND hWnd, uint32_t nMsg, WPARAM wParam, LPARAM lParam)
		{
			static bool isImeSetting = false;
			if (nMsg == WM_IME_SETCONTEXT && isImeSetting == false)
			{
				lParam &= ~(ISC_SHOWUICOMPOSITIONWINDOW | ISC_SHOWUIALLCANDIDATEWINDOW);
				lParam = 0;

				isImeSetting = true;

				return ImmIsUIMessage(hWnd, nMsg, wParam, lParam);
			}

			m_queueMessageData.emplace(hWnd, nMsg, wParam, lParam);

			return DefWindowProc(hWnd, nMsg, wParam, lParam);
		}

		LRESULT CALLBACK WndProc(HWND hwnd, uint32_t umessage, WPARAM wparam, LPARAM lparam)
		{
			switch (umessage)
			{
				// 윈도우가 제거되었는지 확인합니다.
			case WM_DESTROY:
			{
				PostQuitMessage(0);
				return 0;
			}

			// 윈도우가 닫히는지 확인합니다.
			case WM_CLOSE:
			{
				PostQuitMessage(0);
				return 0;
			}

			// 다른 모든 메세지들은 system 클래스의 메세지 처리기에 전달합니다.
			default:
			{
				return WindowsManager::GetInstance()->HandleMsg(hwnd, umessage, wparam, lparam);
			}
			}
		}
	}
}