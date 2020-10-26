#pragma once

#include "Singleton.h"

#define ENABLE_TRACING

namespace est
{
	namespace performance
	{
		namespace tracer
		{
			void RefreshState();

			void Start();
			void End(const wchar_t* saveFilePath);

			void BeginEvent(const wchar_t* title, const wchar_t* category, const wchar_t* file, int line);
			void EndEvent(bool isForceRecord);

			template <typename T>
			void PushArgs(const wchar_t* key, T value);

			float TracingTime();
			bool IsTracing();

			class Profiler
			{
			public:
				Profiler(const wchar_t* title, const wchar_t* category, const wchar_t* file, int line, bool isForceRecord);
				~Profiler();

			private:
				bool m_isForceRecord{ false };
			};
		}
	}
}

#ifdef ENABLE_TRACING
#define TRACER_START()				est::performance::tracer::Start()
#define TRACER_END(filePath)		est::performance::tracer::End(filePath)
#define TRACER_PROFILER(title, function, file, line, isForceRecord)	est::performance::tracer::Profiler profiler_##line(title, function, file, line, isForceRecord)
#define TRACER_PROFILER_DEFINE(title, function, file, line, isForceRecord)	TRACER_PROFILER(title, function, file, line, isForceRecord)
#define TRACER_EVENT(title)			TRACER_PROFILER_DEFINE(title, L"", __FILEW__, __LINE__, false)
#define TRACER_EVENT_FORCE_RECORD(title)			TRACER_PROFILER_DEFINE(title, L"", __FILEW__, __LINE__, true)
#define TRACER_BEGINEVENT(title)	est::performance::tracer::BeginEvent(title, L"", __FILEW__, __LINE__)
#define TRACER_ENDEVENT()			est::performance::tracer::EndEvent(false)
#define TRACER_PUSHARGS(key, value)	est::performance::tracer::PushArgs(key, value)
#else
#define TRACER_START()
#define TRACER_END(filePath)
#define TRACER_PROFILER(title, function, file, line, isForceRecord)
#define TRACER_PROFILER_DEFINE(title, function, file, line, isForceRecord)
#define TRACER_EVENT(title)
#define TRACER_EVENT_FORCE_RECORD(title)
#define TRACER_BEGINEVENT(title)
#define TRACER_ENDEVENT()
#define TRACER_PUSHARGS(key, value)
#endif