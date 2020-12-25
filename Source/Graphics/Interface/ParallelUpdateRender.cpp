#include "stdafx.h"
#include "ParallelUpdateRender.h"

namespace est
{
	namespace graphics
	{
		size_t g_updateThread{ 0 };
		size_t g_renderThread{ 1 };
		Camera g_renderCamera;
		Options g_renderOption;
		Options g_prevRenderOption;

		size_t UpdateThread()
		{
			return g_updateThread;
		}

		size_t RenderThread()
		{
			return g_renderThread;
		}

		Camera& RenderCamera()
		{
			return g_renderCamera;
		}

		Options& RenderOptions()
		{
			return g_renderOption;
		}

		Options& PrevRenderOptions()
		{
			return g_prevRenderOption;
		}

		void SwapThread()
		{
			std::swap(g_updateThread, g_renderThread);
			RenderCamera().SetView(GetCamera().GetView());
			RenderCamera().SetProjection(GetCamera().GetProjection());
			RenderCamera().SetOrthographic(GetCamera().GetOrthographic());
			PrevRenderOptions() = RenderOptions();
			RenderOptions() = GetOptions();
		}
	}
}