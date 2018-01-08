#pragma once

#include "DirectX/D3DInterface.h"

namespace EastEngine
{
	namespace Graphics
	{
		struct MotionPlaybackInfo;

		class ModelLoader;
		class IModel;
		class IModelInstance;
		class IModelNode;
		class ISkeleton;
		class ISkeletonInstance;
		class IMotionPlayer;
		class IMotionRecoder;
		class MotionLoader;

		class IVertexBuffer;
		class IIndexBuffer;
		class IMaterial;

		namespace EmModelNode
		{
			enum Type
			{
				eStatic = 0,

				// 나중에 Skinning을 Static과 Dynamic로 바꾸자
				// 지금 구현된 Skinning 은 Dynamic
				// Static 은 예전에 만든 방식인, 애니메이션의 모든 프레임을 텍스쳐로 구워서 본업데이트 안하는 것
				eSkinned,

				eCount,
			};
		}

		namespace EmPrimitive
		{
			enum Type
			{
				eTriangleList = 0,
				eTriangleStrip,
				eLineList,
				eLineStrip,
				ePointList,
				eTriangleListAdj,
				eTriangleStripAdj,
				eLineListAdj,
				eLineStripAdj,
				eQuadPatchList,
				eTrianglePatchList,
			};
		}

		struct ModelSubset
		{
			String::StringID strName;

			uint32_t nMaterialID = std::numeric_limits<uint32_t>::max();
			uint32_t nStartIndex = 0;
			uint32_t nIndexCount = 0;

			EmPrimitive::Type emPrimitiveType = EmPrimitive::eTriangleList;
		};

		enum
		{
			eMaxLod = 5,
		};

		struct LODReductionRate
		{
			union
			{
				struct Level
				{
					float fLv0;
					float fLv1;
					float fLv2;
					float fLv3;
					float fLv4;
				};
				Level level;
				float fLv[5];
			};

			LODReductionRate();
			LODReductionRate(float fLv0, float fLv1, float fLv2, float fLv3, float fLv4);
		};

		class IMotion : public Resource
		{
		public:
			struct Keyframe
			{
				float fTime = 0.f;
				Math::Vector3 f3Pos;
				Math::Vector3 f3Scale = Math::Vector3::One;
				Math::Quaternion quatRotation;
			};

			class IBone
			{
			protected:
				IBone() = default;
				virtual ~IBone() = default;

			public:
				virtual const String::StringID& GetName() const = 0;
				virtual float GetStartTime() const = 0;
				virtual float GetEndTime() const = 0;

				virtual void Update(IMotionRecoder* pRecoder, float fPlayTime, bool isInverse) = 0;

			public:
				virtual uint32_t GetKeyframeCount() const = 0;
				virtual const Keyframe* GetKeyframe(uint32_t nIndex) const = 0;
			};

		protected:
			IMotion() = default;
			virtual ~IMotion() = default;

		public:
			static IMotion* Create(const MotionLoader& loader);
			static void Destroy(IMotion** ppMotion);

			static bool SaveToFile(IMotion* pMotion, const char* strFilePath);

		public:
			virtual void Update(IMotionRecoder* pRecoder, float fPlayTime, bool isInverse) = 0;

			virtual float GetStartTime() const = 0;
			virtual float GetEndTime() const = 0;

		public:
			virtual const String::StringID& GetName() = 0;

			virtual size_t GetBoneCount() = 0;
			virtual const IBone* GetBone(size_t nIndex) const = 0;
			virtual const IBone* GetBone(const String::StringID& strBoneName) const = 0;

		public:
			virtual int GetReferenceCount() const = 0;
			virtual int IncreaseReference() = 0;
			virtual int DecreaseReference() = 0;
		};

		namespace EmMotion
		{
			enum Layers
			{
				eLayer1 = 0,
				eLayer2,
				eLayer3,
				eLayer4,

				eLayerCount,
			};
		}

		struct MotionPlaybackInfo
		{
			enum : size_t
			{
				eMaxLoopCount = std::numeric_limits<size_t>::max(),
			};

			float fSpeed = 1.f;
			float fWeight = 0.f;
			float fBlendTime = 0.f;
			uint32_t nLoopCount = 1;
			bool isInverse = false;

			void Reset()
			{
				fSpeed = 1.f;
				fWeight = 0.f;
				fBlendTime = 0.f;
				nLoopCount = 1;
				isInverse = false;
			}
		};

		class IMotionPlayer
		{
		protected:
			IMotionPlayer() = default;
			virtual ~IMotionPlayer() = default;

		public:
			virtual float GetPlayTime() const = 0;
			virtual void SetPlayTime(float fPlayTime) = 0;

			virtual float GetSpeed() const = 0;
			virtual void SetSpeed(float fSpeed) = 0;

			virtual float GetWeight() const = 0;
			virtual void SetWeight(float fWeight) = 0;

			virtual float GetBlendWeight() const = 0;

			virtual float GetBlendTime() const = 0;
			virtual int GetMaxLoopCount() const = 0;
			virtual int GetLoopCount() const = 0;
			virtual bool IsLoop() const = 0;
			virtual bool IsInverse() const = 0;

			virtual bool IsPause() const = 0;
			virtual void SetPause(bool isPause) = 0;

			virtual IMotion* GetMotion() const = 0;
			virtual bool IsPlaying() const = 0;
		};

		class IMotionRecoder
		{
		protected:
			IMotionRecoder() = default;
			virtual ~IMotionRecoder() = default;

		public:
			enum : size_t
			{
				eInvalidCachingIndex = std::numeric_limits<size_t>::max(),
			};

		public:
			virtual void SetCaching(const String::StringID& strBoneName, size_t nIndex) = 0;
			virtual size_t GetCaching(const String::StringID& strBoneName) const = 0;

			virtual void SetKeyframe(const String::StringID& strBoneName, const IMotion::Keyframe& keyframe) = 0;
			virtual const IMotion::Keyframe* GetKeyframe(const String::StringID& strBoneName) const = 0;
		};

		class IMotionSystem
		{
		protected:
			IMotionSystem() = default;
			virtual ~IMotionSystem() = default;

		public:
			static IMotionSystem* Create(ISkeletonInstance* pSkeletonInstance);
			static void Destroy(IMotionSystem** ppMotionSystem);

		public:
			virtual void Update(float fElapsedTime) = 0;

			virtual void Play(EmMotion::Layers emLayer, IMotion* pMotion, const MotionPlaybackInfo* pPlayback = nullptr) = 0;
			virtual void Stop(EmMotion::Layers emLayer, float fStopTime) = 0;

			virtual IMotionPlayer* GetPlayer(EmMotion::Layers emLayer) = 0;
		};

		class IMaterialInstance
		{
		protected:
			IMaterialInstance() = default;
			virtual ~IMaterialInstance() = default;

		public:
			virtual IMaterial* GetMaterial(const String::StringID& strNodeName, uint32_t nIndex) const = 0;
		};

		class IModelNode
		{
		protected:
			IModelNode() = default;
			virtual ~IModelNode() = default;

		public:
			virtual void Update(float fElapsedTime, const Math::Matrix& matParent, ISkeletonInstance* pSkeletonInstance, IMaterialInstance* pMaterialInstance, bool isModelVisible) const = 0;

		public:
			virtual EmModelNode::Type GetType() const = 0;

			virtual bool IsVisible() const = 0;
			virtual void SetVisible(bool isVisible) = 0;

			virtual float GetDistanceFromCamera() const = 0;
			virtual void SetDistanceFromCamera(float fDist) = 0;

			virtual const String::StringID& GetName() const = 0;
			virtual const String::StringID& GetAttachedBoneName() const = 0;

			virtual IModelNode* GetParentNode() = 0;

			virtual IVertexBuffer* GetVertexBuffer(uint32_t nLOD = 0) = 0;
			virtual IIndexBuffer* GetIndexBuffer(uint32_t nLOD = 0) = 0;

			virtual size_t GetChildNodeCount() const = 0;
			virtual IModelNode* GetChildNode(size_t nIndex) = 0;

			virtual size_t GetMaterialCount() const = 0;
			virtual IMaterial* GetMaterial(size_t nIndex) = 0;

			virtual size_t GetModelSubsetCount(uint32_t nLOD = 0) const = 0;
			virtual ModelSubset* GetModelSubset(size_t nIndex, uint32_t nLOD = 0) = 0;

			virtual void SetOriginAABB(const Collision::AABB& aabb) = 0;
			virtual const Collision::AABB& GetOriginAABB() const = 0;

			virtual uint32_t GetLOD() = 0;
			virtual void SetLOD(uint32_t nLOD) = 0;
		};

		class IModel : public Resource
		{
		protected:
			IModel() = default;
			virtual ~IModel() = default;

		public:
			static IModel* Create(const ModelLoader& loader, bool isThreadLoad = true, size_t nReserveInstance = 8);
			static void Destroy(IModel** ppModel);

			static IModelInstance* CreateInstance(const ModelLoader& loader, bool isThreadLoad = true);
			static IModelInstance* CreateInstance(IModel* pModel);
			static void DestroyInstance(IModelInstance** ppModelInstance);

			static bool SaveToFile(IModel* pModel, const char* strFilePath);

		public:
			virtual void Update(float fElapsedTime, const Math::Matrix& matParent, ISkeletonInstance* pSkeletonInstance, IMaterialInstance* pMaterialInstance) = 0;

			virtual void ChangeName(const String::StringID& strName) = 0;

		public:
			virtual const Math::Vector3& GetLocalPosition() const = 0;
			virtual void SetLocalPosition(const Math::Vector3& f3Pos) = 0;
			virtual const Math::Vector3& GetLocalScale() const = 0;
			virtual void SetLocalScale(const Math::Vector3& f3Scale) = 0;
			virtual const Math::Quaternion& GetLocalRotation() const = 0;
			virtual void SetLocalRotation(const Math::Quaternion& quat) = 0;

			virtual const Math::Matrix& GetLocalTransform() const = 0;

			virtual const String::StringID& GetName() const = 0;
			virtual const std::string& GetFilePath() const = 0;

			virtual size_t GetNodeCount() const = 0;
			virtual IModelNode* GetNode(size_t nIndex) const = 0;
			virtual IModelNode* GetNode(const String::StringID& strName) const = 0;

			virtual bool IsVisible() const = 0;
			virtual void SetVisible(bool bVisible) = 0;

			virtual ISkeleton* GetSkeleton() const = 0;

		public:
			virtual int GetReferenceCount() const = 0;
			virtual int IncreaseReference() = 0;
			virtual int DecreaseReference() = 0;
		};

		class IModelInstance
		{
		protected:
			IModelInstance() = default;
			virtual ~IModelInstance() = default;

		public:
			virtual void Update(float fElapsedTime, const Math::Matrix& matParent) = 0;

			virtual bool Attachment(IModelInstance* pInstance, const String::StringID& strNodeName, const Math::Matrix& matOffset) = 0;
			virtual IModelInstance* GetAttachment(size_t nIndex) const = 0;
			virtual size_t GetAttachmentCount() const = 0;
			virtual bool IsAttachment() const = 0;

			virtual bool Dettachment(IModelInstance* pInstance) = 0;

		public:
			virtual bool IsLoadComplete() = 0;

			virtual void SetVisible(bool isVisible) = 0;
			virtual bool IsVisible() = 0;

			virtual void ChangeMaterial(const String::StringID& strNodeName, uint32_t nIndex, IMaterial* pMaterial) = 0;

		public:
			virtual IModel* GetModel() = 0;
			virtual IMotionSystem* GetMotionSystem() = 0;
			virtual ISkeletonInstance* GetSkeleton() = 0;

			virtual const Math::Matrix& GetWorldMatrix() = 0;
		};

		class ISkeleton
		{
		public:
			class IBone
			{
			protected:
				IBone() = default;
				virtual ~IBone() = default;

			public:
				virtual const String::StringID& GetName() const = 0;
				virtual const Math::Matrix& GetMotionOffsetMatrix() const = 0;
				virtual const Math::Matrix& GetDefaultMotionData() const = 0;

				virtual IBone* GetParent() const = 0;

				virtual size_t GetChildBoneCount() const = 0;
				virtual IBone* GetChildBone(size_t nIndex) const = 0;
				virtual IBone* GetChildBone(const String::StringID& strBoneName, bool isFindInAllDepth = false) const = 0;

				virtual bool IsRootBone() const = 0;
			};

		protected:
			ISkeleton() = default;
			virtual ~ISkeleton() = default;

		public:
			static ISkeleton* Create();
			static void Destroy(ISkeleton** ppSkeleton);

			static ISkeletonInstance* CreateInstance(ISkeleton* pSkeleton);
			static void DestroyInstance(ISkeletonInstance** ppSkeletonInstance);

		public:
			virtual IBone* GetRootBone() const = 0;

			virtual size_t GetBoneCount() const = 0;
			virtual IBone* GetBone(size_t nIndex) const = 0;
			virtual IBone* GetBone(const String::StringID& strBoneName) const = 0;

			virtual size_t GetSkinnedListCount() const = 0;
			virtual void GetSkinnedList(size_t nIndex, String::StringID& strSkinnedName_out, const String::StringID** ppBoneNames_out, uint32_t& nElementCount_out) = 0;
		};

		class ISkeletonInstance
		{
		public:
			class IBone
			{
			protected:
				IBone() = default;
				virtual ~IBone() = default;

			public:
				virtual void Update(const Math::Matrix& matWorld, const Math::Matrix& matParent) = 0;

			public:
				virtual const String::StringID& GetName() const = 0;

				virtual IBone* GetParent() const = 0;

				virtual size_t GetChildBoneCount() const = 0;
				virtual IBone* GetChildBone(size_t nIndex) const = 0;
				virtual IBone* GetChildBone(const String::StringID& strBoneName, bool isFindInAllDepth = false) const = 0;

				virtual const Math::Matrix& GetMotionTransform() const = 0;
				virtual void SetMotionData(const Math::Matrix& matrix) = 0;
				virtual void ClearMotionData() = 0;

				virtual const Math::Matrix& GetLocalTransform() const = 0;
				virtual const Math::Matrix& GetGlobalTransform() const = 0;

				virtual bool IsRootBone() const = 0;
			};

		protected:
			ISkeletonInstance() = default;
			virtual ~ISkeletonInstance() = default;

		public:
			virtual void Update(const Math::Matrix& matWorld) = 0;

			virtual ISkeleton* GetSkeleton() = 0;

			virtual size_t GetBoneCount() const = 0;
			virtual IBone* GetBone(size_t nIndex) const = 0;
			virtual IBone* GetBone(const String::StringID& strBoneName) const = 0;

			virtual void GetSkinnedData(const String::StringID& strSkinnedName, const Math::Matrix*** pppMatrixList_out, uint32_t& nElementCount_out) = 0;
			virtual void SetIdentity() = 0;
			virtual void SetDirty() = 0;
			virtual bool IsDirty() = 0;
		};
	}
}