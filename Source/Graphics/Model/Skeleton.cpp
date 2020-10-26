#include "stdafx.h"
#include "Skeleton.h"

#include "CommonLib/FileStream.h"

#include "GeometryModel.h"

namespace sid
{
	RegisterStringID(NoParent);
};

namespace est
{
	namespace graphics
	{
		Skeleton::SkinnedData::SkinnedData(const string::StringID& strName)
			: name(strName)
		{
		}

		Skeleton::Bone::Bone(const string::StringID& boneName, const math::Matrix& matMotionOffset, const math::Matrix& matDefaultMotionData, uint32_t index, uint32_t parentIndex)
			: m_boneName(boneName)
			, m_index(index)
			, m_parentIndex(parentIndex)
			, m_matMotionOffset(matMotionOffset)
			, m_matDefaultMotionData(matDefaultMotionData)
		{
		}

		Skeleton::Skeleton()
		{
		}

		Skeleton::~Skeleton()
		{
		}

		ISkeleton::IBone* Skeleton::GetBone(const string::StringID& boneName)
		{
			auto iter = std::find_if(m_bones.begin(), m_bones.end(), [&](const Skeleton::Bone& bone)
			{
				return bone.GetName() == boneName;
			});

			if (iter != m_bones.end())
				return &(*iter);

			return nullptr;
		}

		void Skeleton::GetSkinnedList(uint32_t index, string::StringID& skinnedName_out, const string::StringID** pBoneNames_out, uint32_t& nElementCount_out)
		{
			if (index >= m_skinnedList.size())
			{
				skinnedName_out.clear();
				pBoneNames_out = nullptr;
				nElementCount_out = 0;
				return;
			}

			skinnedName_out = m_skinnedList[index].name;
			*pBoneNames_out = &m_skinnedList[index].boneNames.front();
			nElementCount_out = static_cast<uint32_t>(m_skinnedList[index].boneNames.size());
		}

		bool Skeleton::CreateBone(const string::StringID& boneName, const math::Matrix& matMotionOffset, const math::Matrix& matDefaultMotionData)
		{
			if (boneName.empty() == true)
				return false;

			const uint32_t nodeIndex = static_cast<uint32_t>(m_bones.size());
			m_bones.emplace_back(boneName, matMotionOffset, matDefaultMotionData, nodeIndex, eInvalidBoneIndex);

			return true;
		}

		bool Skeleton::CreateBone(const string::StringID& boneName, const string::StringID& parentBoneName, const math::Matrix& matMotionOffset, const math::Matrix& matDefaultMotionData)
		{
			if (parentBoneName.empty() == true || boneName.empty() == true)
				return false;

			ISkeleton::IBone* pParentBone = GetBone(parentBoneName);
			if (pParentBone == nullptr)
				return false;

			const uint32_t parentNodeIndex = pParentBone->GetIndex();
			const uint32_t nodeIndex = static_cast<uint32_t>(m_bones.size());
			m_bones.emplace_back(boneName, matMotionOffset, matDefaultMotionData, nodeIndex, parentNodeIndex);

			return true;
		}

		void Skeleton::SetSkinnedList(const string::StringID& skinnedName, const string::StringID* pBoneNames, size_t nNameCount)
		{
			if (skinnedName.empty() == true || pBoneNames == nullptr || nNameCount == 0)
				return;

			auto iter = std::find_if(m_skinnedList.begin(), m_skinnedList.end(), [&](Skeleton::SkinnedData& skinnedData)
			{
				return skinnedData.name == skinnedName;
			});

			if (iter != m_skinnedList.end())
				return;

			SkinnedData& skinnedData = m_skinnedList.emplace_back(skinnedName);
			skinnedData.boneNames.reserve(nNameCount);
			skinnedData.boneNames.assign(pBoneNames, pBoneNames + nNameCount);
		}

		void Skeleton::LoadFile(BinaryReader& binaryReader)
		{
			const uint32_t boneCount = binaryReader;
			m_bones.resize(boneCount);

			for (uint32_t i = 0; i < boneCount; ++i)
			{
				const string::StringID boneName = binaryReader.ReadString();
				const string::StringID parentBoneName = binaryReader.ReadString();

				const math::Matrix& motionOffsetMatrix = binaryReader;
				const math::Matrix& defaultMotionMatrix = binaryReader;

				if (parentBoneName == sid::NoParent || parentBoneName == sid::None)
				{
					CreateBone(boneName, motionOffsetMatrix, defaultMotionMatrix);
				}
				else
				{
					CreateBone(boneName, parentBoneName, motionOffsetMatrix, defaultMotionMatrix);
				}
			}
		}

		SkeletonInstance::BoneInstance::BoneInstance(const Skeleton::IBone* pOriginBone, const BoneInstance* pParentBone)
			: m_pOriginBone(pOriginBone)
			, m_index(pOriginBone->GetIndex())
			, m_boneName(pOriginBone->GetName())
			, m_pParentBone(pParentBone)
		{
		}

		void SkeletonInstance::BoneInstance::Update(const math::Matrix& matWorld, const math::Matrix* pUserOffsetMatrix)
		{
			if (pUserOffsetMatrix != nullptr)
			{
				if (m_pParentBone != nullptr)
				{
					m_matLocal = *pUserOffsetMatrix * m_matMotion * m_pParentBone->m_matLocal;
				}
				else
				{
					m_matLocal = *pUserOffsetMatrix * m_matMotion;
				}
			}
			else
			{
				if (m_pParentBone != nullptr)
				{
					m_matLocal = m_matMotion * m_pParentBone->m_matLocal;
				}
				else
				{
					m_matLocal = m_matMotion;
				}
			}

			m_matSkinning = m_pOriginBone->GetMotionOffsetMatrix() * m_matLocal;

			m_matGlobal = m_matLocal * matWorld;
		}

		void SkeletonInstance::Initialize(ISkeleton* pSkeleton)
		{
			m_pSkeleton = pSkeleton;

			const uint32_t boneCount = pSkeleton->GetBoneCount();
			m_bones.reserve(boneCount);
			m_rmapBones.reserve(boneCount);

			for (uint32_t i = 0; i < boneCount; ++i)
			{
				Skeleton::IBone* pBoneOrigin = pSkeleton->GetBone(i);
				if (pBoneOrigin->GetParentIndex() != ISkeleton::eInvalidBoneIndex)
				{
					BoneInstance* pParentBone = &m_bones[pBoneOrigin->GetParentIndex()];
					m_bones.emplace_back(pBoneOrigin, pParentBone);
				}
				else
				{
					m_bones.emplace_back(pBoneOrigin, nullptr);
				}

				m_rmapBones.emplace(pBoneOrigin->GetName(), &m_bones.back());
			}

			CreateSkinnedData(pSkeleton);

			SetIdentity();

			Update(math::Matrix::Identity);
		}

		void SkeletonInstance::Update(const math::Matrix& matWorld)
		{
			std::for_each(m_bones.begin(), m_bones.end(), [&](BoneInstance& boneInstance)
			{
				boneInstance.Update(matWorld, GetUserOffsetMatrix(boneInstance.GetName()));
			});
		}

		SkeletonInstance::IBone* SkeletonInstance::GetBone(const string::StringID& boneName)
		{
			auto iter = m_rmapBones.find(boneName);
			if (iter != m_rmapBones.end())
				return iter->second;

			return nullptr;
		}

		void SkeletonInstance::GetSkinnedData(const string::StringID& skinnedName, const math::Matrix* const** pppMatrixList_out, uint32_t& nElementCount_out)
		{
			auto iter = m_rmapSkinnendData.find(skinnedName);
			if (iter == m_rmapSkinnendData.end())
			{
				*pppMatrixList_out = nullptr;
				nElementCount_out = 0;
				return;
			}

			*pppMatrixList_out = iter->second.data();
			nElementCount_out = static_cast<uint32_t>(iter->second.size());
		}

		void SkeletonInstance::SetIdentity()
		{
			if (m_isDirty == false)
				return;

			std::for_each(m_bones.begin(), m_bones.end(), [](BoneInstance& boneInstance)
			{
				boneInstance.ClearMotionMatrix();
			});

			m_isDirty = false;
		}

		const math::Matrix* SkeletonInstance::GetUserOffsetMatrix(const string::StringID& boneName) const
		{
			auto iter = m_rmapUserOffsetMatrix.find(boneName);
			if (iter != m_rmapUserOffsetMatrix.end())
				return &iter->second;

			return nullptr;
		}

		void SkeletonInstance::SetUserOffsetMatrix(const string::StringID& boneName, const math::Matrix& matrix)
		{
			auto iter = m_rmapUserOffsetMatrix.find(boneName);
			if (iter != m_rmapUserOffsetMatrix.end())
			{
				iter.value() = matrix;
			}
			else
			{
				m_rmapUserOffsetMatrix.emplace(boneName, matrix);
			}
		}

		void SkeletonInstance::CreateSkinnedData(ISkeleton* pSkeleton)
		{
			const uint32_t count = pSkeleton->GetSkinnedListCount();
			for (uint32_t i = 0; i < count; ++i)
			{
				string::StringID skinnedName;
				const string::StringID* pBoneNames = nullptr;
				uint32_t boneCount = 0;

				pSkeleton->GetSkinnedList(i, skinnedName, &pBoneNames, boneCount);

				if (pBoneNames == nullptr || boneCount == 0)
					continue;

				auto iter_result = m_rmapSkinnendData.emplace(skinnedName, std::vector<const math::Matrix*>());
				if (iter_result.second == false)
				{
					assert(false);
				}

				std::vector<const math::Matrix*>& skinnedData = iter_result.first.value();
				skinnedData.resize(boneCount);
				for (size_t j = 0; j < boneCount; ++j)
				{
					auto iter_find = m_rmapBones.find(pBoneNames[j]);
					if (iter_find != m_rmapBones.end())
					{
						skinnedData[j] = &iter_find->second->GetSkinningMatrix();
					}
				}
			}
		}
	}
}