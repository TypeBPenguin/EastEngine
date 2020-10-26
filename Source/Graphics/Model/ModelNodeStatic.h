#pragma once

#include "ModelNode.h"

namespace est
{
	namespace graphics
	{
		class ModelNodeStatic : public ModelNode
		{
		public:
			ModelNodeStatic(LOD emLOD = eLv0);
			ModelNodeStatic(const wchar_t* filePath, BinaryReader& binaryReader);
			virtual ~ModelNodeStatic();

		public:
			virtual void Update(float elapsedTime, const math::Matrix& matParent, ISkeletonInstance* pSkeletonInstance, IMaterialInstance* pMaterialInstance, ITransformInstance* pTransformInstance, bool isModelVisible) const override;

		public:
			virtual IModelNode::Type GetType() const override { return IModelNode::Type::eStatic; }
		};
	}
}