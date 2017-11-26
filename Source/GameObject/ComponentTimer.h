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
			void StartTimer(std::function<void(uint32_t, float, float)> funcCallback, uint32_t nTimerID, uint32_t nInterval, uint32_t nLifeTime = Timer::TimeAction::eUnlimitedTime)
			{
				m_listTimeActions.emplace_back(funcCallback, nTimerID, nInterval, nLifeTime);
			}

		private:
			std::function<void(float, float)> m_funcCallback;
			std::list<Timer::TimeAction> m_listTimeActions;
		};
	}
}