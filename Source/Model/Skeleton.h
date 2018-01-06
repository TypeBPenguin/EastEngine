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
				Bone(const String::StringID& strBoneName, const Math::Matrix& matMotionOffset, const Math::Matrix& matDefaultMotionData, Bone* pParentBone);
				virtual ~Bone();

			public:
				virtual const String::StringID& GetName() const override { return m_strBoneName; }
				virtual const Math::Matrix& GetMotionOffsetMatrix() const override { return m_matMotionOffset; }
				virtual const Math::Matrix& GetDefaultMotionData() const override { return m_matDefaultMotionData; }

				virtual IBone* GetParent() const override { return m_pParentBone; }

				virtual size_t GetChildBoneCount() const override { return m_clnChildBones.size(); }
				virtual IBone* GetChildBone(size_t nIndex) const override { return m_vecChildBonesIndexing[nIndex]; }
				virtual IBone* GetChildBone(const String::StringID& strBoneName, bool isFindInAllDepth = false) const override;

				virtual bool IsRootBone() const override { return false; }

			public:
				Bone* AddChildBone(const String::StringID& strBoneName, const Math::Matrix& matMotionOffset, const Math::Matrix& matDefaultMotionData);
				
			private:
				String::StringID m_strBoneName;
				Math::Matrix m_matMotionOffset;
				Math::Matrix m_matDefaultMotionData;

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

			virtual size_t GetBoneCount() const override { return m_vecBones.size(); }
			virtual IBone* GetBone(size_t nIndex) const override { return m_vecBones[nIndex]; }
			virtual IBone* GetBone(const String::StringID& strBoneName) const override;

			virtual size_t GetSkinnedListCount() const override { return m_vecSkinnedList.size(); }
			virtual void GetSkinnedList(size_t nIndex, String::StringID& strSkinnedName_out, const String::StringID** ppBoneNames_out, size_t& nElementCount_out) override;

		public:
			Bone* CreateBone(const String::StringID& strBoneName, const Math::Matrix& matMotionOffset, const Math::Matrix& matDefaultMotionData);
			Bone* CreateBone(const String::StringID& strParentBoneName, const String::StringID& strBoneName, const Math::Matrix& matMotionOffset, const Math::Matrix& matDefaultMotionData);

			void SetSkinnedList(const String::StringID& strSkinnedName, const String::StringID* pBoneNames, size_t nNameCount);

		private:
			Bone* CreateChildBone(Bone* pParentBone, const String::StringID& strBoneName, const Math::Matrix& matMotionOffset, const Math::Matrix& matDefaultMotionData);

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
				virtual void Update(const Math::Matrix& matWorld, const Math::Matrix& matParent) override;

			public:
				virtual const String::StringID& GetName() const override { return m_pBoneHierarchy->GetName(); }

				virtual IBone* GetParent() const override { return m_pParentBone; }

				virtual size_t GetChildBoneCount() const override { return m_vecChildBonesIndexing.size(); }
				virtual IBone* GetChildBone(size_t nIndex) const override { return m_vecChildBonesIndexing[nIndex]; }
				virtual IBone* GetChildBone(const String::StringID& strBoneName, bool isFindInAllDepth = false) const override;

				virtual const Math::Matrix& GetMotionTransform() const override { return m_matMotionTransform; }
				virtual void SetMotionData(const Math::Matrix& matrix) override { m_matMotionData = matrix; }
				virtual void ClearMotionData() override { m_matMotionData = m_pBoneHierarchy->GetDefaultMotionData(); }

				virtual const Math::Matrix& GetLocalTransform() const override { return m_matLocalTransform; }
				virtual const Math::Matrix& GetGlobalTransform() const override { return m_matGlobalTransform; }

				virtual bool IsRootBone() const override { return false; }

			public:
				SkeletonInstance::BoneInstance* AddChildBone(Skeleton::IBone* pBoneHierarchy);

			private:
				void RenderBone();

			protected:
				Math::Matrix m_matMotionData;
				Math::Matrix m_matMotionTransform;
				Math::Matrix m_matLocalTransform;
				Math::Matrix m_matGlobalTransform;

				BoneInstance* m_pParentBone;
				plf::colony<BoneInstance> m_clnChildBones;
				std::vector<BoneInstance*> m_vecChildBonesIndexing;

				Skeleton::IBone* m_pBoneHierarchy;
			};

		private:
			class RootBone : public BoneInstance
			{
			public:
				RootBone(Skeleton::IBone* pBoneHierarchy);
				virtual ~RootBone();

				virtual void Update(const Math::Matrix& matWorld, const Math::Matrix& matParent) override;

				virtual bool IsRootBone() const override { return true; }
			};

		public:
			SkeletonInstance(ISkeleton* pSkeleton);
			virtual ~SkeletonInstance();

		public:
			virtual void Update(const Math::Matrix& matWorld) override;

			virtual ISkeleton* GetSkeleton() override { return m_pSkeleton; }

			virtual size_t GetBoneCount() const override { return m_vecBones.size(); }
			virtual IBone* GetBone(size_t nIndex) const override { return m_vecBones[nIndex]; }
			virtual IBone* GetBone(const String::StringID& strBoneName) const override;

			virtual void GetSkinnedData(const String::StringID& strSkinnedName, const Math::Matrix*** pppMatrixList_out, size_t& nElementCount_out) override;
			virtual void SetIdentity() override;
			virtual void SetDirty() override { m_isDirty = true; }
			virtual bool IsDirty() override { return m_isDirty; }

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