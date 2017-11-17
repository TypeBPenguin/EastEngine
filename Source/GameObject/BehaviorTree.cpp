#include "stdafx.h"
#include "BehaviorTree.h"

namespace StrID
{
	RegisterStringID(Root);
}

namespace EastEngine
{
	namespace GameObject
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