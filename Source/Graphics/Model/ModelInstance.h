#pragma once

#include "ModelInterface.h"

#include "Skeleton.h"
#include "MotionSystem.h"

namespace est
{
	namespace graphics
	{
		class MaterialInstance : public IMaterialInstance
		{
		public:
			MaterialInstance();
			virtual ~MaterialInstance();

		public:
			virtual MaterialPtr GetMaterial(const string::StringID& nodeName, uint32_t index) const override;

		public:
			void AddMaterial(const string::StringID& nodeName, uint32_t index, const MaterialPtr& pMaterial);

		private:
			struct NodeMaterial
			{
				string::StringID nodeName;
				uint32_t index{ std::numeric_limits<uint32_t>::max() };
				MaterialPtr pMaterial;

				NodeMaterial(const string::StringID& nodeName, uint32_t index, const MaterialPtr& pMaterial);
			};

			std::vector<NodeMaterial> m_materials;
		};

		class TransformInstance : public ITransformInstance
		{
		public:
			TransformInstance();
			virtual ~TransformInstance();

		public:
			virtual void SetTransform(const string::StringID& nodeName, const math::Matrix& matTransform) override;
			virtual const math::Matrix& GetPrevTransform(const string::StringID& nodeName) const override;

			virtual void SetVTFID(const string::StringID& nodeName, uint32_t VTFID) override;
			virtual uint32_t GetPrevVTFID(const string::StringID& nodeName) const override;

		private:
			tsl::robin_map<string::StringID, math::Matrix> m_rmapPrevTransforms;
			tsl::robin_map<string::StringID, uint32_t> m_rmapPrevVTFIDs;
		};

		class ModelInstance : public IModelInstance
		{
		private:
			struct AttachmentNode
			{
				enum Type
				{
					eNone = 0,
					eBone,
				};

				uint32_t id{ 0 };
				ModelInstancePtr pInstance;
				string::StringID nodeName;
				Type emType{ Type::eNone };
				math::Matrix matOffset;

				AttachmentNode(uint32_t id, ModelInstancePtr pInstance, const string::StringID& nodeName, const math::Matrix& matOffset, Type emAttachNodeType);
			};

		public:
			ModelInstance(IModel* pModel);
			ModelInstance(ModelInstance&& source) noexcept;
			virtual ~ModelInstance();

			ModelInstance& operator = (ModelInstance&& source) noexcept;

		public:
			void UpdateTransformations();
			void UpdateModel();

		public:
			virtual void Update(float elapsedTime, const math::Matrix& matParent) override;

			virtual bool Attachment(uint32_t id, ModelInstancePtr pInstance, const string::StringID& nodeName, const math::Matrix& matOffset = math::Matrix::Identity) override;
			virtual bool Attachment(uint32_t id, ModelInstancePtr pInstance, const math::Matrix& matOffset = math::Matrix::Identity) override;
			virtual IModelInstance* GetAttachment(uint32_t id) const override;
			virtual size_t GetAttachmentCount() const override { return m_attachmentNode.size(); }
			virtual bool IsAttachment() const override { return m_isAttachment; }

			virtual bool Dettachment(uint32_t id) override;

		public:
			virtual bool IsLoadComplete() const override;

			virtual void SetVisible(bool isVisible) { m_isVisible = isVisible; }
			virtual bool IsVisible() const { return m_isVisible; }

			virtual void ChangeMaterial(const string::StringID& nodeName, uint32_t index, const MaterialPtr& pMaterial);

		public:
			virtual IModel* GetModel() override { return m_pModel; }
			virtual IMotionSystem* GetMotionSystem() override { return &m_motionSystem; }
			virtual ISkeletonInstance* GetSkeleton() override { return &m_skeletonInstance; }

		public:
			void LoadCompleteCallback(bool bSuccess);
			void SetAttachment(bool isAttachment) { m_isAttachment = isAttachment; }

		private:
			bool m_isVisible{ true };
			bool m_isAttachment{ false };

			IModel* m_pModel{ nullptr };

			float m_elapsedTime{ 0.f };

			math::Matrix m_matParent;

			MotionSystem m_motionSystem;
			SkeletonInstance m_skeletonInstance;
			MaterialInstance m_materialInstance;
			TransformInstance m_transformInstance;

			std::vector<AttachmentNode> m_attachmentNode;
		};
	}
}