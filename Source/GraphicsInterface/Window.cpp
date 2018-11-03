#include "stdafx.h"
#include "Window.h"

namespace eastengine
{
	namespace graphics
	{
		struct WndMsg
		{
			HWND hWnd{ nullptr };
			uint32_t msg{ 0 };
			WPARAM wParam{ 0 };
			LPARAM lParam{ 0 };
		};
		Concurrency::concurrent_queue<WndMsg> s_queueMsg;

		LRESULT CALLBACK WndProc(HWND hWnd, uint32_t msg, WPARAM wParam, LPARAM lParam)
		{
			switch (msg)
			{
			case WM_SYSCOMMAND:
				// Disable ALT application menu
				if ((wParam & 0xfff0) == SC_KEYMENU)
					return 0;

				break;
			case WM_DESTROY:
				PostQuitMessage(0);
				return 0;
			}

			s_queueMsg.push({ hWnd, msg, wParam, lParam });

			return DefWindowProc(hWnd, msg, wParam, lParam);
		}

		Window::Window()
		{
		}

		Window::~Window()
		{
		}

		void Window::AddMessageHandler(const string::StringID& strName, std::function<void(HWND, uint32_t, WPARAM, LPARAM)> funcHandler)
		{
			RemoveMessageHandler(strName);

			m_umapHandlers.emplace(strName, funcHandler);
		}

		void Window::RemoveMessageHandler(const string::StringID& strName)
		{
			m_umapHandlers.erase(strName);
		}

		void Window::Run(std::function<void()> funcUpdate)
		{
			MSG msg{};

			while (true)
			{
				if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
				{
					if (msg.message == WM_QUIT)
						break;

					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
				else
				{
					while (s_queueMsg.empty() == false)
					{
						WndMsg wndMsg{};
						if (s_queueMsg.try_pop(wndMsg) == true)
						{
							for (auto& iter : m_umapHandlers)
							{
								iter.second(wndMsg.hWnd, wndMsg.msg, wndMsg.wParam, wndMsg.lParam);
							}
						}
					}

					Update();

					funcUpdate();

					Render();

					Present();
				}
			}
		}

		void Window::InitializeWindow(uint32_t nWidth, uint32_t nHeight, bool isFullScreen, const string::StringID& strApplicationTitle, const string::StringID& strApplicationName)
		{
			m_hInstance = GetModuleHandle(NULL);
			m_n2ScreenSize = { nWidth, nHeight };
			m_isFullScreen = isFullScreen;
			m_strApplicationTitle = strApplicationTitle;
			m_strApplicationName = strApplicationName;

			math::Rect rtWindowSize{};
			if (m_isFullScreen == true)
			{
				HMONITOR hMonitor = MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTONEAREST);
				MONITORINFO monitorInfo{};
				GetMonitorInfo(hMonitor, &monitorInfo);

				rtWindowSize.right = m_n2ScreenSize.x = monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left;
				rtWindowSize.bottom = m_n2ScreenSize.y = monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top;
			}
			else
			{
				int nPosX = static_cast<int>((GetSystemMetrics(SM_CXSCREEN) - m_n2ScreenSize.x) * 0.5f);
				int nPosY = static_cast<int>((GetSystemMetrics(SM_CYSCREEN) - m_n2ScreenSize.y) * 0.5f);

				rtWindowSize.right = m_n2ScreenSize.x;
				rtWindowSize.bottom = m_n2ScreenSize.y;
				::AdjustWindowRect(&rtWindowSize, WS_OVERLAPPEDWINDOW | WS_VISIBLE, false);
				OffsetRect(&rtWindowSize, nPosX, nPosY);
			}

			WNDCLASSEX wc{};
			wc.style = CS_HREDRAW | CS_VREDRAW;
			wc.lpfnWndProc = WndProc;
			wc.cbClsExtra = 0;
			wc.cbWndExtra = 0;
			wc.hInstance = m_hInstance;
			wc.hIcon = LoadIcon(nullptr, IDI_WINLOGO);
			wc.hIconSm = wc.hIcon;
			wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
			wc.hbrBackground = static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
			wc.lpszMenuName = nullptr;
			wc.lpszClassName = m_strApplicationName.c_str();
			wc.cbSize = sizeof(WNDCLASSEX);

			if (RegisterClassEx(&wc) == false)
			{
				throw_line("failed to register class");
			}

			m_hWnd = CreateWindowEx(WS_EX_APPWINDOW,
				m_strApplicationName.c_str(),
				m_strApplicationTitle.c_str(),
				WS_OVERLAPPEDWINDOW | WS_VISIBLE,
				rtWindowSize.left, rtWindowSize.top,
				rtWindowSize.GetWidth(), rtWindowSize.GetHeight(),
				nullptr, nullptr,
				m_hInstance, nullptr);

			if (m_hWnd == nullptr)
			{
				throw_line("failed to create window");
			}

			if (m_isFullScreen == true)
			{
				SetWindowLong(m_hWnd, GWL_STYLE, 0);
			}

			ShowWindow(m_hWnd, SW_SHOW);
			UpdateWindow(m_hWnd);
		}
	}
}