#pragma once

#include "CommonLib/Timer.h"
#include "ComponentInterface.h"

namespace est
{
	namespace gameobject
	{
		class ComponentTimer : public IComponent
		{
		public:
			ComponentTimer(IActor* pOwner);
			virtual ~ComponentTimer();

		public:
			virtual void Update(float elapsedTime) override;

		public:
			// Callback Function Parameter : nTimerID(uint32_t), elapsedTime(float), fProcessTime(float)
			void StartTimeAction(std::function<void(uint32_t, float, float)> funcCallback, uint32_t timerID, uint32_t interval, uint32_t lifeTime= Timer::TimeAction::eUnlimitedTime)
			{
				m_timeActions.emplace_back(funcCallback, timerID, interval, lifeTime);
			}

			void StopTimeAction(uint32_t timerID)
			{
				auto iter = std::find_if(m_timeActions.begin(), m_timeActions.end(), [timerID](const Timer::TimeAction& timeAction)
				{
					return timeAction.timerID == timerID;
				});

				if (iter != m_timeActions.end())
				{
					iter->isStopRequest = true;
				}
			}

		private:
			std::vector<Timer::TimeAction> m_timeActions;
		};
	}
}