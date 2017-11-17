#include "stdafx.h"
#include "ComponentTimer.h"

namespace EastEngine
{
	namespace GameObject
	{
		ComponentTimer::ComponentTimer(IActor* pOwner)
			: IComponent(pOwner, EmComponent::eTimer)
		{
		}

		ComponentTimer::~ComponentTimer()
		{
			m_listTimeActions.clear();
		}

		void ComponentTimer::Update(float fElapsedTime)
		{
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
	}
}