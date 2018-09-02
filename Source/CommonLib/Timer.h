#pragma once

#include "Singleton.h"

namespace eastengine
{
	class Stopwatch
	{
	public:
		Stopwatch();
		~Stopwatch();

	public:
		void Start();
		void Stop();

		double Elapsed() const;
		int64_t MilliSec() const;
		int64_t MicroSec() const;
		int64_t NanoSec() const;

	private:
		class Impl;
		std::unique_ptr<Impl> m_pImpl;
	};

	class Timer : public Singleton<Timer>
	{
		friend Singleton<Timer>;
	public:
		struct TimeAction
		{
			enum
			{
				eUnlimitedTime = std::numeric_limits<uint32_t>::max(),
			};

			uint32_t nTimerID{ 0 };
			uint32_t nInterval{ 0 };
			uint32_t nLifeTime{ 0 };
			float fIntervalCheckTime{ 0.f };
			float fProcessTime{ 0.f };
			std::function<void(uint32_t, float, float)> funcCallback;

			TimeAction(std::function<void(uint32_t, float, float)> funcCallback, uint32_t nTimerID, uint32_t nInterval, uint32_t nLifeTime);

			bool Update(float fElapsedTime);
		};

	private:
		Timer();
		virtual ~Timer();

	public:
		// 시간 초기화 함수, 루프 이전에 호출
		void Reset();

		// 타이머 시작 또는 재개 시 호출
		void Start();

		// 일시정지
		void Stop();

		// 매 프레임마다 호출
		void Tick();

		// 초단위
		double GetGameTime() const;

		float GetElapsedTime() const;

	public:
		// Callback Function Parameter : nTimerID(uint32_t), fElapsedTime(float), fProcessTime(float)
		void StartTimeAction(std::function<void(uint32_t, float, float)> funcCallback, uint32_t nTimerID, uint32_t nInterval, uint32_t nLifeTime = TimeAction::eUnlimitedTime);
		void StopTimeAction(uint32_t nTimerID);

	private:
		class Impl;
		std::unique_ptr<Impl> m_pImpl;
	};
}