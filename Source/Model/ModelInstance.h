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
				uint32_t nIndex = UINT32_MAX;
				IMaterial* pMaterial = nullptr;

				NodeMaterial(const String::StringID& strNodeName, uint32_t nIndex, IMaterial* pMaterial);
			};

			std::vector<NodeMaterial> m_vecMaterials;
		};

		class ModelInstance : public IModelInstance
		{
		private:
			struct RequestAttachmentNode
			{
				IModelInstance* pInstance = nullptr;
				String::StringID strNodeName;

				RequestAttachmentNode(IModelInstance* pInstance, const String::StringID& strNodeName);
			};

			struct AttachmentNode
			{
				IModelInstance* pInstance = nullptr;
				const Math::Matrix* pTargetMatrix = nullptr;

				AttachmentNode(IModelInstance* pInstance, const Math::Matrix* pTargetMatrix);
			};

		public:
			ModelInstance(IModel* pModel);
			virtual ~ModelInstance();

		public:
			void Process();

		public:
			virtual void Update(float fElapsedTime, const Math::Matrix& matParent) override
			{
				m_fUpdateTimeData = fElapsedTime;
				m_matUpdateData = matParent;
			}

			virtual bool Attachment(IModelInstance* pInstance, const String::StringID& strNodeName) override;
			virtual IModelInstance* GetAttachment(size_t nIndex) const override { return m_vecAttachmentNode[nIndex].pInstance; }
			virtual size_t GetAttachmentCount() const override { return m_vecAttachmentNode.size(); }

			virtual void PlayMotion(IMotion* pMotion, const MotionState* pMotionState = nullptr) override;
			virtual void StopMotion(float fStopTime) override;
			virtual IMotion* GetMotion() override;

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

		private:
			bool m_isVisible;

			float m_fUpdateTimeData;
			Math::Matrix m_matUpdateData;

			Math::Matrix m_matWorld;

			IModel* m_pModel;
			IMotionSystem* m_pMotionSystem;

			ISkeletonInstance* m_pSkeletonInstance;
			MaterialInstance* m_pMaterialInstance;

			std::vector<AttachmentNode> m_vecAttachmentNode;
			std::list<RequestAttachmentNode> m_listRequestAttachmentNode;
		};
	}
}