#pragma once

#include "ModelNode.h"

namespace eastengine
{
	namespace graphics
	{
		class ModelNodeStatic : public ModelNode
		{
		public:
			ModelNodeStatic(uint32_t nMaxLod = 0);
			virtual ~ModelNodeStatic();

			virtual void Update(float fElapsedTime, const math::Matrix& matParent, ISkeletonInstance* pSkeletonInstance, IMaterialInstance* pMaterialInstance, bool isModelVisible) const override;
		};
	}
}