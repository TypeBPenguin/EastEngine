#pragma once

#include "ModelInterface.h"

namespace EastEngine
{
	namespace Graphics
	{
		class MaterialInstance : public IMaterialInstance
		{
		public:
			MaterialInstance();
			virtual ~MaterialInstance();

		public:
			virtual IMaterial* GetMaterial(const String::StringID& strNodeName, uint32_t nIndex) const override;

		public:
			void AddMaterial(const String::StringID& strNodeName, uint32_t nIndex, IMaterial* pMaterial);

		private:
			struct NodeMaterial
			{
				String::StringID strNodeName;
				uint32_t nIndex = std::numeric_limits<uint32_t>::max();
				IMaterial* pMaterial = nullptr;

				NodeMaterial(const String::StringID& strNodeName, uint32_t nIndex, IMaterial* pMaterial);
			};

			std::vector<NodeMaterial> m_vecMaterials;
		};

		class ModelInstance : public IModelInstance
		{
		private:
			struct AttachmentNode
			{
				enum EmAttachNodeType
				{
					eNone = 0,
					eBone,
				};

				ModelInstance* pInstance = nullptr;
				String::StringID strNodeName;
				EmAttachNodeType emAttachNodeType = EmAttachNodeType::eNone;
				Math::Matrix matOffset;

				AttachmentNode(ModelInstance* pInstance, const String::StringID& strNodeName, const Math::Matrix& matOffset, EmAttachNodeType emAttachNodeType);
			};

		public:
			ModelInstance(IModel* pModel);
			virtual ~ModelInstance();

		public:
			void Ready();

			void UpdateTransformations();
			void UpdateModel();

		public:
			virtual void Update(float fElapsedTime, const Math::Matrix& matParent) override;

			virtual bool Attachment(IModelInstance* pInstance, const String::StringID& strNodeName, const Math::Matrix& matOffset) override;
			virtual IModelInstance* GetAttachment(size_t nIndex) const override { return m_vecAttachmentNode[nIndex].pInstance; }
			virtual size_t GetAttachmentCount() const override { return m_vecAttachmentNode.size(); }
			virtual bool IsAttachment() const override { return m_isAttachment; }

			virtual bool Dettachment(IModelInstance* pInstance) override;

		public:
			virtual bool IsLoadComplete() override;

			virtual void SetVisible(bool isVisible) { m_isVisible = isVisible; }
			virtual bool IsVisible() { return m_isVisible; }

			virtual void ChangeMaterial(const String::StringID& strNodeName, uint32_t nIndex, IMaterial* pMaterial);

		public:
			virtual IModel* GetModel() override { return m_pModel; }
			virtual IMotionSystem* GetMotionSystem() override { return m_pMotionSystem; }
			virtual ISkeletonInstance* GetSkeleton() override { return m_pSkeletonInstance; }

			virtual const Math::Matrix& GetWorldMatrix() { return m_matWorld; }

		public:
			void LoadCompleteCallback(bool bSuccess);
			void SetAttachment(bool isAttachment) { m_isAttachment = isAttachment; }

		private:
			bool m_isVisible;
			bool m_isAttachment;

			float m_fElapsedTime;
			Math::Matrix m_matParent;

			Math::Matrix m_matWorld;

			IModel* m_pModel;
			IMotionSystem* m_pMotionSystem;

			ISkeletonInstance* m_pSkeletonInstance;
			MaterialInstance* m_pMaterialInstance;

			std::vector<AttachmentNode> m_vecAttachmentNode;
			std::list<AttachmentNode> m_listRequestAttachmentNode;
		};
	}
}