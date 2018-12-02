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

			uint32_t timerID{ 0 };
			uint32_t interval{ 0 };
			uint32_t lifeTime{ 0 };
			float intervalCheckTime{ 0.f };
			float processTime{ 0.f };
			std::function<void(uint32_t, float, float)> funcCallback;
			bool isStopRequest{ false };

			TimeAction(std::function<void(uint32_t, float, float)> funcCallback, uint32_t timerID, uint32_t interval, uint32_t lifeTime);
			TimeAction(TimeAction&& source) noexcept;

			TimeAction& operator = (TimeAction&& source) noexcept;

			bool Update(float elapsedTime);
		};

	private:
		Timer();
		virtual ~Timer();

	public:
		// �ð� �ʱ�ȭ �Լ�, ���� ������ ȣ��
		void Reset();

		// Ÿ�̸� ���� �Ǵ� �簳 �� ȣ��
		void Start();

		// �Ͻ�����
		void Stop();

		// �� �����Ӹ��� ȣ��
		void Tick();

		// �ʴ���
		double GetGameTime() const;

		float GetElapsedTime() const;

	public:
		// Callback Function Parameter : timerID(uint32_t), elapsedTime(float), processTime(float)
		void StartTimeAction(std::function<void(uint32_t, float, float)> funcCallback, uint32_t timerID, uint32_t interval, uint32_t lifeTime = TimeAction::eUnlimitedTime);
		void StopTimeAction(uint32_t timerID);

	private:
		class Impl;
		std::unique_ptr<Impl> m_pImpl;
	};
}