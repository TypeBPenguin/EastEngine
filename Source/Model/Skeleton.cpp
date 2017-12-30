#include "stdafx.h"
#include "Skeleton.h"

#include "DirectX/DebugUtil.h"
#include "Renderer/RendererManager.h"

namespace StrID
{
	RegisterStringID(EastEngine_RootBone);
}

namespace EastEngine
{
	namespace Graphics
	{
		Skeleton::SkinnedData::SkinnedData(const String::StringID& strName)
			: strName(strName)
		{
		}

		Skeleton::Bone::Bone(const String::StringID& strBoneName, const Math::Matrix& matMotionOffset, const Math::Matrix& matDefaultMotionData, Bone* pParentBone)
			: m_strBoneName(strBoneName)
			, m_matMotionOffset(matMotionOffset)
			, m_matDefaultMotionData(matDefaultMotionData)
			, m_pParentBone(pParentBone)
		{
			m_clnChildBones.reserve(32);
			m_vecChildBonesIndexing.reserve(32);
		}

		Skeleton::Bone::~Bone()
		{
			m_vecChildBonesIndexing.clear();
			m_clnChildBones.clear();

			m_pParentBone = nullptr;
		}

		ISkeleton::IBone* Skeleton::Bone::GetChildBone(const String::StringID& strBoneName, bool isFindInAllDepth) const
		{
			if (isFindInAllDepth == true)
			{
				auto iter = std::find_if(m_clnChildBones.begin(), m_clnChildBones.end(), [&strBoneName](const Bone& bone)
				{
					return bone.GetName() == strBoneName;
				});

				if (iter != m_clnChildBones.end())
					return &(*iter);
			}
			else
			{
				for (auto& childBone : m_clnChildBones)
				{
					if (childBone.GetName() == strBoneName)
						return &childBone;

					ISkeleton::IBone* pBone = childBone.GetChildBone(strBoneName, isFindInAllDepth);
					if (pBone != nullptr)
						return pBone;
				}
			}

			return nullptr;
		}

		Skeleton::Bone* Skeleton::Bone::AddChildBone(const String::StringID& strBoneName, const Math::Matrix& matMotionOffset, const Math::Matrix& matDefaultMotionData)
		{
			auto iter_result = m_clnChildBones.emplace(strBoneName, matMotionOffset, matDefaultMotionData, this);
			Bone* pNewBone = &(*iter_result);

			m_vecChildBonesIndexing.emplace_back(pNewBone);

			return pNewBone;
		}

		Skeleton::RootBone::RootBone()
			: Bone(StrID::EastEngine_RootBone, Math::Matrix::Identity, Math::Matrix::Identity, nullptr)
		{
		}

		Skeleton::Skeleton()
			: m_pRootBone(new RootBone)
		{
		}

		Skeleton::~Skeleton()
		{
			m_clnSkeletonInstance.clear();

			SafeDelete(m_pRootBone);
			m_vecBones.clear();
			m_vecSkinnedList.clear();
		}

		ISkeletonInstance* Skeleton::CreateInstance()
		{
			auto iter_result = m_clnSkeletonInstance.emplace(this);

			return &(*iter_result);
		}

		void Skeleton::DestroyInstance(ISkeletonInstance** ppSkeletonInstance)
		{
			if (ppSkeletonInstance == nullptr || *ppSkeletonInstance == nullptr)
				return;

			SkeletonInstance* pInstance = static_cast<SkeletonInstance*>(*ppSkeletonInstance);

			auto iter = std::find_if(m_clnSkeletonInstance.begin(), m_clnSkeletonInstance.end(), [&pInstance](const SkeletonInstance& instance)
			{
				return &instance == pInstance;
			});

			if (iter == m_clnSkeletonInstance.end())
				return;

			m_clnSkeletonInstance.erase(iter);
			*ppSkeletonInstance = nullptr;
		}

		ISkeleton::IBone* Skeleton::GetBone(const String::StringID& strBoneName) const
		{
			auto iter = std::find_if(m_vecBones.begin(), m_vecBones.end(), [&](Skeleton::Bone* pBone)
			{
				return pBone->GetName() == strBoneName;
			});

			if (iter != m_vecBones.end())
				return *iter;

			return nullptr;
		}

		void Skeleton::GetSkinnedList(size_t nIndex, String::StringID& strSkinnedName_out, const String::StringID** pBoneNames_out, size_t& nElementCount_out)
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
			nElementCount_out = m_vecSkinnedList[nIndex].vecBoneNames.size();
		}

		Skeleton::Bone* Skeleton::CreateBone(const String::StringID& strBoneName, const Math::Matrix& matMotionOffset, const Math::Matrix& matDefaultMotionData)
		{
			if (strBoneName.empty() == true)
				return nullptr;

			ISkeleton::IBone* pChildBone = m_pRootBone->GetChildBone(strBoneName);
			if (pChildBone != nullptr)
				return static_cast<Bone*>(pChildBone);

			return CreateChildBone(m_pRootBone, strBoneName, matMotionOffset, matDefaultMotionData);
		}

		Skeleton::Bone* Skeleton::CreateBone(const String::StringID& strParentBoneName, const String::StringID& strBoneName, const Math::Matrix& matMotionOffset, const Math::Matrix& matDefaultMotionData)
		{
			if (strParentBoneName.empty() == true || strBoneName.empty() == true)
				return nullptr;

			ISkeleton::IBone* pBone = GetBone(strParentBoneName);
			if (pBone == nullptr)
				return nullptr;

			Bone* pParentBone = static_cast<Bone*>(pBone);

			ISkeleton::IBone* pChildBone = pParentBone->GetChildBone(strBoneName);
			if (pChildBone != nullptr)
				return static_cast<Bone*>(pChildBone);

			return CreateChildBone(pParentBone, strBoneName, matMotionOffset, matDefaultMotionData);
		}

		void Skeleton::SetSkinnedList(const String::StringID& strSkinnedName, const String::StringID* pBoneNames, size_t nNameCount)
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

			std::vector<String::StringID>& vecNames = m_vecSkinnedList.back().vecBoneNames;
			vecNames.reserve(nNameCount);

			vecNames.insert(vecNames.end(), pBoneNames, pBoneNames + nNameCount);
		}

		Skeleton::Bone* Skeleton::CreateChildBone(Bone* pParentBone, const String::StringID& strBoneName, const Math::Matrix& matMotionOffset, const Math::Matrix& matDefaultMotionData)
		{
			if (pParentBone == nullptr || strBoneName.empty() == true)
				return nullptr;

			Bone* pChildBone = pParentBone->AddChildBone(strBoneName, matMotionOffset, matDefaultMotionData);
			if (pChildBone == nullptr)
				return nullptr;

			m_vecBones.emplace_back(pChildBone);

			return pChildBone;
		}

		SkeletonInstance::BoneInstance::BoneInstance(Skeleton::IBone* pBoneHierarchy, BoneInstance* pParentBone)
			: m_pParentBone(pParentBone)
			, m_pBoneHierarchy(pBoneHierarchy)
		{
		}

		SkeletonInstance::BoneInstance::~BoneInstance()
		{
			m_vecChildBonesIndexing.clear();
			m_clnChildBones.clear();

			m_pParentBone = nullptr;
			m_pBoneHierarchy = nullptr;
		}

		void SkeletonInstance::BoneInstance::Update(const Math::Matrix& matParent)
		{
			m_matTransform = m_matMotionData * matParent;
			m_matMotionTransform = m_pBoneHierarchy->GetMotionOffsetMatrix() * m_matTransform;

			std::for_each(m_clnChildBones.begin(), m_clnChildBones.end(), [&](BoneInstance& childBone)
			{
				childBone.Update(m_matTransform);
			});
		}

		SkeletonInstance::IBone* SkeletonInstance::BoneInstance::GetChildBone(const String::StringID& strBoneName, bool isFindInAllDepth) const
		{
			auto iter = std::find_if(m_clnChildBones.begin(), m_clnChildBones.end(), [&](const BoneInstance& bone)
			{
				return bone.GetName() == strBoneName;
			});

			if (iter != m_clnChildBones.end())
				return &(*iter);

			return nullptr;
		}

		SkeletonInstance::BoneInstance* SkeletonInstance::BoneInstance::AddChildBone(Skeleton::IBone* pBoneHierarchy)
		{
			auto iter_result = m_clnChildBones.emplace(pBoneHierarchy, this);
			BoneInstance* pNewBone = &(*iter_result);

			m_vecChildBonesIndexing.emplace_back(pNewBone);

			return pNewBone;
		}

		SkeletonInstance::RootBone::RootBone(Skeleton::Bone* pBoneHierarchy)
			: BoneInstance(pBoneHierarchy, nullptr)
		{
		}

		SkeletonInstance::RootBone::~RootBone()
		{
		}

		void SkeletonInstance::RootBone::Update(const Math::Matrix& matParent)
		{
			std::for_each(m_clnChildBones.begin(), m_clnChildBones.end(), [&](BoneInstance& boneInstance)
			{
				boneInstance.Update(matParent);
			});
		}

		SkeletonInstance::SkeletonInstance(ISkeleton* pSkeleton)
			: m_pSkeleton(pSkeleton)
			, m_isDirty(false)
			, m_pRootBone(new RootBone(static_cast<Skeleton::Bone*>(pSkeleton->GetRootBone())))
		{
			m_vecBones.reserve(pSkeleton->GetBoneCount());
			m_umapBone.reserve(pSkeleton->GetBoneCount());

			CreateBone(pSkeleton->GetRootBone(), m_pRootBone);

			Update();

			CreateSkinnedData(pSkeleton);
		}

		SkeletonInstance::~SkeletonInstance()
		{
			m_pSkeleton = nullptr;

			SafeDelete(m_pRootBone);
			m_vecBones.clear();
			m_umapBone.clear();
			m_umapSkinnendData.clear();
		}

		void SkeletonInstance::Update()
		{
			if (m_vecBones.empty() == false)
			{
				m_pRootBone->Update(Math::Matrix::Identity);
			}
		}

		SkeletonInstance::IBone* SkeletonInstance::GetBone(const String::StringID& strBoneName) const
		{
			auto iter = m_umapBone.find(strBoneName);
			if (iter != m_umapBone.end())
				return iter->second;

			return nullptr;
		}

		void SkeletonInstance::GetSkinnedData(const String::StringID& strSkinnedName, const Math::Matrix*** pppMatrixList_out, size_t& nElementCount_out)
		{
			auto iter = m_umapSkinnendData.find(strSkinnedName);
			if (iter == m_umapSkinnendData.end())
			{
				*pppMatrixList_out = nullptr;
				nElementCount_out = 0;
				return;
			}

			*pppMatrixList_out = &iter->second.front();
			nElementCount_out = iter->second.size();
		}

		void SkeletonInstance::SetIdentity()
		{
			if (m_isDirty == false)
				return;

			std::for_each(m_vecBones.begin(), m_vecBones.end(), [](BoneInstance* pBone)
			{
				pBone->ClearMotionData();
			});

			m_isDirty = false;
		}

		void SkeletonInstance::CreateBone(Skeleton::IBone* pBoneHierarchy, BoneInstance* pParentBone)
		{
			uint32_t nChildCount = pBoneHierarchy->GetChildBoneCount();
			for (uint32_t i = 0; i < nChildCount; ++i)
			{
				Skeleton::IBone* pChildBoneHierarchy = pBoneHierarchy->GetChildBone(i);

				BoneInstance* pChildBone = pParentBone->AddChildBone(pChildBoneHierarchy);
				m_vecBones.emplace_back(pChildBone);
				m_umapBone.emplace(pChildBone->GetName(), pChildBone);

				CreateBone(pChildBoneHierarchy, pChildBone);
			}
		}

		void SkeletonInstance::CreateSkinnedData(ISkeleton* pSkeleton)
		{
			const size_t nCount = pSkeleton->GetSkinnedListCount();
			for (size_t i = 0; i < nCount; ++i)
			{
				String::StringID strSkinnedName;
				const String::StringID* pBoneNames = nullptr;
				size_t nBoneCount = 0;

				pSkeleton->GetSkinnedList(i, strSkinnedName, &pBoneNames, nBoneCount);

				if (pBoneNames == nullptr || nBoneCount == 0)
					continue;

				auto iter_result = m_umapSkinnendData.emplace(strSkinnedName, std::vector<const Math::Matrix*>());
				if (iter_result.second == false)
				{
					assert(false);
				}

				std::vector<const Math::Matrix*>& vecSkinnedData = iter_result.first->second;
				vecSkinnedData.resize(nBoneCount);
				for (size_t j = 0; j < nBoneCount; ++j)
				{
					auto iter_find = m_umapBone.find(pBoneNames[j]);
					if (iter_find != m_umapBone.end())
					{
						vecSkinnedData[j] = &iter_find->second->GetMotionTransform();
					}
				}
			}
		}
	}
}