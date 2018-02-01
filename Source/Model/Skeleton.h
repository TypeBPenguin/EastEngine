#pragma once

#include "ModelInterface.h"

namespace EastEngine
{
	namespace Graphics
	{
		class Skeleton : public ISkeleton
		{
		public:
			class Bone : public ISkeleton::IBone
			{
			public:
				Bone(const String::StringID& strBoneName, const Math::Matrix& matMotionOffset, const Math::Matrix& matDefaultMotionData, uint32_t nIndex, uint32_t nParentIndex);
				Bone(const Bone& other);
				Bone(const Bone&& other) noexcept;

				virtual ~Bone() = default;

			public:
				virtual const String::StringID& GetName() const override { return m_strBoneName; }
				virtual const Math::Matrix& GetMotionOffsetMatrix() const override { return m_matMotionOffset; }
				virtual const Math::Matrix& GetDefaultMotionData() const override { return m_matDefaultMotionData; }

				virtual uint32_t GetIndex() const override { return m_nIndex; }
				virtual uint32_t GetParentIndex() const override { return m_nParentIndex; }

			private:
				String::StringID m_strBoneName;
				Math::Matrix m_matMotionOffset;
				Math::Matrix m_matDefaultMotionData;

				uint32_t m_nIndex{ eInvalidBoneIndex };
				uint32_t m_nParentIndex{ eInvalidBoneIndex };
			};

		public:
			Skeleton();
			virtual ~Skeleton();

		public:
			virtual size_t GetBoneCount() const override { return m_vecBones.size(); }
			virtual IBone* GetBone(size_t nIndex) override { return &m_vecBones[nIndex]; }
			virtual IBone* GetBone(const String::StringID& strBoneName) override;

			virtual size_t GetSkinnedListCount() const override { return m_vecSkinnedList.size(); }
			virtual void GetSkinnedList(size_t nIndex, String::StringID& strSkinnedName_out, const String::StringID** ppBoneNames_out, size_t& nElementCount_out) override;

		public:
			Bone* CreateBone(const String::StringID& strBoneName, const Math::Matrix& matMotionOffset, const Math::Matrix& matDefaultMotionData);
			Bone* CreateBone(const String::StringID& strParentBoneName, const String::StringID& strBoneName, const Math::Matrix& matMotionOffset, const Math::Matrix& matDefaultMotionData);

			void SetSkinnedList(const String::StringID& strSkinnedName, const String::StringID* pBoneNames, size_t nNameCount);

		private:
			std::vector<Bone> m_vecBones;

			struct SkinnedData
			{
				String::StringID strName;
				std::vector<String::StringID> vecBoneNames;

				SkinnedData(const String::StringID& strName);
			};
			std::vector<SkinnedData> m_vecSkinnedList;
		};

		class SkeletonInstance : public ISkeletonInstance
		{
		public:
			class BoneInstance : public ISkeletonInstance::IBone
			{
			public:
				BoneInstance(Skeleton::IBone* pBoneOrigin, BoneInstance* pParentBone);
				virtual ~BoneInstance() = default;

			public:
				virtual uint32_t GetIndex() const override { return m_pBoneOrigin->GetIndex(); }
				virtual const String::StringID& GetName() const override { return m_strBoneName; }

				virtual IBone* GetParent() const override { return m_pParentBone; }

				virtual const Math::Matrix& GetSkinningMatrix() const override { return m_matSkinning; }

				virtual void SetMotionTransform(const Math::Transform& transform) override { m_motionTransform = transform; }
				virtual const Math::Transform& GetMotionTransform() const override { return m_motionTransform; }
				virtual void ClearMotionTransform() override { m_motionTransform = m_pBoneOrigin->GetDefaultMotionData(); }

				virtual const Math::Vector3& GetUserOffsetScale() const override { return m_userOffsetScale; }
				virtual void SetUserOffsetScale(const Math::Vector3& f3Scale) override { m_userOffsetScale = f3Scale; }

				virtual const Math::Vector3& GetUserOffsetRotation() const override { return m_userOffsetRotation; }
				virtual void SetUserOffsetRotation(const Math::Vector3& f3Rotation) override { m_userOffsetRotation = f3Rotation; }

				virtual const Math::Vector3& GetUserOffsetPosition() const override { return m_userOffsetPosition; }
				virtual void SetUserOffsetPosition(const Math::Vector3& f3Position) override { m_userOffsetPosition = f3Position; }

				virtual const Math::Matrix& GetLocalMatrix() const override { return m_matLocal; }
				virtual const Math::Matrix& GetGlobalMatrix() const override { return m_matGlobal; }

			public:
				void Update(const Math::Matrix& matWorld);

			protected:
				const String::StringID m_strBoneName;

				BoneInstance* m_pParentBone{ nullptr };
				Skeleton::IBone* m_pBoneOrigin{ nullptr };

				Math::Transform m_motionTransform;

				Math::Vector3 m_userOffsetScale{ Math::Vector3::One };
				Math::Vector3 m_userOffsetRotation;
				Math::Vector3 m_userOffsetPosition;

				Math::Matrix m_matSkinning;
				Math::Matrix m_matLocal;
				Math::Matrix m_matGlobal;
				const Math::Matrix m_matMotionOffset;
			};

		public:
			SkeletonInstance() = default;
			virtual ~SkeletonInstance() = default;

		public:
			virtual ISkeleton* GetSkeleton() override { return m_pSkeleton; }

			virtual size_t GetBoneCount() const override { return m_vecBones.size(); }
			virtual IBone* GetBone(size_t nIndex) override { return &m_vecBones[nIndex]; }
			virtual IBone* GetBone(const String::StringID& strBoneName) override;

			virtual void GetSkinnedData(const String::StringID& strSkinnedName, const Math::Matrix*** pppMatrixList_out, size_t& nElementCount_out) override;
			virtual void SetIdentity() override;
			virtual void SetDirty() override { m_isDirty = true; }
			virtual bool IsDirty() const override { return m_isDirty; }
			virtual bool IsValid() const override { return m_pSkeleton != nullptr; }

		public:
			void Initialize(ISkeleton* pSkeleton);
			void Update(const Math::Matrix& matWorld);

		private:
			void CreateSkinnedData(ISkeleton* pSkeleton);

		private:
			bool m_isDirty{ true };

			ISkeleton* m_pSkeleton{ nullptr };

			std::vector<BoneInstance> m_vecBones;
			std::unordered_map<String::StringID, BoneInstance*> m_umapBones;

			std::unordered_map<String::StringID, std::vector<const Math::Matrix*>> m_umapSkinnendData;
		};
	}
}