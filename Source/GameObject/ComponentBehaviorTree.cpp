#include "stdafx.h"
#include "ComponentBehaviorTree.h"

namespace eastengine
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

		bool ComponentBehaviorTree::LoadFile(file::Stream& file)
		{
			return true;
		}

		bool ComponentBehaviorTree::SaveFile(file::Stream& file)
		{
			return true;
		}
	}
}