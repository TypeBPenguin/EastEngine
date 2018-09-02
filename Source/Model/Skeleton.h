#pragma once

#include "ModelInterface.h"

namespace eastengine
{
	namespace graphics
	{
		class Skeleton : public ISkeleton
		{
		public:
			class Bone : public ISkeleton::IBone
			{
			public:
				Bone(const String::StringID& strBoneName, const math::Matrix& matMotionOffset, const math::Matrix& matDefaultMotionData, uint32_t nIndex, uint32_t nParentIndex);
				Bone(const Bone& other);
				Bone(const Bone&& other) noexcept;

				virtual ~Bone() = default;

			public:
				virtual const String::StringID& GetName() const override { return m_strBoneName; }
				virtual const math::Matrix& GetMotionOffsetMatrix() const override { return m_matMotionOffset; }
				virtual const math::Matrix& GetDefaultMotionData() const override { return m_matDefaultMotionData; }

				virtual uint32_t GetIndex() const override { return m_nIndex; }
				virtual uint32_t GetParentIndex() const override { return m_nParentIndex; }

			private:
				String::StringID m_strBoneName;
				math::Matrix m_matMotionOffset;
				math::Matrix m_matDefaultMotionData;

				uint32_t m_nIndex{ eInvalidBoneIndex };
				uint32_t m_nParentIndex{ eInvalidBoneIndex };
			};

		public:
			Skeleton();
			virtual ~Skeleton();

		public:
			virtual uint32_t GetBoneCount() const override { return static_cast<uint32_t>(m_vecBones.size()); }
			virtual IBone* GetBone(uint32_t nIndex) override { return &m_vecBones[nIndex]; }
			virtual IBone* GetBone(const String::StringID& strBoneName) override;

			virtual uint32_t GetSkinnedListCount() const override { return static_cast<uint32_t>(m_vecSkinnedList.size()); }
			virtual void GetSkinnedList(uint32_t nIndex, String::StringID& strSkinnedName_out, const String::StringID** ppBoneNames_out, uint32_t& nElementCount_out) override;

		public:
			void ReserveBone(size_t nSize) { m_vecBones.reserve(nSize); }
			bool CreateBone(const String::StringID& strBoneName, const math::Matrix& matMotionOffset, const math::Matrix& matDefaultMotionData);
			bool CreateBone(const String::StringID& strParentBoneName, const String::StringID& strBoneName, const math::Matrix& matMotionOffset, const math::Matrix& matDefaultMotionData);

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

				virtual const math::Matrix& GetSkinningMatrix() const override { return m_matSkinning; }

				virtual void SetMotionMatrix(const math::Matrix& matrix) override { m_matMotion = matrix; }
				virtual const math::Matrix& GetMotionMatrix() const override { return m_matMotion; }
				virtual void ClearMotionMatrix() override { m_matMotion = m_pBoneOrigin->GetDefaultMotionData(); }

				virtual const math::Matrix& GetUserOffsetMatrix() const override { return m_matUserOffset; }
				virtual void SetUserOffsetMatrix(const math::Matrix& matrix) override { m_matUserOffset = matrix; }

				virtual const math::Matrix& GetLocalMatrix() const override { return m_matLocal; }
				virtual const math::Matrix& GetGlobalMatrix() const override { return m_matGlobal; }

			public:
				void Update(const math::Matrix& matWorld);

			protected:
				const String::StringID m_strBoneName;

				BoneInstance* m_pParentBone{ nullptr };
				Skeleton::IBone* m_pBoneOrigin{ nullptr };

				math::Matrix m_matUserOffset;
				math::Matrix m_matMotion;

				math::Matrix m_matSkinning;
				math::Matrix m_matLocal;
				math::Matrix m_matGlobal;
				const math::Matrix m_matMotionOffset;
			};

		public:
			SkeletonInstance() = default;
			virtual ~SkeletonInstance() = default;

		public:
			virtual ISkeleton* GetSkeleton() override { return m_pSkeleton; }

			virtual size_t GetBoneCount() const override { return m_vecBones.size(); }
			virtual IBone* GetBone(size_t nIndex) override { return &m_vecBones[nIndex]; }
			virtual IBone* GetBone(const String::StringID& strBoneName) override;

			virtual void GetSkinnedData(const String::StringID& strSkinnedName, const math::Matrix*** pppMatrixList_out, uint32_t& nElementCount_out) override;
			virtual void SetIdentity() override;
			virtual void SetDirty() override { m_isDirty = true; }
			virtual bool IsDirty() const override { return m_isDirty; }
			virtual bool IsValid() const override { return m_pSkeleton != nullptr; }

		public:
			void Initialize(ISkeleton* pSkeleton);
			void Update(const math::Matrix& matWorld);

		private:
			void CreateSkinnedData(ISkeleton* pSkeleton);

		private:
			bool m_isDirty{ true };

			ISkeleton* m_pSkeleton{ nullptr };

			std::vector<BoneInstance> m_vecBones;
			std::unordered_map<String::StringID, BoneInstance*> m_umapBones;

			std::unordered_map<String::StringID, std::vector<const math::Matrix*>> m_umapSkinnendData;
		};
	}
}