#pragma once

#include "CommonLib/Timer.h"

#include "ComponentInterface.h"

namespace EastEngine
{
	namespace GameObject
	{
		class ComponentTimer : public IComponent
		{
		public:
			ComponentTimer(IActor* pOwner);
			virtual ~ComponentTimer();

		public:
			virtual void Update(float fElapsedTime) override;

		public:
			// Callback Function Parameter : nTimerID(uint32_t), fElapsedTime(float), fProcessTime(float)
			void StartTimer(std::function<void(uint32_t, float, float)> funcCallback, uint32_t nTimerID, uint32_t nInterval, uint32_t nLifeTime = Timer::TimeAction::eUnlimitedTime)
			{
				m_listTimeActions.emplace_back(funcCallback, nTimerID, nInterval, nLifeTime);
			}

			void EndTimer(uint32_t nTimerID)
			{
				auto iter = std::find_if(m_listTimeActions.begin(), m_listTimeActions.end(), [nTimerID](const Timer::TimeAction& timeAction)
				{
					return timeAction.nTimerID == nTimerID;
				});

				if (iter != m_listTimeActions.end())
				{
					m_listTimeActions.erase(iter);
				}
			}

		private:
			std::list<Timer::TimeAction> m_listTimeActions;
		};
	}
}