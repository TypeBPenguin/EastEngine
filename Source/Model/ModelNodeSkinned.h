#pragma once

#include "ModelNode.h"

namespace EastEngine
{
	namespace Graphics
	{
		class ModelNodeSkinned : public ModelNode
		{
		public:
			ModelNodeSkinned(uint32_t nLodMax = 0);
			virtual ~ModelNodeSkinned();

			virtual void Update(float fElapsedTime, const Math::Matrix& matParent, ISkeletonInstance* pSkeletonInstance, IMaterialInstance* pMaterialInstance, bool isModelVisible) override;

		public:
			uint32_t GetBoneCount() const { return m_vecBoneName.size(); }
			const String::StringID& GetBoneName(uint32_t nIndex) { return m_vecBoneName[nIndex]; }

			void SetBoneNameList(const std::vector<String::StringID>& vecBoneName) { m_vecBoneName.clear(); m_vecBoneName.assign(vecBoneName.begin(), vecBoneName.end()); }

		private:
			std::vector<String::StringID> m_vecBoneName;
		};
	}
}