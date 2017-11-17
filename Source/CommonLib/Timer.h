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
		void Reset();	// �ð� �ʱ�ȭ �Լ�, ���� ������ ȣ��
		void Start();	// Ÿ�̸� ���� �Ǵ� �簳 �� ȣ��
		void Stop();	// �Ͻ�����
		void Tick();	// �� �����Ӹ��� ȣ��

		// �ʴ���
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