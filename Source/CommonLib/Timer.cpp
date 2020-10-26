#include "stdafx.h"
#include "Timer.h"

namespace est
{
	class Stopwatch::Impl
	{
	public:
		Impl() = default;
		~Impl() = default;

	public:
		void Start() { m_timeStart = std::chrono::high_resolution_clock::now(); }
		void Stop() { m_timeEnd = std::chrono::high_resolution_clock::now(); }

		double Elapsed() const { return std::chrono::duration<double>(m_timeEnd - m_timeStart).count(); }
		int64_t MilliSec() const { return std::chrono::duration_cast<std::chrono::milliseconds>(m_timeEnd - m_timeStart).count(); }
		int64_t MicroSec() const { return std::chrono::duration_cast<std::chrono::microseconds>(m_timeEnd - m_timeStart).count(); }
		int64_t NanoSec() const { return std::chrono::duration_cast<std::chrono::nanoseconds>(m_timeEnd - m_timeStart).count(); }

	private:
		std::chrono::high_resolution_clock::time_point m_timeStart;
		std::chrono::high_resolution_clock::time_point m_timeEnd;
	};

	Stopwatch::Stopwatch()
		: m_pImpl{ std::make_unique<Impl>() }
	{
	}

	Stopwatch::~Stopwatch()
	{
	}

	void Stopwatch::Start()
	{
		m_pImpl->Start();
	}

	void Stopwatch::Stop()
	{
		m_pImpl->Stop();
	}

	double Stopwatch::Elapsed() const
	{
		return m_pImpl->Elapsed();
	}

	int64_t Stopwatch::MilliSec() const
	{
		return m_pImpl->MilliSec();
	}

	int64_t Stopwatch::MicroSec() const
	{
		return m_pImpl->MicroSec();
	}

	int64_t Stopwatch::NanoSec() const
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
		float GetElapsedTime() const { return static_cast<float>(m_elapsedTime); };

		void SetLimitElapsedTime(double limitElapsedTime) { m_limitElapsedTime = limitElapsedTime; }

	public:
		void StartTimeAction(std::function<void(uint32_t, float, float)> funcCallback, uint32_t timerID, uint32_t interval, uint32_t lifeTime = TimeAction::eUnlimitedTime);
		void StopTimeAction(uint32_t timerID);

	private:
		bool m_isStopped{ false };
		double m_elapsedTime{ 0.0 };
		double m_limitElapsedTime{ std::numeric_limits<double>::max() };
		double m_gameTime{ 0.0 };

		std::chrono::high_resolution_clock::time_point m_baseTime;
		std::chrono::high_resolution_clock::time_point m_stopTime;
		std::chrono::high_resolution_clock::time_point m_prevTime;
		std::chrono::high_resolution_clock::time_point m_curTime;

		std::chrono::milliseconds m_pausedTime{};

		std::vector<TimeAction> m_timeActions;
	};

	Timer::Impl::Impl()
	{
	}

	Timer::Impl::~Impl()
	{
	}

	void Timer::Impl::Reset()
	{
		std::chrono::high_resolution_clock::time_point curTime = std::chrono::high_resolution_clock::now();
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

		std::chrono::high_resolution_clock::time_point curTime = std::chrono::high_resolution_clock::now();

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

		std::chrono::high_resolution_clock::time_point curTime = std::chrono::high_resolution_clock::now();

		m_stopTime = curTime;
		m_isStopped = true;
	}

	void Timer::Impl::Tick()
	{
		std::chrono::high_resolution_clock::time_point curTime = std::chrono::high_resolution_clock::now();

		std::chrono::duration<double> diffTime = std::chrono::duration_cast<std::chrono::duration<double>>(m_stopTime - curTime);
		if (std::abs(diffTime.count()) <= std::numeric_limits<double>::epsilon())
		{
			m_elapsedTime = 0.0;
			return;
		}

		m_curTime = curTime;

		m_elapsedTime = std::clamp(std::chrono::duration_cast<std::chrono::duration<double>>(m_curTime - m_prevTime).count(), 0.0, m_limitElapsedTime);

		m_prevTime = m_curTime;

		if (m_isStopped == true)
		{
			m_gameTime = std::chrono::duration_cast<std::chrono::duration<double>>((m_stopTime - m_pausedTime) - m_baseTime).count();
		}
		else
		{
			m_gameTime = std::chrono::duration_cast<std::chrono::duration<double>>((m_curTime - m_pausedTime) - m_baseTime).count();;
		}

		const float elapsedTime = GetElapsedTime();
		m_timeActions.erase(std::remove_if(m_timeActions.begin(), m_timeActions.end(), [elapsedTime](TimeAction& timeActions)
		{
			if (timeActions.isStopRequest == true)
				return true;

			return timeActions.Update(elapsedTime);
		}), m_timeActions.end());
	}

	double Timer::Impl::GetGameTime() const
	{
		return m_gameTime;
	}

	void Timer::Impl::StartTimeAction(std::function<void(uint32_t, float, float)> funcCallback, uint32_t timerID, uint32_t interval, uint32_t lifeTime)
	{
		m_timeActions.emplace_back(funcCallback, timerID, interval, lifeTime);
	}

	void Timer::Impl::StopTimeAction(uint32_t timerID)
	{
		auto iter = std::find_if(m_timeActions.begin(), m_timeActions.end(), [timerID](const TimeAction& timeAction)
		{
			return timeAction.timerID == timerID;
		});

		if (iter != m_timeActions.end())
		{
			iter->isStopRequest = true;
		}
	}

	Timer::TimeAction::TimeAction(std::function<void(uint32_t, float, float)> funcCallback, uint32_t timerID, uint32_t interval, uint32_t lifeTime)
		: funcCallback(funcCallback)
		, timerID(timerID)
		, interval(interval)
		, lifeTime(lifeTime)
	{
	}

	Timer::TimeAction::TimeAction(TimeAction&& source) noexcept
	{
		*this = std::move(source);
	}

	Timer::TimeAction& Timer::TimeAction::operator = (TimeAction&& source) noexcept
	{
		timerID = std::move(source.timerID);
		interval = std::move(source.interval);
		lifeTime = std::move(source.lifeTime);
		intervalCheckTime = std::move(source.intervalCheckTime);
		processTime = std::move(source.processTime);
		funcCallback = std::move(source.funcCallback);
		isStopRequest = std::move(source.isStopRequest);

		return *this;
	}

	bool Timer::TimeAction::Update(float elapsedTime)
	{
		if (lifeTime != eUnlimitedTime)
		{
			uint32_t nProcessTime = static_cast<uint32_t>(processTime * 1000.f);
			if (nProcessTime >= lifeTime)
				return false;
		}

		processTime += elapsedTime;
		intervalCheckTime += elapsedTime;

		const uint32_t intervalCheckTimeMS = static_cast<uint32_t>(intervalCheckTime * 1000.f);
		if (intervalCheckTimeMS >= interval)
		{
			intervalCheckTime -= static_cast<float>(interval) * 0.001f;

			funcCallback(timerID, elapsedTime, processTime);
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

	void Timer::SetLimitElapsedTime(double limitElapsedTime)
	{
		m_pImpl->SetLimitElapsedTime(limitElapsedTime);
	}

	void Timer::StartTimeAction(std::function<void(uint32_t, float, float)> funcCallback, uint32_t timerID, uint32_t interval, uint32_t lifeTime)
	{
		m_pImpl->StartTimeAction(funcCallback, timerID, interval, lifeTime);
	}

	void Timer::StopTimeAction(uint32_t timerID)
	{
		m_pImpl->StopTimeAction(timerID);
	}
}