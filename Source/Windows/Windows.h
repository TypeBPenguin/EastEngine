#pragma once

#include "CommonLib/Singleton.h"

namespace EastEngine
{
	namespace Windows
	{
		using FuncMessageHandler = bool(*)(HWND hWnd, uint32_t nMsg, WPARAM wParam, LPARAM lParam);

		struct MessageData
		{
			HWND hWnd{ nullptr };
			uint32_t nMsg{ 0 };
			WPARAM wParam{ 0 };
			LPARAM lParam{ 0 };

			MessageData(HWND _hWnd, uint32_t _nMsg, WPARAM _wParam, LPARAM _lParam);
		};

		class WindowsManager : public Singleton<WindowsManager>
		{
			friend Singleton<WindowsManager>;
		private:
			WindowsManager();
			virtual ~WindowsManager();

		public:
			bool Initialize(const char* strApplicationName, uint32_t nScreenWidth, uint32_t nScreenHeight, bool isFullScreen);

		public:
			void ProcessMessages();

			bool AddMessageHandler(FuncMessageHandler funcMessageHandler);

			void HandleMessage(HWND hWnd, uint32_t nMsg, WPARAM wParam, LPARAM lParam);

		public:
			HWND GetHwnd();
			HINSTANCE GetHInstance();

		private:
			class Impl;
			std::unique_ptr<Impl> m_pImpl;
		};

		inline HWND GetHwnd() { return WindowsManager::GetInstance()->GetHwnd(); }
		inline HINSTANCE GetHInstance() { return WindowsManager::GetInstance()->GetHInstance(); }
	}
}