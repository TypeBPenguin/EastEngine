#pragma once

#include "ModelInterface.h"

#include "Skeleton.h"
#include "MotionSystem.h"

namespace eastengine
{
	namespace graphics
	{
		class MaterialInstance : public IMaterialInstance
		{
		public:
			MaterialInstance();
			virtual ~MaterialInstance();

		public:
			virtual IMaterial* GetMaterial(const string::StringID& strNodeName, uint32_t nIndex) const override;

		public:
			void AddMaterial(const string::StringID& strNodeName, uint32_t nIndex, IMaterial* pMaterial);

		private:
			struct NodeMaterial
			{
				string::StringID strNodeName;
				uint32_t nIndex = std::numeric_limits<uint32_t>::max();
				IMaterial* pMaterial = nullptr;

				NodeMaterial(const string::StringID& strNodeName, uint32_t nIndex, IMaterial* pMaterial);
			};

			std::vector<NodeMaterial> m_vecMaterials;
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

				ModelInstance* pInstance{ nullptr };
				string::StringID nodeName;
				Type emType{ Type::eNone };
				math::Matrix matOffset;

				AttachmentNode(ModelInstance* pInstance, const string::StringID& nodeName, const math::Matrix& matOffset, Type emAttachNodeType);
			};

		public:
			ModelInstance(IModel* pModel);
			ModelInstance(const ModelInstance& source);
			ModelInstance(ModelInstance&& source) noexcept;
			virtual ~ModelInstance();

			ModelInstance& operator = (const ModelInstance& source);
			ModelInstance& operator = (ModelInstance&& source) noexcept;

		public:
			void UpdateTransformations();
			void UpdateModel();

		public:
			virtual void Update(float elapsedTime, const math::Matrix& matParent) override;

			virtual bool Attachment(IModelInstance* pInstance, const string::StringID& strNodeName, const math::Matrix& matOffset = math::Matrix::Identity) override;
			virtual bool Attachment(IModelInstance* pInstance, const math::Matrix& matOffset = math::Matrix::Identity) override;
			virtual IModelInstance* GetAttachment(size_t nIndex) const override { return m_attachmentNode[nIndex].pInstance; }
			virtual size_t GetAttachmentCount() const override { return m_attachmentNode.size(); }
			virtual bool IsAttachment() const override { return m_isAttachment; }

			virtual bool Dettachment(IModelInstance* pInstance) override;

		public:
			virtual bool IsLoadComplete() const override;

			virtual void SetVisible(bool isVisible) { m_isVisible = isVisible; }
			virtual bool IsVisible() const { return m_isVisible; }

			virtual void ChangeMaterial(const string::StringID& strNodeName, uint32_t nIndex, IMaterial* pMaterial);

		public:
			virtual IModel* GetModel() override { return m_pModel; }
			virtual IMotionSystem* GetMotionSystem() override { return &m_motionSystem; }
			virtual ISkeletonInstance* GetSkeleton() override { return &m_skeletonInstance; }

			virtual const math::Matrix& GetWorldMatrix() const { return m_matWorld; }

		public:
			void LoadCompleteCallback(bool bSuccess);
			void SetAttachment(bool isAttachment) { m_isAttachment = isAttachment; }

		private:
			bool m_isVisible{ true };
			bool m_isAttachment{ false };

			IModel* m_pModel{ nullptr };

			float m_elapsedTime{ 0.f };

			math::Matrix m_matParent;
			math::Matrix m_matWorld;

			MotionSystem m_motionSystem;
			SkeletonInstance m_skeletonInstance;
			MaterialInstance m_materialInstance;

			std::vector<AttachmentNode> m_attachmentNode;
		};
	}
}