#pragma once

namespace EastEngine
{
	namespace Performance
	{
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