#pragma once

#include "Singleton.h"

namespace EastEngine
{
	class Timer : public Singleton<Timer>
	{
		friend Singleton<Timer>;
	public:
		struct TimeAction
		{
			enum
			{
				eUnlimitedTime = UINT32_MAX,
			};

			uint32_t nTimerID = 0;
			uint32_t nInterval = 0;
			uint32_t nLifeTime = 0;
			float fIntervalCheckTime = 0.f;
			float fProcessTime = 0.f;
			std::function<void(float, float)> funcCallback = nullptr;

			TimeAction(std::function<void(float, float)> funcCallback, uint32_t nTimerID, uint32_t nInterval, uint32_t nLifeTime);

			bool Update(float fElapsedTime);
		};

	private:
		Timer();
		virtual ~Timer();

	public:
		void Reset();	// 시간 초기화 함수, 루프 이전에 호출
		void Start();	// 타이머 시작 또는 재개 시 호출
		void Stop();	// 일시정지
		void Tick();	// 매 프레임마다 호출

		// 초단위
		double GetGameTime() const;
		float GetDeltaTime() const { return static_cast<float>(m_dDeltaTime); };

	public:
		void StartTimer(std::function<void(float, float)> funcCallback, uint32_t nTimerID, uint32_t nInterval, uint32_t nLifeTime = TimeAction::eUnlimitedTime)
		{
			m_listTimeActions.emplace_back(funcCallback, nTimerID, nInterval, nLifeTime);
		}

	private:
		bool m_isStopped;
		double m_dDeltaTime;

		std::chrono::system_clock::time_point m_baseTime;
		std::chrono::system_clock::time_point m_stopTime;
		std::chrono::system_clock::time_point m_prevTime;
		std::chrono::system_clock::time_point m_curTime;

		std::chrono::milliseconds m_pausedTime;

		std::list<Timer::TimeAction> m_listTimeActions;
	};
}