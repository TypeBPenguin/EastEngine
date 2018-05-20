#include "stdafx.h"
#include "Timer.h"

namespace eastengine
{
	class Counter::Impl
	{
	public:
		Impl() = default;
		~Impl() = default;

	public:
		void Start() { m_timeStart = std::chrono::system_clock::now(); }
		void End() { m_timeEnd = std::chrono::system_clock::now(); }

		double Count() const { return std::chrono::duration<double>(m_timeEnd - m_timeStart).count(); }
		int64_t MilliSec() const { return std::chrono::duration_cast<std::chrono::milliseconds>(m_timeEnd - m_timeStart).count(); }
		int64_t MicroSec() const { return std::chrono::duration_cast<std::chrono::microseconds>(m_timeEnd - m_timeStart).count(); }
		int64_t NanoSec() const { return std::chrono::duration_cast<std::chrono::nanoseconds>(m_timeEnd - m_timeStart).count(); }

	private:
		std::chrono::system_clock::time_point m_timeStart;
		std::chrono::system_clock::time_point m_timeEnd;
	};

	Counter::Counter()
		: m_pImpl{ std::make_unique<Impl>() }
	{
	}

	Counter::~Counter()
	{
	}

	void Counter::Start()
	{
		m_pImpl->Start();
	}

	void Counter::End()
	{
		m_pImpl->End();
	}

	double Counter::Count() const
	{
		return m_pImpl->Count();
	}

	int64_t Counter::MilliSec() const
	{
		return m_pImpl->MilliSec();
	}

	int64_t Counter::MicroSec() const
	{
		return m_pImpl->MicroSec();
	}

	int64_t Counter::NanoSec() const
	{
		return m_pImpl->NanoSec();
	}

	class Timer::Impl
	{
	public:
		Impl();
		~Impl();

	public:
		void Reset();
		void Start();
		void Stop();
		void Tick();

		double GetGameTime() const;
		float GetElapsedTime() const { return static_cast<float>(m_dDeltaTime); };

	public:
		void StartTimeAction(std::function<void(uint32_t, float, float)> funcCallback, uint32_t nTimerID, uint32_t nInterval, uint32_t nLifeTime = TimeAction::eUnlimitedTime);
		void StopTimeAction(uint32_t nTimerID);

	private:
		bool m_isStopped{ false };
		double m_dDeltaTime{ 0.0 };

		std::chrono::system_clock::time_point m_baseTime;
		std::chrono::system_clock::time_point m_stopTime;
		std::chrono::system_clock::time_point m_prevTime;
		std::chrono::system_clock::time_point m_curTime;

		std::chrono::milliseconds m_pausedTime;

		std::list<TimeAction> m_listTimeActions;
	};

	Timer::Impl::Impl()
	{
		Reset();
	}

	Timer::Impl::~Impl()
	{
	}

	void Timer::Impl::Reset()
	{
		std::chrono::system_clock::time_point curTime = std::chrono::system_clock::now();
		m_baseTime = curTime;
		m_prevTime = curTime;
		m_curTime = curTime;
		m_stopTime = curTime;

		m_pausedTime = std::chrono::milliseconds::zero();

		m_isStopped = false;
	}

	void Timer::Impl::Start()
	{
		if (m_isStopped == false)
			return;

		std::chrono::system_clock::time_point curTime = std::chrono::system_clock::now();

		std::chrono::duration<double> diffTime = std::chrono::duration_cast<std::chrono::duration<double>>(curTime - m_stopTime);

		m_pausedTime += std::chrono::duration_cast<std::chrono::milliseconds>(diffTime);
		m_prevTime = curTime;
		m_stopTime = curTime;

		m_isStopped = false;
	}

	void Timer::Impl::Stop()
	{
		if (m_isStopped == true)
			return;

		std::chrono::system_clock::time_point curTime = std::chrono::system_clock::now();

		m_stopTime = curTime;
		m_isStopped = true;
	}

	void Timer::Impl::Tick()
	{
		std::chrono::system_clock::time_point curTime = std::chrono::system_clock::now();

		std::chrono::duration<double> diffTime = std::chrono::duration_cast<std::chrono::duration<double>>(m_stopTime - curTime);
		if (std::abs(diffTime.count()) <= std::numeric_limits<double>::epsilon())
		{
			m_dDeltaTime = 0.0;
			return;
		}

		m_curTime = curTime;

		m_dDeltaTime = std::chrono::duration_cast<std::chrono::duration<double>>(m_curTime - m_prevTime).count();

		m_prevTime = m_curTime;

		m_dDeltaTime = std::max(m_dDeltaTime, 0.0);

		float fElapsedTime = GetElapsedTime();
		for (auto iter = m_listTimeActions.begin(); iter != m_listTimeActions.end();)
		{
			Timer::TimeAction& timeAction = *iter;

			bool isContinue = timeAction.Update(fElapsedTime);
			if (isContinue == true)
			{
				++iter;
				continue;
			}

			iter = m_listTimeActions.erase(iter);
		}
	}

	double Timer::Impl::GetGameTime() const
	{
		if (m_isStopped)
			return std::chrono::duration_cast<std::chrono::duration<double>>((m_stopTime - m_pausedTime) - m_baseTime).count();

		return std::chrono::duration_cast<std::chrono::duration<double>>((m_curTime - m_pausedTime) - m_baseTime).count();
	}

	void Timer::Impl::StartTimeAction(std::function<void(uint32_t, float, float)> funcCallback, uint32_t nTimerID, uint32_t nInterval, uint32_t nLifeTime)
	{
		m_listTimeActions.emplace_back(funcCallback, nTimerID, nInterval, nLifeTime);
	}

	void Timer::Impl::StopTimeAction(uint32_t nTimerID)
	{
		auto iter = std::find_if(m_listTimeActions.begin(), m_listTimeActions.end(), [nTimerID](const TimeAction& timeAction)
		{
			return timeAction.nTimerID == nTimerID;
		});

		if (iter != m_listTimeActions.end())
		{
			m_listTimeActions.erase(iter);
		}
	}

	Timer::TimeAction::TimeAction(std::function<void(uint32_t, float, float)> funcCallback, uint32_t nTimerID, uint32_t nInterval, uint32_t nLifeTime)
		: funcCallback(funcCallback)
		, nTimerID(nTimerID)
		, nInterval(nInterval)
		, nLifeTime(nLifeTime)
	{
	}

	bool Timer::TimeAction::Update(float fElapsedTime)
	{
		if (nLifeTime != eUnlimitedTime)
		{
			uint32_t nProcessTime = static_cast<uint32_t>(fProcessTime * 1000.f);
			if (nProcessTime >= nLifeTime)
				return false;
		}

		fProcessTime += fElapsedTime;
		fIntervalCheckTime += fElapsedTime;

		uint32_t nIntervalCheckTime = static_cast<uint32_t>(fIntervalCheckTime * 1000.f);
		if (nIntervalCheckTime >= nInterval)
		{
			fIntervalCheckTime -= nInterval * 0.001f;

			funcCallback(nTimerID, fElapsedTime, fProcessTime);
		}

		return true;
	}

	Timer::Timer()
		: m_pImpl{ std::make_unique<Impl>() }
	{
	}

	Timer::~Timer()
	{
	}

	void Timer::Reset()
	{
		m_pImpl->Reset();
	}

	void Timer::Start()
	{
		m_pImpl->Start();
	}

	void Timer::Stop()
	{
		m_pImpl->Stop();
	}

	void Timer::Tick()
	{
		m_pImpl->Tick();
	}

	double Timer::GetGameTime() const
	{
		return m_pImpl->GetGameTime();
	}

	float Timer::GetElapsedTime() const
	{
		return m_pImpl->GetElapsedTime();
	}

	void Timer::StartTimeAction(std::function<void(uint32_t, float, float)> funcCallback, uint32_t nTimerID, uint32_t nInterval, uint32_t nLifeTime)
	{
		m_pImpl->StartTimeAction(funcCallback, nTimerID, nInterval, nLifeTime);
	}

	void Timer::StopTimeAction(uint32_t nTimerID)
	{
		m_pImpl->StopTimeAction(nTimerID);
	}
}