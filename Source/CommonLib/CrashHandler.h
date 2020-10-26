#pragma once

namespace est
{
	namespace CrashHandler
	{
		bool Initialize(const wchar_t* path);
		void Release();

		void ForceCrash();
	}
}