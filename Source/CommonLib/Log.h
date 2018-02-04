#pragma once

namespace EastEngine
{
	namespace Log
	{
		void Message(const char* file, int line, const char* msg, ...);
		void Warning(const char* file, int line, const char* msg, ...);
		void Error(const char* file, int line, const char* msg, ...);
	}
}

#define LOG_MESSAGE(format, ...)	EastEngine::Log::Message(__FILE__, __LINE__, format, __VA_ARGS__);
#define LOG_WARNING(format, ...)	EastEngine::Log::Warning(__FILE__, __LINE__, format, __VA_ARGS__);
#define LOG_ERROR(format, ...)		EastEngine::Log::Error(__FILE__, __LINE__, format, __VA_ARGS__);