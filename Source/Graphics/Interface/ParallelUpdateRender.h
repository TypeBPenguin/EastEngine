#pragma once

#include "Camera.h"
#include "GraphicsInterface.h"

namespace est
{
	namespace graphics
	{
		size_t UpdateThread();
		size_t RenderThread();
		Camera& RenderCamera();
		Options& RenderOptions();
		Options& PrevRenderOptions();
		void SwapThread();
	}
}