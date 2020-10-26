#pragma once

namespace est
{
	namespace Log
	{
		void Message(const wchar_t* file, int line, const wchar_t* msg, ...);
		void Warning(const wchar_t* file, int line, const wchar_t* msg, ...);
		void Error(const wchar_t* file, int line, const wchar_t* msg, ...);
	}
}

#define LOG_MESSAGE(format, ...)	est::Log::Message(__FILEW__, __LINE__, format, __VA_ARGS__);
#define LOG_WARNING(format, ...)	est::Log::Warning(__FILEW__, __LINE__, format, __VA_ARGS__);
#define LOG_ERROR(format, ...)		est::Log::Error(__FILEW__, __LINE__, format, __VA_ARGS__);
