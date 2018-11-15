#pragma once

#include "ModelNode.h"

namespace eastengine
{
	namespace graphics
	{
		class ModelNodeSkinned : public ModelNode
		{
		public:
			ModelNodeSkinned(LOD emLOD = eLv0);
			virtual ~ModelNodeSkinned();

		public:
			virtual void Update(float fElapsedTime, const math::Matrix& matParent, ISkeletonInstance* pSkeletonInstance, IMaterialInstance* pMaterialInstance, bool isModelVisible) const override;

		public:
			virtual IModelNode::Type GetType() const override { return IModelNode::Type::eSkinned; }

		public:
			uint32_t GetBoneCount() const { return static_cast<uint32_t>(m_vecBoneName.size()); }
			const string::StringID& GetBoneName(uint32_t index) { assert(index <= GetBoneCount()); return m_vecBoneName[index]; }

			void SetBoneNameList(const std::vector<string::StringID>& vecBoneName) { m_vecBoneName.clear(); m_vecBoneName.assign(vecBoneName.begin(), vecBoneName.end()); }

		private:
			std::vector<string::StringID> m_vecBoneName;
		};
	}
}