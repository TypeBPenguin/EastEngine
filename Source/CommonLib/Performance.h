#pragma once

#include "Singleton.h"

namespace eastengine
{
	namespace performance
	{
		namespace tracer
		{
			void RefreshState();

			void Start();
			void End(const char* strSaveFilePath);

			void BeginEvent(const char* strTitle, const char* strCategory, const char* strFile, int nLine);
			void EndEvent();

			template <typename T>
			void PushArgs(const char* strKey, T value);

			float TracingTime();
			bool IsTracing();

			struct Profiler
			{
				Profiler(const char* strTitle, const char* strCategory, const char* strFile, int nLine);
				~Profiler();
			};
		}
	}
}

#define TRACER_START()				eastengine::performance::tracer::Start();
#define TRACER_END(filePath)		eastengine::performance::tracer::End(filePath);
#define TRACER_EVENT(title)			eastengine::performance::tracer::Profiler profiler(title, __FUNCTION__, __FILE__, __LINE__);
#define TRACER_BEGINEVENT(title)	eastengine::performance::tracer::BeginEvent(title, __FUNCTION__, __FILE__, __LINE__);
#define TRACER_ENDEVENT()			eastengine::performance::tracer::EndEvent();
#define TRACER_PUSHARGS(key, value)	eastengine::performance::tracer::PushArgs(key, value);
