#include "stdafx.h"
#include "GraphicsInterface.h"

namespace est
{
	namespace graphics
	{
		static Options s_options;
		static Options s_prevOptions;
		static DebugInfo s_debugInfo;
		static DebugInfo s_prevDebugInfo;
		static Camera s_camera;

		Options& GetOptions()
		{
			return s_options;
		}

		Options& GetPrevOptions()
		{
			return s_prevOptions;
		}

		DebugInfo& GetDebugInfo()
		{
			return s_debugInfo;
		}

		DebugInfo& GetPrevDebugInfo()
		{
			return s_prevDebugInfo;
		}

		Camera& GetCamera()
		{
			return s_camera;
		}

		ICursor* GetCursor()
		{

		}
	}
}