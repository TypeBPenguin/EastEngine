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
		class IMotionRecorder;
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
			enum : uint32_t
			{
				eInvalidMaterialID = std::numeric_limits<uint32_t>::max(),
			};

			String::StringID strName;

			uint32_t nMaterialID = eInvalidMaterialID;
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
			struct tKey {};
			using Key = PhantomType<tKey, const String::StringKey>;

			virtual Key GetKey() const = 0;

		public:
			struct Keyframe
			{
				float fTime = 0.f;
				Math::Transform transform;
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

				virtual void Update(float fInterval, IMotionRecorder* pRecorder, float fPlayTime, bool isInverse) const = 0;

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
			virtual void Update(IMotionRecorder* pRecorder, float fPlayTime, bool isInverse) const = 0;

			virtual float GetStartTime() const = 0;
			virtual float GetEndTime() const = 0;

		public:
			virtual const String::StringID& GetName() const = 0;
			virtual const std::string& GetFilePath() const = 0;

			virtual size_t GetBoneCount() const = 0;
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

		class IMotionRecorder
		{
		protected:
			IMotionRecorder() = default;
			virtual ~IMotionRecorder() = default;

		public:
			virtual void SetTransform(const String::StringID& strBoneName, const Math::Transform& keyframe) = 0;
			virtual const Math::Transform* GetTransform(const String::StringID& strBoneName) const = 0;
		};

		class IMotionSystem
		{
		protected:
			IMotionSystem() = default;
			virtual ~IMotionSystem() = default;

		public:
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

			virtual IModelNode* GetParentNode() const = 0;

			virtual IVertexBuffer* GetVertexBuffer(uint32_t nLOD = 0) const = 0;
			virtual IIndexBuffer* GetIndexBuffer(uint32_t nLOD = 0) const = 0;

			virtual size_t GetChildNodeCount() const = 0;
			virtual IModelNode* GetChildNode(size_t nIndex) const = 0;

			virtual size_t GetMaterialCount() const = 0;
			virtual IMaterial* GetMaterial(size_t nIndex) const = 0;
			virtual IMaterial* GetMaterial(const String::StringID& strMaterialName, uint32_t& nMaterialID_out) const = 0;

			virtual size_t GetModelSubsetCount(uint32_t nLOD = 0) const = 0;
			virtual const ModelSubset* GetModelSubset(size_t nIndex, uint32_t nLOD = 0) const = 0;

			virtual void SetOriginAABB(const Collision::AABB& aabb) = 0;
			virtual const Collision::AABB& GetOriginAABB() const = 0;

			virtual uint32_t GetLOD() const = 0;
			virtual void SetLOD(uint32_t nLOD) = 0;
		};

		class IModel : public Resource
		{
		public:
			struct tKey {};
			using Key = PhantomType<tKey, const String::StringKey>;

			virtual Key GetKey() const = 0;

		protected:
			IModel() = default;
			virtual ~IModel() = default;

		public:
			static IModel* Create(const ModelLoader& loader, bool isThreadLoad = true);

			static IModelInstance* CreateInstance(const ModelLoader& loader, bool isThreadLoad = true);
			static IModelInstance* CreateInstance(IModel* pIModel);
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

			virtual const Math::Matrix& GetLocalMatrix() const = 0;

			virtual const String::StringID& GetName() const = 0;
			virtual const std::string& GetFilePath() const = 0;

			virtual size_t GetNodeCount() const = 0;
			virtual IModelNode* GetNode(size_t nIndex) const = 0;
			virtual IModelNode* GetNode(const String::StringID& strName) const = 0;

			virtual bool IsVisible() const = 0;
			virtual void SetVisible(bool bVisible) = 0;

			virtual ISkeleton* GetSkeleton() = 0;

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

			virtual bool Attachment(IModelInstance* pInstance, const String::StringID& strNodeName, const Math::Matrix& matOffset = Math::Matrix::Identity) = 0;
			virtual bool Attachment(IModelInstance* pInstance, const Math::Matrix& matOffset = Math::Matrix::Identity) = 0;
			virtual IModelInstance* GetAttachment(size_t nIndex) const = 0;
			virtual size_t GetAttachmentCount() const = 0;
			virtual bool IsAttachment() const = 0;

			virtual bool Dettachment(IModelInstance* pInstance) = 0;

		public:
			virtual bool IsLoadComplete() const = 0;

			virtual void SetVisible(bool isVisible) = 0;
			virtual bool IsVisible() const = 0;

			virtual void ChangeMaterial(const String::StringID& strNodeName, uint32_t nIndex, IMaterial* pMaterial) = 0;

		public:
			virtual IModel* GetModel() = 0;
			virtual IMotionSystem* GetMotionSystem() = 0;
			virtual ISkeletonInstance* GetSkeleton() = 0;

			virtual const Math::Matrix& GetWorldMatrix() const = 0;
		};

		class ISkeleton
		{
		public:
			enum : uint32_t
			{
				eInvalidBoneIndex = std::numeric_limits<uint32_t>::max(),
			};

			class IBone
			{
			protected:
				IBone() = default;
				virtual ~IBone() = default;

			public:
				virtual const String::StringID& GetName() const = 0;
				virtual const Math::Matrix& GetMotionOffsetMatrix() const = 0;
				virtual const Math::Matrix& GetDefaultMotionData() const = 0;

				virtual uint32_t GetIndex() const = 0;
				virtual uint32_t GetParentIndex() const = 0;
			};

		protected:
			ISkeleton() = default;
			virtual ~ISkeleton() = default;

		public:
			virtual size_t GetBoneCount() const = 0;
			virtual IBone* GetBone(size_t nIndex) = 0;
			virtual IBone* GetBone(const String::StringID& strBoneName) = 0;

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
				virtual uint32_t GetIndex() const = 0;
				virtual const String::StringID& GetName() const = 0;

				virtual IBone* GetParent() const = 0;

				virtual const Math::Matrix& GetSkinningMatrix() const = 0;

				virtual void SetMotionMatrix(const Math::Matrix& matrix) = 0;
				virtual const Math::Matrix& GetMotionMatrix() const = 0;
				virtual void ClearMotionMatrix() = 0;

				virtual const Math::Matrix& GetUserOffsetMatrix() const = 0;
				virtual void SetUserOffsetMatrix(const Math::Matrix& matrix) = 0;

				virtual const Math::Matrix& GetLocalMatrix() const = 0;
				virtual const Math::Matrix& GetGlobalMatrix() const = 0;
			};

		protected:
			ISkeletonInstance() = default;
			virtual ~ISkeletonInstance() = default;

		public:
			virtual ISkeleton* GetSkeleton() = 0;

			virtual size_t GetBoneCount() const = 0;
			virtual IBone* GetBone(size_t nIndex) = 0;
			virtual IBone* GetBone(const String::StringID& strBoneName) = 0;

			virtual void GetSkinnedData(const String::StringID& strSkinnedName, const Math::Matrix*** pppMatrixList_out, uint32_t& nElementCount_out) = 0;
			virtual void SetIdentity() = 0;
			virtual void SetDirty() = 0;
			virtual bool IsDirty() const = 0;
			virtual bool IsValid() const = 0;
		};
	}
}

namespace std
{
	template <>
	struct hash<EastEngine::Graphics::IModel::Key>
	{
		std::uint64_t operator()(const EastEngine::Graphics::IModel::Key& key) const
		{
			return key.value.value;
		}
	};

	template <>
	struct hash<EastEngine::Graphics::IMotion::Key>
	{
		std::uint64_t operator()(const EastEngine::Graphics::IMotion::Key& key) const
		{
			return key.value.value;
		}
	};
}