#pragma once

namespace eastengine
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
			void Run(std::function<void()> funcUpdate);

		protected:
			void InitializeWindow(uint32_t nWidth, uint32_t nHeight, bool isFullScreen, const string::StringID& strApplicationTitle, const string::StringID& strApplicationName);

		protected:
			HWND m_hWnd{ nullptr };
			HINSTANCE m_hInstance{ nullptr };

			math::UInt2 m_n2ScreenSize{ 800, 600 };
			bool m_isFullScreen{ false };

			string::StringID m_strApplicationTitle;
			string::StringID m_strApplicationName;

			std::unordered_map<string::StringID, std::function<void(HWND, uint32_t, WPARAM, LPARAM)>> m_umapHandlers;
		};
	}
}