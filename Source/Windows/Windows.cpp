#include "stdafx.h"
#include "Windows.h"

namespace EastEngine
{
	namespace Windows
	{
		LRESULT CALLBACK WndProc(HWND hWnd, uint32_t nMsg, WPARAM wParam, LPARAM lParam)
		{
			static bool isImeSetting = false;
			if (nMsg == WM_IME_SETCONTEXT && isImeSetting == false)
			{
				lParam &= ~(ISC_SHOWUICOMPOSITIONWINDOW | ISC_SHOWUIALLCANDIDATEWINDOW);
				lParam = 0;

				isImeSetting = true;

				return ImmIsUIMessage(hWnd, nMsg, wParam, lParam);
			}

			switch (nMsg)
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
			// �ٸ� ��� �޼������� ��ϵ� MessageHandler �� �����մϴ�.
			default:
			{
				WindowsManager::GetInstance()->HandleMessage(hWnd, nMsg, wParam, lParam);
			}
			}

			return DefWindowProc(hWnd, nMsg, wParam, lParam);
		}

		class WindowsManager::Impl
		{
		public:
			Impl();
			~Impl();

		public:
			bool Initialize(const char* strApplicationName, uint32_t nScreenWidth, uint32_t nScreenHeight, bool isFullScreen);

		public:
			void ProcessMessages();

			bool AddMessageHandler(FuncMessageHandler funcMessageHandler);

			void HandleMessage(HWND hWnd, uint32_t nMsg, WPARAM wParam, LPARAM lParam);

		public:
			HWND GetHwnd() { return m_hWnd; }
			HINSTANCE GetHInstance() { return m_hInstance; }

		private:
			bool m_isInitialized{ false };
			bool m_isFullScreen{ false };

			std::string m_strApplicationName;
			HINSTANCE m_hInstance{ nullptr };
			HWND m_hWnd{ nullptr };

			uint32_t m_nScreenWidth{ 0 };
			uint32_t m_nScreenHeight{ 0 };

			std::mutex m_mutex;
			std::vector<FuncMessageHandler> m_vecFuncMessageHandler;
			std::queue<MessageData> m_queueMessageData;
		};

		WindowsManager::Impl::Impl()
		{
		}

		WindowsManager::Impl::~Impl()
		{
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
		}

		bool WindowsManager::Impl::Initialize(const char* strApplicationName, uint32_t nScreenWidth, uint32_t nScreenHeight, bool isFullScreen)
		{
			if (m_isInitialized == true)
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

			m_isInitialized = true;

			return true;
		}

		void WindowsManager::Impl::ProcessMessages()
		{
			std::lock_guard<std::mutex> lock(m_mutex);

			if (m_queueMessageData.empty() == true)
				return;

			const size_t nSize = m_vecFuncMessageHandler.size();

			while (m_queueMessageData.empty() == false)
			{
				const MessageData& messageData = m_queueMessageData.front();

				for (size_t i = 0; i < nSize; ++i)
				{
					m_vecFuncMessageHandler[i](messageData.hWnd, messageData.nMsg, messageData.wParam, messageData.lParam);
				}

				m_queueMessageData.pop();
			}
		}

		bool WindowsManager::Impl::AddMessageHandler(FuncMessageHandler funcMessageHandler)
		{
			if (funcMessageHandler == nullptr)
				return false;

			std::lock_guard<std::mutex> lock(m_mutex);

			m_vecFuncMessageHandler.push_back(funcMessageHandler);

			return true;
		}

		void WindowsManager::Impl::HandleMessage(HWND hWnd, uint32_t nMsg, WPARAM wParam, LPARAM lParam)
		{
			std::lock_guard<std::mutex> lock(m_mutex);

			m_queueMessageData.emplace(hWnd, nMsg, wParam, lParam);
		}

		MessageData::MessageData(HWND _hWnd, uint32_t _nMsg, WPARAM _wParam, LPARAM _lParam)
			: hWnd(_hWnd)
			, nMsg(_nMsg)
			, wParam(_wParam)
			, lParam(_lParam)
		{
		}

		WindowsManager::WindowsManager()
			: m_pImpl{ std::make_unique<Impl>() }
		{
		}

		WindowsManager::~WindowsManager()
		{
		}

		bool WindowsManager::Initialize(const char* strApplicationName, uint32_t nScreenWidth, uint32_t nScreenHeight, bool isFullScreen)
		{
			return m_pImpl->Initialize(strApplicationName, nScreenWidth, nScreenHeight, isFullScreen);
		}

		void WindowsManager::ProcessMessages()
		{
			m_pImpl->ProcessMessages();
		}

		bool WindowsManager::AddMessageHandler(FuncMessageHandler funcMessageHandler)
		{
			return m_pImpl->AddMessageHandler(funcMessageHandler);
		}

		void WindowsManager::HandleMessage(HWND hWnd, uint32_t nMsg, WPARAM wParam, LPARAM lParam)
		{
			m_pImpl->HandleMessage(hWnd, nMsg, wParam, lParam);
		}

		HWND WindowsManager::GetHwnd()
		{
			return m_pImpl->GetHwnd();
		}

		HINSTANCE WindowsManager::GetHInstance()
		{
			return m_pImpl->GetHInstance();
		}
	}
}