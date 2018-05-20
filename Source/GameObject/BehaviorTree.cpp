#include "stdafx.h"
#include "BehaviorTree.h"

namespace StrID
{
	RegisterStringID(Root);
}

namespace eastengine
{
	namespace gameobject
	{
		BehaviorTree::BehaviorTree()
			: m_pRoot(new Root(StrID::Root))
		{
		}

		BehaviorTree::~BehaviorTree()
		{
		}
	}
}