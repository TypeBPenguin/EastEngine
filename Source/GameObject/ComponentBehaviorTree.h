#pragma once

#include "CommonLib/BehaviorTree.h"
#include "ComponentInterface.h"

namespace eastengine
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

			virtual bool LoadFile(file::Stream& file);
			virtual bool SaveFile(file::Stream& file);

		public:
			BehaviorTree& GetBehaviorTree() { return m_behaviorTree; }

		private:
			BehaviorTree m_behaviorTree;
		};
	}
}