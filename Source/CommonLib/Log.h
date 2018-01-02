#pragma once

namespace EastEngine
{
	namespace Log
	{
		void Message(const char* msg, ...);
		void Warning(const char* msg, ...);
		void Error(const char* msg, ...);
	}
}

#define LOG_MESSAGE(format, ...)	EastEngine::Log::Message("[Message] ");	\
	EastEngine::Log::Message(format, __VA_ARGS__);								\
	EastEngine::Log::Message(" -> [%s <%d>]\n", __FILE__, __LINE__);

#define LOG_WARNING(format, ...)	EastEngine::Log::Warning("[Warning] ");	\
	EastEngine::Log::Warning(format, __VA_ARGS__);								\
	EastEngine::Log::Warning(" -> [%s <%d>]\n", __FILE__, __LINE__);

#define LOG_ERROR(format, ...)	EastEngine::Log::Error("[Error] ");	\
	EastEngine::Log::Error(format, __VA_ARGS__);						\
	EastEngine::Log::Error(" -> [%s <%d>]\n", __FILE__, __LINE__);