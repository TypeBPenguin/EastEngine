#pragma once

namespace EastEngine
{
	namespace Log
	{
		void ConsoleLog(const char* msg, ...);
	}
}

#define PRINT_LOG(format, ...) EastEngine::Log::ConsoleLog(format, __VA_ARGS__); \
	EastEngine::Log::ConsoleLog(" -> %s <%d>\n", __FILE__, __LINE__);