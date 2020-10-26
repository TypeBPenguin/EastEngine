#pragma once

#include "CommonLib/BehaviorTree.h"
#include "ComponentInterface.h"

namespace est
{
	namespace gameobject
	{
		class ComponentBehaviorTree : public IComponent
		{
		public:
			ComponentBehaviorTree(IActor* pOwner);
			virtual ~ComponentBehaviorTree();

		public:
			virtual void Update(float elapsedTime) override;

		public:
			BehaviorTree& GetBehaviorTree() { return m_behaviorTree; }

		private:
			BehaviorTree m_behaviorTree;
		};
	}
}