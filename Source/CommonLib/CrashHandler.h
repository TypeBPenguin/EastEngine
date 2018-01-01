#pragma once

namespace EastEngine
{
	namespace CrashHandler
	{
		bool Initialize();
		void Release();

		void ForceCrash();
	}
}