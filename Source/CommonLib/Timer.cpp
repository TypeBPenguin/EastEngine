#include "stdafx.h"
#include "Timer.h"

namespace EastEngine
{
	Timer::TimeAction::TimeAction(std::function<void(float, float)> funcCallback, uint32_t nTimerID, uint32_t nInterval, uint32_t nLifeTime)
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

			funcCallback(fElapsedTime, fProcessTime);
		}

		return true;
	}

	Timer::Timer()
		: m_dDeltaTime(0.0)
		, m_isStopped(false)
	{
		Reset();
	}

	Timer::~Timer()
	{
	}

	void Timer::Reset()
	{
		std::chrono::system_clock::time_point curTime = std::chrono::system_clock::now();
		m_baseTime = curTime;
		m_prevTime = curTime;
		m_curTime = curTime;
		m_stopTime = curTime;

		m_pausedTime = std::chrono::milliseconds::zero();

		m_isStopped = false;
	}

	void Timer::Start()
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

	void Timer::Stop()
	{
		if (m_isStopped == true)
			return;

		std::chrono::system_clock::time_point curTime = std::chrono::system_clock::now();

		m_stopTime = curTime;
		m_isStopped = true;
	}

	void Timer::Tick()
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

		float fElapsedTime = GetDeltaTime();
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

	double Timer::GetGameTime() const
	{
		if (m_isStopped)
			return std::chrono::duration_cast<std::chrono::duration<double>>((m_stopTime - m_pausedTime) - m_baseTime).count();

		return std::chrono::duration_cast<std::chrono::duration<double>>((m_curTime - m_pausedTime) - m_baseTime).count();
	}
}