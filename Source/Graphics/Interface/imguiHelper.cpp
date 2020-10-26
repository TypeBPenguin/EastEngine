#include "stdafx.h"
#include "imguiHelper.h"

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace est
{
	namespace imguiHelper
	{
		bool MessageHandler(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
		{
			TRACER_EVENT(__FUNCTIONW__);
			return ImGui_ImplWin32_WndProcHandler(hWnd, nMsg, wParam, lParam) > 0;
		}
	}
}