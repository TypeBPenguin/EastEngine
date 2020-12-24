#include "stdafx.h"
#include "Window.h"

#include "WindowCursor.h"

namespace est
{
	namespace graphics
	{
		std::function<HRESULT(HWND, uint32_t, WPARAM, LPARAM)> s_messageHandler;

		LRESULT CALLBACK WndProc(HWND hWnd, uint32_t msg, WPARAM wParam, LPARAM lParam)
		{
			if (s_messageHandler != nullptr)
			{
				if (s_messageHandler(hWnd, msg, wParam, lParam))
					return 0;
			}

			switch (msg)
			{
			case WM_SYSCOMMAND:
			{
				// Disable ALT application menu
				if ((wParam & 0xfff0) == SC_KEYMENU)
					return 0;
			}
			break;
			case WM_DESTROY:
				PostQuitMessage(0);
				return 0;
			case WM_IME_SETCONTEXT:
			{
				// lParam = 0 으로 설정하고 DefWindowProc에 돌려준다.
				// 이유는 lParam이 0이면 조합창, 후보창, 상태창등을 표시 하지 않도록 하기 위함이다.
				lParam = 0;
				return DefWindowProc(hWnd, msg, wParam, lParam); // 반드시 돌려줘야한다.
			}
			break;
			}

			return DefWindowProc(hWnd, msg, wParam, lParam);
		}

		Window::Window()
		{
		}

		Window::~Window()
		{
			s_messageHandler = nullptr;
		}

		void Window::Run(std::function<bool()> funcUpdate)
		{
			MSG msg{};

			while (true)
			{
				TRACER_EVENT(L"MainLoop");
				{
					TRACER_EVENT(L"WinMessage");
					while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
					{
						if (msg.message == WM_QUIT)
							return;

						TranslateMessage(&msg);
						DispatchMessage(&msg);
					}
				}

				TRACER_EVENT(L"MainUpdate");
				const bool isRunning = funcUpdate();
				if (isRunning == false)
					return;
			}
		}

		void Window::Initialize(uint32_t width, uint32_t height, bool isFullScreen, const string::StringID& applicationTitle, const string::StringID& applicationName, std::function<HRESULT(HWND, uint32_t, WPARAM, LPARAM)> messageHandler)
		{
			if (m_hWnd != nullptr)
				return;

			s_messageHandler = messageHandler;

			m_hInstance = GetModuleHandle(NULL);
			m_screenSize = { width, height };
			m_isFullScreen = isFullScreen;
			m_applicationTitle = applicationTitle;
			m_applicationName = applicationName;

			DWORD style = 0;

			math::Rect rtWindowSize{};
			if (isFullScreen)
			{
				const int screenWidth = GetSystemMetrics(SM_CXSCREEN);
				const int screenHeight = GetSystemMetrics(SM_CYSCREEN);

				rtWindowSize.right = m_screenSize.x = screenWidth;
				rtWindowSize.bottom = m_screenSize.y = screenHeight;
				style = WS_POPUP | WS_CLIPCHILDREN;
			}
			else
			{
				const int posX = static_cast<int>((GetSystemMetrics(SM_CXSCREEN) - m_screenSize.x) * 0.5f);
				const int posY = static_cast<int>((GetSystemMetrics(SM_CYSCREEN) - m_screenSize.y) * 0.5f);

				rtWindowSize.right = m_screenSize.x;
				rtWindowSize.bottom = m_screenSize.y;
				::AdjustWindowRect(&rtWindowSize, WS_OVERLAPPEDWINDOW | WS_VISIBLE, false);
				OffsetRect(&rtWindowSize, posX, posY);
				style = WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX | WS_CLIPCHILDREN;
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
			wc.lpszClassName = m_applicationName.c_str();
			wc.cbSize = sizeof(WNDCLASSEX);

			if (RegisterClassEx(&wc) == false)
			{
				throw_line("failed register window class");
			}

			m_hWnd = CreateWindowEx(WS_EX_APPWINDOW,
				m_applicationName.c_str(),
				m_applicationTitle.c_str(),
				style,
				rtWindowSize.left, rtWindowSize.top,
				rtWindowSize.GetWidth(), rtWindowSize.GetHeight(),
				nullptr, nullptr,
				m_hInstance, nullptr);

			if (m_hWnd == nullptr)
			{
				throw_line("failed create window");
			}

			ShowWindow(m_hWnd, SW_SHOW);
			UpdateWindow(m_hWnd);

			s_messageHandler = messageHandler;

			m_pWindowCursor = std::make_unique<WindowCursor>();
		}

		bool Window::Resize(uint32_t width, uint32_t height, bool isFullScreen)
		{
			math::Rect rect = { 0, 0, static_cast<long>(width), static_cast<long>(height) };
			LONG_PTR style = GetWindowLongPtr(m_hWnd, GWL_STYLE);

			if (isFullScreen == true)
			{
				style &= ~(WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX | WS_CLIPCHILDREN);
				style |= WS_POPUP | WS_CLIPCHILDREN;
			}
			else
			{
				if (FAILED(AdjustWindowRectEx(&rect, WS_OVERLAPPEDWINDOW, false, WS_EX_OVERLAPPEDWINDOW)))
					return false;

				style &= ~(WS_POPUP | WS_CLIPCHILDREN);
				style |= WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX | WS_CLIPCHILDREN;
			}

			SetWindowLongPtr(m_hWnd, GWL_STYLE, style);

			SetWindowPos(m_hWnd, HWND_TOP, 0, 0, rect.GetWidth(), rect.GetHeight(), isFullScreen == true ? NULL : SWP_NOMOVE);
			ShowWindow(m_hWnd, SW_SHOW);

			m_screenSize = { static_cast<uint32_t>(rect.GetWidth()), static_cast<uint32_t>(rect.GetHeight()) };
			m_isFullScreen = isFullScreen;

			return true;
		}
	}
}