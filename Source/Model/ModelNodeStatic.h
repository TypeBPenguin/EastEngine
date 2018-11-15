#pragma once

#include "ModelNode.h"

namespace eastengine
{
	namespace graphics
	{
		class ModelNodeStatic : public ModelNode
		{
		public:
			ModelNodeStatic(LOD emLOD = eLv0);
			virtual ~ModelNodeStatic();

		public:
			virtual void Update(float fElapsedTime, const math::Matrix& matParent, ISkeletonInstance* pSkeletonInstance, IMaterialInstance* pMaterialInstance, bool isModelVisible) const override;

		public:
			virtual IModelNode::Type GetType() const override { return IModelNode::Type::eStatic; }
		};
	}
}