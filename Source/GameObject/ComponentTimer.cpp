#include "stdafx.h"
#include "ComponentTimer.h"

namespace est
{
	namespace gameobject
	{
		ComponentTimer::ComponentTimer(IActor* pOwner)
			: IComponent(pOwner, IComponent::eTimer)
		{
		}

		ComponentTimer::~ComponentTimer()
		{
		}

		void ComponentTimer::Update(float elapsedTime, float lodThreshold)
		{
			m_timeActions.erase(std::remove_if(m_timeActions.begin(), m_timeActions.end(), [elapsedTime](Timer::TimeAction& timeActions)
			{
				if (timeActions.isStopRequest == true)
					return true;

				return timeActions.Update(elapsedTime);
			}), m_timeActions.end());
		}
	}
}