#include "stdafx.h"
#include "Skeleton.h"

#include "../DirectX/DebugUtil.h"
#include "../Renderer/RendererManager.h"

namespace StrID
{
	RegisterStringID(EastEngine_RootBone);
}

namespace EastEngine
{
	namespace Graphics
	{

		static boost::object_pool<Skeleton::Bone> s_poolBone;

		static boost::object_pool<SkeletonInstance> s_poolSkeletonInstance;
		static boost::object_pool<SkeletonInstance::BoneInstance> s_poolBoneInstance;

		Skeleton::SkinnedData::SkinnedData(const String::StringID& strName)
			: strName(strName)
		{
		}

		Skeleton::Bone::Bone(Bone* pParentBone, const Math::Matrix& matOffset, const Math::Matrix& matTransformation)
			: m_pParentBone(pParentBone)
			, m_matOffset(matOffset)
			, m_matTransformation(matTransformation)
		{
		}

		Skeleton::Bone::~Bone()
		{
			std::for_each(m_vecChildBone.begin(), m_vecChildBone.end(), [](Bone* pBone)
			{
				s_poolBone.destroy(pBone);
			});
			m_vecChildBone.clear();

			m_pParentBone = nullptr;
		}

		ISkeleton::IBone* Skeleton::Bone::GetChildBone(const String::StringID& strBoneName, bool isFindInAllDepth)
		{
			if (isFindInAllDepth == true)
			{
				for (auto& pChildBone : m_vecChildBone)
				{
					if (pChildBone->GetName() == strBoneName)
						return pChildBone;
				}
			}
			else
			{
				for (auto& pChildBone : m_vecChildBone)
				{
					if (pChildBone->GetName() == strBoneName)
						return pChildBone;

					ISkeleton::IBone* pBone = pChildBone->GetChildBone(strBoneName, isFindInAllDepth);
					if (pBone != nullptr)
						return pBone;
				}
			}

			return nullptr;
		}

		Skeleton::Bone* Skeleton::Bone::AddChildBone(const String::StringID& strBoneName, const Math::Matrix& matOffset, const Math::Matrix& matTransformation)
		{
			Bone* pNewBone = s_poolBone.construct(this, matOffset, matTransformation);
			pNewBone->m_strBoneName = strBoneName;

			m_vecChildBone.emplace_back(pNewBone);

			return pNewBone;
		}

		Skeleton::Skeleton()
			: m_pRootBone(new RootBone)
		{
		}

		Skeleton::~Skeleton()
		{
			for (auto pSkeletonInstance : m_setSkeletonInstance)
			{
				s_poolSkeletonInstance.destroy(pSkeletonInstance);
			}
			m_setSkeletonInstance.clear();

			SafeDelete(m_pRootBone);
			m_vecBones.clear();
			m_vecSkinnedList.clear();
		}

		ISkeletonInstance* Skeleton::CreateInstance()
		{
			auto iter_result = m_setSkeletonInstance.emplace(s_poolSkeletonInstance.construct(this));
			if (iter_result.second == false)
				return nullptr;

			return *iter_result.first;
		}

		void Skeleton::DestroyInstance(ISkeletonInstance** ppSkeletonInstance)
		{
			if (ppSkeletonInstance == nullptr || *ppSkeletonInstance == nullptr)
				return;

			SkeletonInstance* pInstance = static_cast<SkeletonInstance*>(*ppSkeletonInstance);

			auto iter = m_setSkeletonInstance.find(pInstance);
			if (iter == m_setSkeletonInstance.end())
				return;

			s_poolSkeletonInstance.destroy(pInstance);

			*ppSkeletonInstance = nullptr;

			m_setSkeletonInstance.erase(iter);
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

		void Skeleton::GetSkinnedList(uint32_t nIndex, String::StringID& strSkinnedName_out, const String::StringID** pBoneNames_out, uint32_t& nElementCount_out)
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

		Skeleton::Bone* Skeleton::CreateBone(const String::StringID& strBoneName, const Math::Matrix& matOffset, const Math::Matrix& matTransformation)
		{
			if (strBoneName.empty() == true)
				return nullptr;

			ISkeleton::IBone* pChildBone = m_pRootBone->GetChildBone(strBoneName);
			if (pChildBone != nullptr)
				return static_cast<Bone*>(pChildBone);

			return CreateChildBone(m_pRootBone, strBoneName, matOffset, matTransformation);
		}

		Skeleton::Bone* Skeleton::CreateBone(const String::StringID& strParentBoneName, const String::StringID& strBoneName, const Math::Matrix& matOffset, const Math::Matrix& matTransformation)
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

			return CreateChildBone(pParentBone, strBoneName, matOffset, matTransformation);
		}

		void Skeleton::SetSkinnedList(const String::StringID& strSkinnedName, const String::StringID* pBoneNames, uint32_t nNameCount)
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

		Skeleton::Bone* Skeleton::CreateChildBone(Bone* pParentBone, const String::StringID& strBoneName, const Math::Matrix& matOffset, const Math::Matrix& matTransformation)
		{
			if (pParentBone == nullptr || strBoneName.empty() == true)
				return nullptr;

			Bone* pChildBone = pParentBone->AddChildBone(strBoneName, matOffset, matTransformation);
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
			std::for_each(m_vecChildBone.begin(), m_vecChildBone.end(), [](BoneInstance* pBone)
			{
				s_poolBoneInstance.destroy(pBone);
			});
			m_vecChildBone.clear();

			m_pParentBone = nullptr;
			m_pBoneHierarchy = nullptr;
		}

		void SkeletonInstance::BoneInstance::Update(const Math::Matrix& matParent, bool isPlayingMotion)
		{
			Math::Matrix matBoneTransforms;

			if (isPlayingMotion == true)
			{
				matBoneTransforms = m_matMotionData * matParent;
				m_matTransform = m_pBoneHierarchy->GetMotionOffsetMatrix() * matBoneTransforms;
			}
			else
			{
				matBoneTransforms = matParent;
				m_matTransform = m_pBoneHierarchy->GetTransformation() * matBoneTransforms;
			}

			for (auto& pChildBone : m_vecChildBone)
			{
				pChildBone->Update(matBoneTransforms, isPlayingMotion);
			}
		}

		SkeletonInstance::IBone* SkeletonInstance::BoneInstance::GetChildBone(const String::StringID& strBoneName, bool isFindInAllDepth)
		{
			auto iter = std::find_if(m_vecChildBone.begin(), m_vecChildBone.end(), [&](BoneInstance* pBone)
			{
				return pBone->GetName() == strBoneName;
			});

			if (iter != m_vecChildBone.end())
				return *iter;

			return nullptr;
		}

		SkeletonInstance::BoneInstance* SkeletonInstance::BoneInstance::AddChildBone(Skeleton::IBone* pBoneHierarchy)
		{
			BoneInstance* pNewBone = s_poolBoneInstance.construct(pBoneHierarchy, this);

			m_vecChildBone.emplace_back(pNewBone);

			return pNewBone;
		}

		SkeletonInstance::RootBone::RootBone(Skeleton::Bone* pBoneHierarchy)
			: BoneInstance(pBoneHierarchy, nullptr)
		{
		}

		SkeletonInstance::RootBone::~RootBone()
		{
		}

		void SkeletonInstance::RootBone::Update(const Math::Matrix& matParent, bool isPlayingMotion)
		{
			for (auto& pChildBone : m_vecChildBone)
			{
				pChildBone->Update(matParent, isPlayingMotion);
			}
		}

		SkeletonInstance::SkeletonInstance(ISkeleton* pSkeleton)
			: m_pSkeleton(pSkeleton)
			, m_pRootBone(new RootBone(static_cast<Skeleton::Bone*>(pSkeleton->GetRootBone())))
		{
			m_vecBones.reserve(pSkeleton->GetBoneCount());
			m_umapBone.reserve(pSkeleton->GetBoneCount());

			CreateBone(pSkeleton->GetRootBone(), m_pRootBone);

			Update(false);

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
		
		void SkeletonInstance::Update(bool isPlayingMotion)
		{
			if (m_umapBone.empty() == false)
			{
				m_pRootBone->Update(Math::Matrix::Identity, isPlayingMotion);
			}
		}

		SkeletonInstance::IBone* SkeletonInstance::GetBone(const String::StringID& strBoneName) const
		{
			auto iter = m_umapBone.find(strBoneName);
			if (iter != m_umapBone.end())
				return iter->second;

			return nullptr;
		}

		void SkeletonInstance::GetSkinnedData(const String::StringID& strSkinnedName, const Math::Matrix*** pppMatrixList_out, uint32_t& nElementCount_out)
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
			uint32_t nCount = pSkeleton->GetSkinnedListCount();
			for (uint32_t i = 0; i < nCount; ++i)
			{
				String::StringID strSkinnedName;
				const String::StringID* pBoneNames = nullptr;
				uint32_t nBoneCount = 0;

				pSkeleton->GetSkinnedList(i, strSkinnedName, &pBoneNames, nBoneCount);

				if (pBoneNames == nullptr || nBoneCount == 0)
					continue;

				auto iter_result = m_umapSkinnendData.emplace(strSkinnedName, std::vector<const Math::Matrix*>());
				if (iter_result.second == false)
				{
					assert(false);
				}
				
				auto& vecSkinnedData = iter_result.first->second;
				vecSkinnedData.resize(nBoneCount);
				for (uint32_t j = 0; j < nBoneCount; ++j)
				{
					auto iter_find = m_umapBone.find(pBoneNames[j]);
					if (iter_find != m_umapBone.end())
					{
						vecSkinnedData[j] = &iter_find->second->GetTransform();
					}
				}
			}
		}
	}
}