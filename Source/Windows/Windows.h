#pragma once

#include "CommonLib/Singleton.h"

namespace EastEngine
{
	namespace Windows
	{
		typedef bool(*FuncMessageHandler)(HWND hWnd, uint32_t nMsg, WPARAM wParam, LPARAM lParam);

		struct MessageData
		{
			HWND hWnd;
			uint32_t nMsg;
			WPARAM wParam;
			LPARAM lParam;

			MessageData(HWND _hWnd, uint32_t _nMsg, WPARAM _wParam, LPARAM _lParam);
		};

		class WindowsManager : public Singleton<WindowsManager>
		{
			friend Singleton<WindowsManager>;
		private:
			WindowsManager();
			virtual ~WindowsManager();

		public:
			bool Init(const char* strApplicationName, uint32_t nScreenWidth, uint32_t nScreenHeight, bool isFullScreen);
			void Release();

			void ProcessMessages();

			bool AddMessageHandler(FuncMessageHandler funcMessageHandler);

			LRESULT HandleMsg(HWND hWnd, uint32_t nMsg, WPARAM wParam, LPARAM lParam);

		public:
			HWND GetHwnd() { return m_hWnd; }
			HINSTANCE GetHInstance() { return m_hInstance; }

		private:
			std::string m_strApplicationName;
			HINSTANCE m_hInstance;
			HWND m_hWnd;

			uint32_t m_nScreenWidth;
			uint32_t m_nScreenHeight;

			std::vector<FuncMessageHandler> m_vecFuncMessageHandler;
			std::queue<MessageData> m_queueMessageData;

			bool m_isFullScreen;
			bool m_isInit;
		};

		// Function Prototypes
		static LRESULT CALLBACK WndProc(HWND hWnd, uint32_t nMsg, WPARAM wParam, LPARAM lParam);

		inline HWND GetHwnd() { return WindowsManager::GetInstance()->GetHwnd(); }
		inline HINSTANCE GetHInstance() { return WindowsManager::GetInstance()->GetHInstance(); }
	}
}