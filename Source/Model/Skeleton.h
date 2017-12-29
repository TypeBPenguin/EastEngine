#pragma once

#include "CommonLib/plf_colony.h"

#include "ModelInterface.h"

namespace EastEngine
{
	namespace Graphics
	{
		class SkeletonInstance;

		class Skeleton : public ISkeleton
		{
		public:
			class Bone : public ISkeleton::IBone
			{
			public:
				Bone(const String::StringID& strBoneName, const Math::Matrix& matMotionOffset, Bone* pParentBone);
				virtual ~Bone();

			public:
				virtual const String::StringID& GetName() const override { return m_strBoneName; }
				virtual const Math::Matrix& GetMotionOffsetMatrix() const override { return m_matMotionOffset; }

				virtual IBone* GetParent() const override { return m_pParentBone; }

				virtual uint32_t GetChildBoneCount() const override { return m_clnChildBones.size(); }
				virtual IBone* GetChildBone(uint32_t nIndex) const override { return m_vecChildBonesIndexing[nIndex]; }
				virtual IBone* GetChildBone(const String::StringID& strBoneName, bool isFindInAllDepth = false) const override;

				virtual bool IsRootBone() const override { return false; }

			public:
				Bone* AddChildBone(const String::StringID& strBoneName, const Math::Matrix& matMotionOffset);

			private:
				String::StringID m_strBoneName;
				Math::Matrix m_matMotionOffset;

				Bone* m_pParentBone;
				plf::colony<Bone> m_clnChildBones;
				std::vector<Bone*> m_vecChildBonesIndexing;
			};

		private:
			class RootBone : public Bone
			{
			public:
				RootBone();
				virtual ~RootBone() = default;

				virtual bool IsRootBone() const override { return true; }
			};

		public:
			Skeleton();
			virtual ~Skeleton();

		public:
			ISkeletonInstance* CreateInstance();
			void DestroyInstance(ISkeletonInstance** ppSkeleton);

		public:
			virtual IBone* GetRootBone() const override { return m_pRootBone; }

			virtual uint32_t GetBoneCount() const override { return m_vecBones.size(); }
			virtual IBone* GetBone(uint32_t nIndex) const override { return m_vecBones[nIndex]; }
			virtual IBone* GetBone(const String::StringID& strBoneName) const override;

			virtual uint32_t GetSkinnedListCount() const override { return m_vecSkinnedList.size(); }
			virtual void GetSkinnedList(uint32_t nIndex, String::StringID& strSkinnedName_out, const String::StringID** ppBoneNames_out, uint32_t& nElementCount_out) override;

		public:
			Bone* CreateBone(const String::StringID& strBoneName, const Math::Matrix& matMotionOffset);
			Bone* CreateBone(const String::StringID& strParentBoneName, const String::StringID& strBoneName, const Math::Matrix& matMotionOffset);

			void SetSkinnedList(const String::StringID& strSkinnedName, const String::StringID* pBoneNames, uint32_t nNameCount);

		private:
			Bone* CreateChildBone(Bone* pParentBone, const String::StringID& strBoneName, const Math::Matrix& matMotionOffset);

		private:
			Bone* m_pRootBone;

			std::vector<Bone*> m_vecBones;

			struct SkinnedData
			{
				String::StringID strName;
				std::vector<String::StringID> vecBoneNames;

				SkinnedData(const String::StringID& strName);
			};
			std::vector<SkinnedData> m_vecSkinnedList;

			plf::colony<SkeletonInstance> m_clnSkeletonInstance;
		};

		class SkeletonInstance : public ISkeletonInstance
		{
		public:
			class BoneInstance : public ISkeletonInstance::IBone
			{
			public:
				BoneInstance(Skeleton::IBone* pBoneHierarchy, BoneInstance* pParentBone);
				virtual ~BoneInstance();

			public:
				virtual void Update(const Math::Matrix& matParent, bool isPlayingMotion) override;

			public:
				virtual const String::StringID& GetName() override { return m_pBoneHierarchy->GetName(); }

				virtual IBone* GetParent() override { return m_pParentBone; }

				virtual uint32_t GetChildBoneCount() override { return m_vecChildBonesIndexing.size(); }
				virtual IBone* GetChildBone(uint32_t nIndex) override { return m_vecChildBonesIndexing[nIndex]; }
				virtual IBone* GetChildBone(const String::StringID& strBoneName, bool isFindInAllDepth = false) override;

				virtual const Math::Matrix& GetTransform() override { return m_matTransform; }
				virtual void SetMotionMatrix(const Math::Matrix& matrix) override { m_matMotionData = matrix; }
				virtual void ClearMotionMatrix() override { m_matMotionData = Math::Matrix::Identity; }

			public:
				SkeletonInstance::BoneInstance* AddChildBone(Skeleton::IBone* pBoneHierarchy);

			protected:
				Math::Matrix m_matMotionData;
				Math::Matrix m_matTransform;

				BoneInstance* m_pParentBone;
				plf::colony<BoneInstance> m_clnChildBones;
				std::vector<BoneInstance*> m_vecChildBonesIndexing;

				Skeleton::IBone* m_pBoneHierarchy;
			};

		private:
			class RootBone : public BoneInstance
			{
			public:
				RootBone(Skeleton::Bone* pBoneHierarchy);
				virtual ~RootBone();

				virtual void Update(const Math::Matrix& matParent, bool isPlayingMotion);
			};

		public:
			SkeletonInstance(ISkeleton* pSkeleton);
			virtual ~SkeletonInstance();

		public:
			virtual void Update(bool isPlayingMotion) override;

			virtual ISkeleton* GetSkeleton() override { return m_pSkeleton; }

			virtual uint32_t GetBoneCount() const override { return m_vecBones.size(); }
			virtual IBone* GetBone(uint32_t nIndex) const override { return m_vecBones[nIndex]; }
			virtual IBone* GetBone(const String::StringID& strBoneName) const override;

			virtual void GetSkinnedData(const String::StringID& strSkinnedName, const Math::Matrix*** pppMatrixList_out, uint32_t& nElementCount_out) override;
			virtual void SetIdentity() override;
			virtual void SetDirty() override { m_isDirty = true; }

		private:
			void CreateBone(Skeleton::IBone* pBoneHierarchy, BoneInstance* pParentBone);
			void CreateSkinnedData(ISkeleton* pSkeleton);

		private:
			bool m_isDirty;
			ISkeleton* m_pSkeleton;

			RootBone* m_pRootBone;

			std::vector<BoneInstance*> m_vecBones;
			std::unordered_map<String::StringID, BoneInstance*> m_umapBone;

			std::unordered_map<String::StringID, std::vector<const Math::Matrix*>> m_umapSkinnendData;
		};
	}
}