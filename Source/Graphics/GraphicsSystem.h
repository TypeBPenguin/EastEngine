#pragma once

#include "CommonLib/Singleton.h"

namespace EastEngine
{
	namespace Graphics
	{
		using FuncAfterRender = std::function<void()>;

		static const float s_fScreenDepth = 10000.f;
		static const float s_fScreenNear = 0.1f;

		class GraphicsSystem : public Singleton<GraphicsSystem>
		{
			friend Singleton<GraphicsSystem>;
		private:
			GraphicsSystem();
			virtual ~GraphicsSystem();

		public:
			bool Initialize(HWND hWnd, uint32_t nScreenWidth, uint32_t nScreenHeight, bool isFullScreen, bool isVsync, float fFlushCycleTime);

		public:
			bool HandleMessage(HWND hWnd, uint32_t nMsg, WPARAM wParam, LPARAM lParam);

		public:
			void Update(float fElapsedTime);
			void Render();

			void Flush(float fElapsedTime);

		public:
			void BeginScene(float r, float g, float b, float a);
			void EndScene();

			void AddFuncAfterRender(FuncAfterRender func);

		private:
			class Impl;
			std::unique_ptr<Impl> m_pImpl;
		};
	}
}