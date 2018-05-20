#pragma once

namespace eastengine
{
	namespace Log
	{
		void Message(const char* file, int line, const char* msg, ...);
		void Warning(const char* file, int line, const char* msg, ...);
		void Error(const char* file, int line, const char* msg, ...);
	}
}

#define LOG_MESSAGE(format, ...)	eastengine::Log::Message(__FILE__, __LINE__, format, __VA_ARGS__);
#define LOG_WARNING(format, ...)	eastengine::Log::Warning(__FILE__, __LINE__, format, __VA_ARGS__);
#define LOG_ERROR(format, ...)		eastengine::Log::Error(__FILE__, __LINE__, format, __VA_ARGS__);
