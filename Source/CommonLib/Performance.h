#pragma once

#include "Singleton.h"

namespace EastEngine
{
	namespace Performance
	{
		class Tracer : public Singleton<Tracer>
		{
			friend Singleton<Tracer>;
		private:
			Tracer();
			virtual ~Tracer();

		public:
			void RefreshState();

		public:
			void Start();
			void End(const char* strSaveFilePath);

			void BeginEvent(const char* strTitle, const char* strCategory, const char* strFile, int nLine);
			void EndEvent();

			template <typename T>
			void PushArgs(const char* strKey, T value);

		public:
			float TracingTime() const;
			bool IsTracing() const;

		public:
			struct Profiler
			{
				Profiler(const char* strTitle, const char* strCategory, const char* strFile, int nLine);
				~Profiler();
			};

		public:
			class Impl;
			std::unique_ptr<Impl> m_pImpl;
		};

		class Counter
		{
		public:
			Counter();
			~Counter();

			void Start() { m_timeStart = std::chrono::system_clock::now(); }
			void End() { m_timeEnd = std::chrono::system_clock::now(); }

			double Count() { return std::chrono::duration<double>(m_timeEnd - m_timeStart).count(); }
			int64_t MilliSec() { return std::chrono::duration_cast<std::chrono::milliseconds>(m_timeEnd - m_timeStart).count(); }
			int64_t MicroSec() { return std::chrono::duration_cast<std::chrono::microseconds>(m_timeEnd - m_timeStart).count(); }
			int64_t NanoSec() { return std::chrono::duration_cast<std::chrono::nanoseconds>(m_timeEnd - m_timeStart).count(); }

		private:
			std::chrono::system_clock::time_point m_timeStart;
			std::chrono::system_clock::time_point m_timeEnd;
		};
	}
}

#define PERF_TRACER_START()						EastEngine::Performance::Tracer::GetInstance()->Start();
#define PERF_TRACER_END(filePath)				EastEngine::Performance::Tracer::GetInstance()->End(filePath);
#define PERF_TRACER_EVENT(title, category)		EastEngine::Performance::Tracer::Profiler profiler(title, category, __FILE__, __LINE__);
#define PERF_TRACER_BEGINEVENT(title, category)	EastEngine::Performance::Tracer::GetInstance()->BeginEvent(title, category, __FILE__, __LINE__);
#define PERF_TRACER_ENDEVENT()					EastEngine::Performance::Tracer::GetInstance()->EndEvent();
#define PERF_TRACER_PUSHARGS(key, value)		EastEngine::Performance::Tracer::GetInstance()->PushArgs(key, value);