#pragma once

#include "Singleton.h"

namespace eastengine
{
	namespace Performance
	{
		namespace Tracer
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

#define TRACER_START()				eastengine::Performance::Tracer::Start();
#define TRACER_END(filePath)		eastengine::Performance::Tracer::End(filePath);
#define TRACER_EVENT(title)			eastengine::Performance::Tracer::Profiler profiler(title, __FUNCTION__, __FILE__, __LINE__);
#define TRACER_BEGINEVENT(title)	eastengine::Performance::Tracer::BeginEvent(title, __FUNCTION__, __FILE__, __LINE__);
#define TRACER_ENDEVENT()			eastengine::Performance::Tracer::EndEvent();
#define TRACER_PUSHARGS(key, value)	eastengine::Performance::Tracer::PushArgs(key, value);
