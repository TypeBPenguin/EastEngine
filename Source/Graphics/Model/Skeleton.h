#pragma once

#include "CommonLib/FileStream.h"

#include "ModelInterface.h"

namespace est
{
	namespace graphics
	{
		class Skeleton : public ISkeleton
		{
		public:
			class Bone : public ISkeleton::IBone
			{
			public:
				Bone() = default;
				Bone(const string::StringID& boneName, const math::Matrix& matMotionOffset, const math::Matrix& matDefaultMotionData, uint32_t index, uint32_t parentIndex);
				virtual ~Bone() = default;

			public:
				virtual const string::StringID& GetName() const override { return m_boneName; }
				virtual const math::Matrix& GetMotionOffsetMatrix() const override { return m_matMotionOffset; }
				virtual const math::Matrix& GetDefaultMotionData() const override { return m_matDefaultMotionData; }

				virtual uint32_t GetIndex() const override { return m_index; }
				virtual uint32_t GetParentIndex() const override { return m_parentIndex; }

			private:
				string::StringID m_boneName;
				uint32_t m_index{ eInvalidBoneIndex };
				uint32_t m_parentIndex{ eInvalidBoneIndex };

				math::Matrix m_matMotionOffset;
				math::Matrix m_matDefaultMotionData;
			};

		public:
			Skeleton();
			virtual ~Skeleton();

		public:
			virtual uint32_t GetBoneCount() const override { return static_cast<uint32_t>(m_bones.size()); }
			virtual IBone* GetBone(uint32_t index) override { return &m_bones[index]; }
			virtual IBone* GetBone(const string::StringID& boneName) override;

			virtual uint32_t GetSkinnedListCount() const override { return static_cast<uint32_t>(m_skinnedList.size()); }
			virtual void GetSkinnedList(uint32_t index, string::StringID& skinnedName_out, const string::StringID** ppBoneNames_out, uint32_t& nElementCount_out) override;

		public:
			void ReserveBone(size_t nSize) { m_bones.reserve(nSize); }
			bool CreateBone(const string::StringID& boneName, const math::Matrix& matMotionOffset, const math::Matrix& matDefaultMotionData);
			bool CreateBone(const string::StringID& boneName, const string::StringID& parentBoneName, const math::Matrix& matMotionOffset, const math::Matrix& matDefaultMotionData);

			void SetSkinnedList(const string::StringID& skinnedName, const string::StringID* pBoneNames, size_t nNameCount);

		public:
			void LoadFile(BinaryReader& binaryReader);

		private:
			std::vector<Bone> m_bones;

			struct SkinnedData
			{
				string::StringID name;
				std::vector<string::StringID> boneNames;

				SkinnedData(const string::StringID& strName);
			};
			std::vector<SkinnedData> m_skinnedList;
		};

		class SkeletonInstance : public ISkeletonInstance
		{
		public:
			class BoneInstance : public ISkeletonInstance::IBone
			{
			public:
				BoneInstance(const Skeleton::IBone* pOriginBone, const BoneInstance* pParentBone);
				virtual ~BoneInstance() = default;

			public:
				virtual uint32_t GetIndex() const override { return m_index; }
				virtual const string::StringID& GetName() const override { return m_boneName; }

				virtual const IBone* GetParent() const override { return m_pParentBone; }

				virtual const math::Matrix& GetSkinningMatrix() const override { return m_matSkinning; }

				virtual void SetMotionMatrix(const math::Matrix& matrix) override { m_matMotion = matrix; }
				virtual const math::Matrix& GetMotionMatrix() const override { return m_matMotion; }
				virtual void ClearMotionMatrix() override { m_matMotion = m_pOriginBone->GetDefaultMotionData(); }

				virtual const math::Matrix& GetLocalMatrix() const override { return m_matLocal; }
				virtual const math::Matrix& GetGlobalMatrix() const override { return m_matGlobal; }

			public:
				void Update(const math::Matrix& matWorld, const math::Matrix* pUserOffsetMatrix);

			protected:
				const Skeleton::IBone* m_pOriginBone{ nullptr };
				uint32_t m_index{ ISkeleton::eInvalidBoneIndex };
				string::StringID m_boneName;

				const BoneInstance* m_pParentBone{ nullptr };

				math::Matrix m_matMotion;

				math::Matrix m_matSkinning;
				math::Matrix m_matLocal;
				math::Matrix m_matGlobal;
			};

		public:
			SkeletonInstance() = default;
			virtual ~SkeletonInstance() = default;

		public:
			virtual ISkeleton* GetSkeleton() override { return m_pSkeleton; }

			virtual size_t GetBoneCount() const override { return m_bones.size(); }
			virtual IBone* GetBone(size_t index) override { return &m_bones[index]; }
			virtual IBone* GetBone(const string::StringID& boneName) override;

			virtual void GetSkinnedData(const string::StringID& skinnedName, const math::Matrix* const** pppMatrixList_out, uint32_t& nElementCount_out) override;
			virtual void SetIdentity() override;
			virtual void SetDirty() override { m_isDirty = true; }
			virtual bool IsDirty() const override { return m_isDirty; }
			virtual bool IsValid() const override { return m_pSkeleton != nullptr; }

			virtual const math::Matrix* GetUserOffsetMatrix(const string::StringID& boneName) const override;
			virtual void SetUserOffsetMatrix(const string::StringID& boneName, const math::Matrix& matrix) override;

		public:
			void Initialize(ISkeleton* pSkeleton);
			void Update(const math::Matrix& matWorld);

		private:
			void CreateSkinnedData(ISkeleton* pSkeleton);

		private:
			bool m_isDirty{ true };

			ISkeleton* m_pSkeleton{ nullptr };

			std::vector<BoneInstance> m_bones;
			tsl::robin_map<string::StringID, BoneInstance*> m_rmapBones;
			tsl::robin_map<string::StringID, math::Matrix> m_rmapUserOffsetMatrix;

			tsl::robin_map<string::StringID, std::vector<const math::Matrix*>> m_rmapSkinnendData;
		};
	}
}