#pragma once

#include "ModelNode.h"

namespace EastEngine
{
	namespace Graphics
	{
		class ModelNodeStatic : public ModelNode
		{
		public:
			ModelNodeStatic(uint32_t nMaxLod = 0);
			virtual ~ModelNodeStatic();

			virtual void Update(float fElapsedTime, const Math::Matrix& matParent, ISkeletonInstance* pSkeletonInstance, IMaterialInstance* pMaterialInstance, bool isModelVisible) override;
		};
	}
}