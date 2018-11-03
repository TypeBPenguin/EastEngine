#include "stdafx.h"
#include "Skeleton.h"

#include "GeometryModel.h"

namespace eastengine
{
	namespace graphics
	{
		Skeleton::SkinnedData::SkinnedData(const string::StringID& strName)
			: strName(strName)
		{
		}

		Skeleton::Bone::Bone(const string::StringID& strBoneName, const math::Matrix& matMotionOffset, const math::Matrix& matDefaultMotionData, uint32_t nIndex, uint32_t nParentIndex)
			: m_strBoneName(strBoneName)
			, m_matMotionOffset(matMotionOffset)
			, m_matDefaultMotionData(matDefaultMotionData)
			, m_nIndex(nIndex)
			, m_nParentIndex(nParentIndex)
		{
		}

		Skeleton::Bone::Bone(const Skeleton::Bone& other)
			: m_strBoneName(other.m_strBoneName)
			, m_matMotionOffset(other.m_matMotionOffset)
			, m_matDefaultMotionData(other.m_matDefaultMotionData)
			, m_nIndex(other.m_nIndex)
			, m_nParentIndex(other.m_nParentIndex)
		{
		}

		Skeleton::Bone::Bone(const Skeleton::Bone&& other) noexcept
			: m_strBoneName(std::move(other.m_strBoneName))
			, m_matMotionOffset(std::move(other.m_matMotionOffset))
			, m_matDefaultMotionData(std::move(other.m_matDefaultMotionData))
			, m_nIndex(std::move(other.m_nIndex))
			, m_nParentIndex(std::move(other.m_nParentIndex))
		{
		}

		Skeleton::Skeleton()
		{
		}

		Skeleton::~Skeleton()
		{
		}

		ISkeleton::IBone* Skeleton::GetBone(const string::StringID& strBoneName)
		{
			auto iter = std::find_if(m_vecBones.begin(), m_vecBones.end(), [&](Skeleton::Bone& bone)
			{
				return bone.GetName() == strBoneName;
			});

			if (iter != m_vecBones.end())
				return &(*iter);

			return nullptr;
		}

		void Skeleton::GetSkinnedList(uint32_t nIndex, string::StringID& strSkinnedName_out, const string::StringID** pBoneNames_out, uint32_t& nElementCount_out)
		{
			if (nIndex >= m_vecSkinnedList.size())
			{
				strSkinnedName_out.clear();
				pBoneNames_out = nullptr;
				nElementCount_out = 0;
				return;
			}

			strSkinnedName_out = m_vecSkinnedList[nIndex].strName;
			*pBoneNames_out = &m_vecSkinnedList[nIndex].vecBoneNames.front();
			nElementCount_out = static_cast<uint32_t>(m_vecSkinnedList[nIndex].vecBoneNames.size());
		}

		bool Skeleton::CreateBone(const string::StringID& strBoneName, const math::Matrix& matMotionOffset, const math::Matrix& matDefaultMotionData)
		{
			if (strBoneName.empty() == true)
				return false;

			const uint32_t nNodeIndex = static_cast<uint32_t>(m_vecBones.size());
			m_vecBones.emplace_back(strBoneName, matMotionOffset, matDefaultMotionData, nNodeIndex, eInvalidBoneIndex);

			return true;
		}

		bool Skeleton::CreateBone(const string::StringID& strParentBoneName, const string::StringID& strBoneName, const math::Matrix& matMotionOffset, const math::Matrix& matDefaultMotionData)
		{
			if (strParentBoneName.empty() == true || strBoneName.empty() == true)
				return false;

			ISkeleton::IBone* pParentBone = GetBone(strParentBoneName);
			if (pParentBone == nullptr)
				return false;

			const uint32_t nNodeIndex = static_cast<uint32_t>(m_vecBones.size());
			m_vecBones.emplace_back(strBoneName, matMotionOffset, matDefaultMotionData, nNodeIndex, pParentBone->GetIndex());

			return true;
		}

		void Skeleton::SetSkinnedList(const string::StringID& strSkinnedName, const string::StringID* pBoneNames, size_t nNameCount)
		{
			if (strSkinnedName.empty() == true || pBoneNames == nullptr || nNameCount == 0)
				return;

			auto iter = std::find_if(m_vecSkinnedList.begin(), m_vecSkinnedList.end(), [&](Skeleton::SkinnedData& skinnedData)
			{
				return skinnedData.strName == strSkinnedName;
			});

			if (iter != m_vecSkinnedList.end())
				return;

			m_vecSkinnedList.emplace_back(strSkinnedName);

			std::vector<string::StringID>& vecNames = m_vecSkinnedList.back().vecBoneNames;
			vecNames.reserve(nNameCount);

			vecNames.insert(vecNames.end(), pBoneNames, pBoneNames + nNameCount);
		}

		SkeletonInstance::BoneInstance::BoneInstance(Skeleton::IBone* pBoneOrigin, BoneInstance* pParentBone)
			: m_strBoneName(pBoneOrigin->GetName())
			, m_pParentBone(pParentBone)
			, m_pBoneOrigin(pBoneOrigin)
			, m_matMotionOffset(pBoneOrigin->GetMotionOffsetMatrix())
		{
		}

		void SkeletonInstance::BoneInstance::Update(const math::Matrix& matWorld)
		{
			if (m_pParentBone != nullptr)
			{
				m_matLocal = m_matUserOffset * m_matMotion * m_pParentBone->GetLocalMatrix();
			}
			else
			{
				m_matLocal = m_matUserOffset * m_matMotion;
			}

			m_matSkinning = m_matMotionOffset * m_matLocal;

			m_matGlobal = m_matLocal * matWorld;
		}

		void SkeletonInstance::Initialize(ISkeleton* pSkeleton)
		{
			m_pSkeleton = pSkeleton;

			const uint32_t nBoneCount = pSkeleton->GetBoneCount();
			m_vecBones.reserve(nBoneCount);
			m_umapBones.reserve(nBoneCount);

			for (uint32_t i = 0; i < nBoneCount; ++i)
			{
				Skeleton::IBone* pBoneOrigin = pSkeleton->GetBone(i);
				if (pBoneOrigin->GetParentIndex() != ISkeleton::eInvalidBoneIndex)
				{
					BoneInstance* pParentBone = &m_vecBones[pBoneOrigin->GetParentIndex()];
					m_vecBones.emplace_back(pBoneOrigin, pParentBone);
				}
				else
				{
					m_vecBones.emplace_back(pBoneOrigin, nullptr);
				}

				m_umapBones.emplace(pBoneOrigin->GetName(), &m_vecBones.back());
			}

			CreateSkinnedData(pSkeleton);

			SetIdentity();

			Update(math::Matrix::Identity);
		}

		void SkeletonInstance::Update(const math::Matrix& matWorld)
		{
			std::for_each(m_vecBones.begin(), m_vecBones.end(), [&matWorld](BoneInstance& boneInstance)
			{
				boneInstance.Update(matWorld);
			});
		}

		SkeletonInstance::IBone* SkeletonInstance::GetBone(const string::StringID& strBoneName)
		{
			auto iter = m_umapBones.find(strBoneName);
			if (iter != m_umapBones.end())
				return iter->second;

			return nullptr;
		}

		void SkeletonInstance::GetSkinnedData(const string::StringID& strSkinnedName, const math::Matrix*** pppMatrixList_out, uint32_t& nElementCount_out)
		{
			auto iter = m_umapSkinnendData.find(strSkinnedName);
			if (iter == m_umapSkinnendData.end())
			{
				*pppMatrixList_out = nullptr;
				nElementCount_out = 0;
				return;
			}

			*pppMatrixList_out = &iter->second.front();
			nElementCount_out = static_cast<uint32_t>(iter->second.size());
		}

		void SkeletonInstance::SetIdentity()
		{
			if (m_isDirty == false)
				return;

			std::for_each(m_vecBones.begin(), m_vecBones.end(), [](BoneInstance& boneInstance)
			{
				boneInstance.ClearMotionMatrix();
			});

			m_isDirty = false;
		}

		void SkeletonInstance::CreateSkinnedData(ISkeleton* pSkeleton)
		{
			const uint32_t nCount = pSkeleton->GetSkinnedListCount();
			for (uint32_t i = 0; i < nCount; ++i)
			{
				string::StringID strSkinnedName;
				const string::StringID* pBoneNames = nullptr;
				uint32_t nBoneCount = 0;

				pSkeleton->GetSkinnedList(i, strSkinnedName, &pBoneNames, nBoneCount);

				if (pBoneNames == nullptr || nBoneCount == 0)
					continue;

				auto iter_result = m_umapSkinnendData.emplace(strSkinnedName, std::vector<const math::Matrix*>());
				if (iter_result.second == false)
				{
					assert(false);
				}

				std::vector<const math::Matrix*>& vecSkinnedData = iter_result.first->second;
				vecSkinnedData.resize(nBoneCount);
				for (size_t j = 0; j < nBoneCount; ++j)
				{
					auto iter_find = m_umapBones.find(pBoneNames[j]);
					if (iter_find != m_umapBones.end())
					{
						vecSkinnedData[j] = &iter_find->second->GetSkinningMatrix();
					}
				}
			}
		}
	}
}