#include "stdafx.h"
#include "ComponentBehaviorTree.h"

namespace est
{
	namespace gameobject
	{
		ComponentBehaviorTree::ComponentBehaviorTree(IActor* pOwner)
			: IComponent(pOwner, IComponent::eBehaviorTree)
		{
		}

		ComponentBehaviorTree::~ComponentBehaviorTree()
		{
		}

		void ComponentBehaviorTree::Update(float elapsedTime)
		{
			m_behaviorTree.Run(elapsedTime);
		}
	}
}