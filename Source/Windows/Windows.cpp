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

			// �� ���ø����̼��� �ν��Ͻ��� �����ɴϴ�.
			m_hInstance = GetModuleHandle(nullptr);

			// ���ø����̼��� �̸��� ����
			m_strApplicationName = strApplicationName;

			// ������ Ŭ������ �⺻ �������� ����
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

			// ������ Ŭ������ ���
			RegisterClassEx(&wc);

			// Ǯ��ũ�� ��� ������ ���� ���� ȭ�� ������ �Ѵ�.
			if (m_isFullScreen)
			{
				// ���� Ǯ��ũ�� ����� ȭ�� ũ�⸦ ����ũ�� ũ�⿡ ���߰� ������ 32bit�� �մϴ�.
				memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
				dmScreenSettings.dmSize = sizeof(dmScreenSettings);
				dmScreenSettings.dmPelsWidth = GetSystemMetrics(SM_CXSCREEN);
				dmScreenSettings.dmPelsHeight = GetSystemMetrics(SM_CYSCREEN);
				dmScreenSettings.dmBitsPerPel = 32;
				dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

				// Ǯ��ũ���� �´� ���÷��� ������ �մϴ�.
				ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);

				// �������� ��ġ�� ȭ���� ���� ���� ����ϴ�.
				posX = posY = 0;
			}
			else
			{
				// â�� ������� �߾ӿ� ������ �մϴ�.
				posX = static_cast<int>((GetSystemMetrics(SM_CXSCREEN) - nScreenWidth) * 0.5f);
				posY = static_cast<int>((GetSystemMetrics(SM_CYSCREEN) - nScreenHeight) * 0.5f);
			}

			Math::Rect rtWindowSize;
			rtWindowSize.right = nScreenWidth;
			rtWindowSize.bottom = nScreenHeight;
			::AdjustWindowRect(&rtWindowSize, WS_OVERLAPPEDWINDOW | WS_VISIBLE, false);

			// ������ ������ â�� ����� �ڵ��� �����´�.
			m_hWnd = CreateWindowEx(WS_EX_APPWINDOW, strApplicationName, strApplicationName,
				//WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP,
				WS_OVERLAPPEDWINDOW | WS_VISIBLE,
				posX, posY, rtWindowSize.GetWidth(), rtWindowSize.GetHeight(), nullptr, nullptr, m_hInstance, nullptr);

			if (m_hWnd == nullptr)
				return false;

			// �����츦 ȭ�鿡 ǥ��
			ShowWindow(m_hWnd, SW_SHOW);
			UpdateWindow(m_hWnd);

			m_isInit = true;

			return true;
		}

		void WindowsManager::Release()
		{
			if (m_isInit == false)
				return;

			// Ǯ��ũ�� ��带 �������� �� ���÷��� ���� �ٲ�
			if (m_isFullScreen)
			{
				ChangeDisplaySettings(NULL, 0);
			}

			// â�� ����
			DestroyWindow(m_hWnd);

			// ���ø����̼� �ν��Ͻ��� ����
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
				// �����찡 ���ŵǾ����� Ȯ���մϴ�.
			case WM_DESTROY:
			{
				PostQuitMessage(0);
				return 0;
			}

			// �����찡 �������� Ȯ���մϴ�.
			case WM_CLOSE:
			{
				PostQuitMessage(0);
				return 0;
			}

			// �ٸ� ��� �޼������� system Ŭ������ �޼��� ó���⿡ �����մϴ�.
			default:
			{
				return WindowsManager::GetInstance()->HandleMsg(hwnd, umessage, wparam, lparam);
			}
			}
		}
	}
}