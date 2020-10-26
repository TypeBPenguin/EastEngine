#pragma once

namespace est
{
	namespace graphics
	{
		class Window
		{
		public:
			Window();
			virtual ~Window() = 0;

		protected:
			virtual void Update() = 0;
			virtual void Render() = 0;
			virtual void Present() = 0;

		public:
			void AddMessageHandler(const string::StringID& strName, std::function<void(HWND, uint32_t, WPARAM, LPARAM)> funcHandler);
			void RemoveMessageHandler(const string::StringID& strName);
			void Run(std::function<bool()> funcUpdate);

		protected:
			void InitializeWindow(uint32_t nWidth, uint32_t nHeight, bool isFullScreen, const string::StringID& applicationTitle, const string::StringID& applicationName);

		protected:
			HWND m_hWnd{ nullptr };
			HINSTANCE m_hInstance{ nullptr };

			math::uint2 m_n2ScreenSize{ 800, 600 };
			bool m_isFullScreen{ false };
			bool m_isRunning{ true };

			string::StringID m_applicationTitle;
			string::StringID m_applicationName;

			tsl::robin_map<string::StringID, std::function<void(HWND, uint32_t, WPARAM, LPARAM)>> m_umapHandlers;
		};
	}
}