#pragma once

namespace EastEngine
{
	namespace CrashHandler
	{
		bool Initialize(const char* strPath);
		void Release();

		void ForceCrash();
	}
}