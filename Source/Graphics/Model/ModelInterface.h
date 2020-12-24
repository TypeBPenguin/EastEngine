#pragma once

#include "Graphics/Interface/GraphicsInterface.h"
#include "ModelLoader.h"
#include "MotionLoader.h"

namespace sid
{
	RegisterStringID(Model);
	RegisterStringID(Motion);
}

namespace est
{
	namespace graphics
	{
		struct MotionPlaybackInfo;

		class IModel;
		class IModelInstance;
		class IModelNode;
		class ISkeleton;
		class ISkeletonInstance;
		class ITransformInstance;
		class IMotionPlayer;

		struct ModelSubset
		{
			enum : uint32_t
			{
				eInvalidMaterialID = std::numeric_limits<uint32_t>::max(),
			};

			string::StringID name;

			uint32_t materialID{ eInvalidMaterialID };
			uint32_t startIndex{ 0 };
			uint32_t indexCount{ 0 };

			Primitive pimitiveType{ Primitive::eTriangleList };
		};

		enum LOD
		{
			eLv0 = 0,
			eLv1,
			eLv2,
			eLv3,
			eLv4,
			eMaxLod,
		};

		struct LODReductionRate
		{
			union
			{
				struct
				{
					float lv0;
					float lv1;
					float lv2;
					float lv3;
					float lv4;
				};
				float levels[eMaxLod];
			};

			LODReductionRate();
			LODReductionRate(float lv0, float lv1, float lv2, float lv3, float lv4);
		};

		enum MotionLayers
		{
			eLayer1 = 0,
			eLayer2,
			eLayer3,
			eLayer4,

			eLayerCount,
		};

		struct IMotionEvent
		{
			const int id{ 0 };
			const float time{ 0.f };

			IMotionEvent(int id, float time);
			virtual ~IMotionEvent() = 0;
		};

		class IMotion;
		using MotionPtr = std::shared_ptr<IMotion>;

		class IMotion : public IResource
		{
			GraphicsResource(IMotion);
		private:
			struct tKey { static constexpr const wchar_t* DefaultValue() { return L""; } };

		public:
			using Key = PhantomType<tKey, string::StringID>;
			virtual Key GetKey() const = 0;

		public:
			struct Keyframe
			{
				float time{ 0.f };
				math::Transform transform;
			};

			class IBone
			{
			protected:
				IBone() = default;
				virtual ~IBone() = default;

			public:
				virtual const string::StringID& GetName() const = 0;
				virtual float GetStartTime() const = 0;
				virtual float GetEndTime() const = 0;

			public:
				virtual uint32_t GetKeyframeCount() const = 0;
				virtual const Keyframe* GetKeyframe(uint32_t index) const = 0;
			};

		protected:
			IMotion() = default;
			virtual ~IMotion() = default;

		public:
			virtual const string::StringID& GetResourceType() const override { return sid::Motion; }

		public:
			virtual float GetStartTime() const = 0;
			virtual float GetEndTime() const = 0;
			virtual float GetFrameInterval() const = 0;

		public:
			virtual const string::StringID& GetName() const = 0;
			virtual const std::wstring& GetFilePath() const = 0;

			virtual uint32_t GetBoneCount() const = 0;
			virtual const IBone* GetBone(uint32_t index) const = 0;
			virtual const IBone* GetBone(const string::StringID& boneName) const = 0;
		};

		struct MotionPlaybackInfo
		{
			enum : uint32_t
			{
				eMaxLoopCount = std::numeric_limits<uint32_t>::max(),
			};

			float speed{ 1.f };
			float weight{ 1.f };
			float blendTime{ 0.2f };
			uint32_t loopCount{ 1 };
			bool isInverse{ false };
			bool isFreezeAtLastFrame{ false };

			void Reset()
			{
				speed = 1.f;
				weight = 1.f;
				blendTime = 0.2f;
				loopCount = 1;
				isInverse = false;
				isFreezeAtLastFrame = false;
			}
		};

		class IMotionPlayer
		{
		protected:
			IMotionPlayer() = default;
			virtual ~IMotionPlayer() = default;

		public:
			virtual float GetPlayTime() const = 0;
			virtual void SetPlayTime(float playTime) = 0;

			virtual float GetSpeed() const = 0;
			virtual void SetSpeed(float speed) = 0;

			virtual float GetWeight() const = 0;
			virtual void SetWeight(float weight) = 0;

			virtual float GetBlendWeight() const = 0;

			virtual float GetBlendTime() const = 0;
			virtual int GetMaxLoopCount() const = 0;
			virtual int GetLoopCount() const = 0;
			virtual bool IsLoop() const = 0;
			virtual bool IsInverse() const = 0;

			virtual bool IsPause() const = 0;
			virtual void SetPause(bool isPause) = 0;

			virtual MotionPtr GetMotion() const = 0;
			virtual bool IsPlaying() const = 0;

			virtual const IMotionEvent* PopEvent() = 0;
		};

		class IMotionSystem
		{
		protected:
			IMotionSystem() = default;
			virtual ~IMotionSystem() = default;

		public:
			virtual void Play(MotionLayers emLayer, const MotionPtr& pMotion, const MotionPlaybackInfo* pPlayback = nullptr) = 0;
			virtual void Stop(MotionLayers emLayer, float stopTime) = 0;

			virtual IMotionPlayer* GetPlayer(MotionLayers emLayer) = 0;
		};

		class IMaterialInstance
		{
		protected:
			IMaterialInstance() = default;
			virtual ~IMaterialInstance() = default;

		public:
			virtual MaterialPtr GetMaterial(const string::StringID& nodeName, uint32_t index) const = 0;
		};

		class IModelNode
		{
		protected:
			IModelNode() = default;
			virtual ~IModelNode() = default;

		public:
			enum Type
			{
				eStatic = 0,

				// 나중에 Skinning을 Static과 Dynamic로 나누자
				// 지금 구현된 Skinning 은 Dynamic
				// Static 은 예전에 만든 방식인, 애니메이션의 모든 프레임을 텍스쳐로 구워서 본업데이트 안하는 것
				eSkinned,

				eCount,
			};

		public:
			virtual IModelNode::Type GetType() const = 0;

			virtual bool IsVisible() const = 0;
			virtual void SetVisible(bool isVisible) = 0;

			virtual float GetDistanceFromCamera() const = 0;
			virtual void SetDistanceFromCamera(float dist) = 0;

			virtual const string::StringID& GetName() const = 0;
			virtual const string::StringID& GetAttachedBoneName() const = 0;

			virtual const string::StringID& GetParentName() const = 0;

			virtual VertexBufferPtr GetVertexBuffer(LOD emLod = eLv0) const = 0;
			virtual IndexBufferPtr GetIndexBuffer(LOD emLod = eLv0) const = 0;

			virtual void GetRawVertices(const VertexPos** ppVertices, size_t& vertexCount) const = 0;
			virtual void GetRawIndices(const uint32_t** ppIndices, size_t& indexCount) const = 0;

			virtual uint32_t GetChildNodeCount() const = 0;
			virtual IModelNode* GetChildNode(uint32_t index) const = 0;

			virtual uint32_t GetMaterialCount() const = 0;
			virtual MaterialPtr GetMaterial(uint32_t index) const = 0;
			virtual MaterialPtr GetMaterial(const string::StringID& strMaterialName, uint32_t& nMaterialID_out) const = 0;

			virtual uint32_t GetModelSubsetCount(LOD emLod = eLv0) const = 0;
			virtual const ModelSubset* GetModelSubset(uint32_t index, LOD emLod = eLv0) const = 0;

			virtual void SetOriginAABB(const collision::AABB& aabb) = 0;
			virtual const collision::AABB& GetOriginAABB() const = 0;

			virtual LOD GetLOD() const = 0;
			virtual void SetLOD(LOD emLod) = 0;
		};

		struct ModelInstanceDeleter { void operator()(IModelInstance* pModelInstance); };
		using ModelInstancePtr = std::unique_ptr<IModelInstance, ModelInstanceDeleter>;

		class IModel : public IResource
		{
			GraphicsResource(IModel);
		private:
			struct tKey { static constexpr const wchar_t* DefaultValue() { return L""; } };

		public:
			using Key = PhantomType<tKey, string::StringID>;
			virtual Key GetKey() const = 0;

		protected:
			IModel() = default;
			virtual ~IModel() = default;

		public:
			virtual const string::StringID& GetResourceType() const override { return sid::Model; }

		public:
			virtual void Update(float elapsedTime, const math::Matrix& matParent, ISkeletonInstance* pSkeletonInstance, IMaterialInstance* pMaterialInstance, ITransformInstance* pTransformInstance) = 0;

			virtual void ChangeName(const string::StringID& strName) = 0;

		public:
			virtual const math::float3& GetLocalPosition() const = 0;
			virtual void SetLocalPosition(const math::float3& f3Pos) = 0;
			virtual const math::float3& GetLocalScale() const = 0;
			virtual void SetLocalScale(const math::float3& f3Scale) = 0;
			virtual const math::Quaternion& GetLocalRotation() const = 0;
			virtual void SetLocalRotation(const math::Quaternion& quat) = 0;

			virtual const math::Matrix& GetLocalMatrix() const = 0;

			virtual const string::StringID& GetName() const = 0;
			virtual const std::wstring& GetFilePath() const = 0;

			virtual uint32_t GetNodeCount() const = 0;
			virtual IModelNode* GetNode(uint32_t index) const = 0;
			virtual IModelNode* GetNode(const string::StringID& strName) const = 0;

			virtual bool IsVisible() const = 0;
			virtual void SetVisible(bool bVisible) = 0;

			virtual ISkeleton* GetSkeleton() = 0;
		};

		class IModelInstance
		{
		protected:
			IModelInstance() = default;
			virtual ~IModelInstance() = default;

		public:
			virtual void Update(float elapsedTime, const math::Matrix& matParent) = 0;

			virtual bool Attachment(uint32_t id, ModelInstancePtr pInstance, const string::StringID& nodeName, const math::Matrix& matOffset = math::Matrix::Identity) = 0;
			virtual bool Attachment(uint32_t id, ModelInstancePtr pInstance, const math::Matrix& matOffset = math::Matrix::Identity) = 0;
			virtual IModelInstance* GetAttachment(uint32_t id) const = 0;
			virtual size_t GetAttachmentCount() const = 0;
			virtual bool IsAttachment() const = 0;

			virtual bool Dettachment(uint32_t id) = 0;

		public:
			virtual bool IsLoadComplete() const = 0;

			virtual void SetVisible(bool isVisible) = 0;
			virtual bool IsVisible() const = 0;

			virtual void ChangeMaterial(const string::StringID& nodeName, uint32_t index, const MaterialPtr& pMaterial) = 0;

		public:
			virtual IModel* GetModel() = 0;
			virtual IMotionSystem* GetMotionSystem() = 0;
			virtual ISkeletonInstance* GetSkeleton() = 0;
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
				virtual const string::StringID& GetName() const = 0;
				virtual const math::Matrix& GetMotionOffsetMatrix() const = 0;
				virtual const math::Matrix& GetDefaultMotionData() const = 0;

				virtual uint32_t GetIndex() const = 0;
				virtual uint32_t GetParentIndex() const = 0;
			};

		protected:
			ISkeleton() = default;
			virtual ~ISkeleton() = default;

		public:
			virtual uint32_t GetBoneCount() const = 0;
			virtual IBone* GetBone(uint32_t index) = 0;
			virtual IBone* GetBone(const string::StringID& boneName) = 0;

			virtual uint32_t GetSkinnedListCount() const = 0;
			virtual void GetSkinnedList(uint32_t index, string::StringID& skinnedName_out, const string::StringID** ppBoneNames_out, uint32_t& nElementCount_out) = 0;
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
				virtual const string::StringID& GetName() const = 0;

				virtual const IBone* GetParent() const = 0;

				virtual const math::Matrix& GetSkinningMatrix() const = 0;

				virtual void SetMotionMatrix(const math::Matrix& matrix) = 0;
				virtual const math::Matrix& GetMotionMatrix() const = 0;
				virtual void ClearMotionMatrix() = 0;

				virtual const math::Matrix& GetLocalMatrix() const = 0;
				virtual const math::Matrix& GetGlobalMatrix() const = 0;
			};

		protected:
			ISkeletonInstance() = default;
			virtual ~ISkeletonInstance() = default;

		public:
			virtual ISkeleton* GetSkeleton() = 0;

			virtual size_t GetBoneCount() const = 0;
			virtual IBone* GetBone(size_t index) = 0;
			virtual IBone* GetBone(const string::StringID& boneName) = 0;

			virtual void GetSkinnedData(const string::StringID& skinnedName, const math::Matrix* const** pppMatrixList_out, uint32_t& nElementCount_out) = 0;
			virtual void SetIdentity() = 0;
			virtual void SetDirty() = 0;
			virtual bool IsDirty() const = 0;
			virtual bool IsValid() const = 0;

			virtual const math::Matrix* GetUserOffsetMatrix(const string::StringID& boneName) const = 0;
			virtual void SetUserOffsetMatrix(const string::StringID& boneName, const math::Matrix& matrix) = 0;
		};

		class ITransformInstance
		{
		public:
			ITransformInstance() = default;
			virtual ~ITransformInstance() = default;

		public:
			virtual void SetTransform(const string::StringID& nodeName, const math::Matrix& matTransform) = 0;
			virtual const math::Matrix& GetPrevTransform(const string::StringID& nodeName) const = 0;

			virtual void SetVTFID(const string::StringID& nodeName, uint32_t VTFID) = 0;
			virtual uint32_t GetPrevVTFID(const string::StringID& nodeName) const = 0;
		};

		MotionPtr CreateMotion(const MotionLoader& loader);
		bool SaveFile(IMotion* pMotion, const wchar_t* filePath);

		IModel* CreateModel(const ModelLoader& loader, bool isThreadLoad = true);

		ModelInstancePtr CreateModelInstance(const ModelLoader& loader, bool isThreadLoad = true);
		ModelInstancePtr CreateModelInstance(IModel* pIModel);

		bool SaveFile(IModel* pModel, const wchar_t* filePath);
	}
}

namespace std
{
	template <>
	struct hash<est::graphics::IModel::Key>
	{
		const size_t operator()(const est::graphics::IModel::Key& key) const
		{
			return reinterpret_cast<size_t>(key.Value().GetData());
		}
	};

	template <>
	struct hash<est::graphics::IMotion::Key>
	{
		const size_t operator()(const est::graphics::IMotion::Key& key) const
		{
			return reinterpret_cast<size_t>(key.Value().GetData());
		}
	};
}