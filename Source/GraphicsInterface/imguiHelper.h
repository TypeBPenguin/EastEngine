#pragma once

#include "GraphicsInterface/GraphicsInterface.h"

#include "imgui.h"

namespace eastengine
{
	namespace math
	{
		inline const ImVec2& Convert(const float2& vec2)
		{
			return *reinterpret_cast<const ImVec2*>(&vec2);
		}

		inline const float2& Convert(const ImVec2& vec2)
		{
			return *reinterpret_cast<const float2*>(&vec2);
		}
	}

	namespace imguiHelper
	{
		bool MessageHandler(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam);

		ImTextureID GetTextureID(const graphics::ITexture* pTexture);
	}
}