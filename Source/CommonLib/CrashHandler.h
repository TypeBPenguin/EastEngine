#pragma once

namespace eastengine
{
	namespace CrashHandler
	{
		bool Initialize(const char* strPath);
		void Release();

		void ForceCrash();
	}
}