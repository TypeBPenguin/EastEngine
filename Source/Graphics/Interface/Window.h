#pragma once

#include "CommonLib/Singleton.h"

namespace est
{
	namespace graphics
	{
		class WindowCursor;

		class Window : public Singleton<Window>
		{
			friend Singleton<Window>;
		private:
			Window();
			virtual ~Window();

		public:
			void Run(std::function<bool()> funcUpdate);

		public:
			HWND GetHwnd() const { return m_hWnd; }
			HINSTANCE GetInstanceHandle() const { return m_hInstance; }
			const math::uint2& GetScreenSize() const { return m_screenSize; }
			bool IsFullScreen() const { return m_isFullScreen; }

		public:
			void Initialize(uint32_t width, uint32_t height, bool isFullScreen, const string::StringID& applicationTitle, const string::StringID& applicationName, std::function<HRESULT(HWND, uint32_t, WPARAM, LPARAM)> messageHandler);
			bool Resize(uint32_t width, uint32_t height, bool isFullScreen);

		private:
			HWND m_hWnd{ nullptr };
			HINSTANCE m_hInstance{ nullptr };

			math::uint2 m_screenSize{ 800, 600 };
			bool m_isFullScreen{ false };

			string::StringID m_applicationTitle;
			string::StringID m_applicationName;

			std::unique_ptr<WindowCursor> m_pWindowCursor;
		};
	}
}