#include "stdafx.h"
#include "ModelInstance.h"

#include "Model.h"
#include "ModelManager.h"

namespace eastengine
{
	namespace graphics
	{
		MaterialInstance::MaterialInstance()
		{
		}

		MaterialInstance::~MaterialInstance()
		{
			for (auto& nodeMaterial : m_vecMaterials)
			{
				ReleaseResource(&nodeMaterial.pMaterial);
			}
			m_vecMaterials.clear();
		}

		IMaterial* MaterialInstance::GetMaterial(const String::StringID& strNodeName, uint32_t nIndex) const
		{
			auto iter = std::find_if(m_vecMaterials.begin(), m_vecMaterials.end(), [strNodeName, nIndex](const NodeMaterial& nodeMaterial)
			{
				return nodeMaterial.strNodeName == strNodeName && nodeMaterial.nIndex == nIndex;
			});

			if (iter != m_vecMaterials.end())
				return iter->pMaterial;

			return nullptr;
		}

		void MaterialInstance::AddMaterial(const String::StringID& strNodeName, uint32_t nIndex, IMaterial* pMaterial)
		{
			pMaterial->IncreaseReference();

			m_vecMaterials.emplace_back(strNodeName, nIndex, pMaterial);
		}

		MaterialInstance::NodeMaterial::NodeMaterial(const String::StringID& strNodeName, uint32_t nIndex, IMaterial* pMaterial)
			: strNodeName(strNodeName)
			, nIndex(nIndex)
			, pMaterial(pMaterial)
		{
		}

		ModelInstance::AttachmentNode::AttachmentNode(ModelInstance* pInstance, const String::StringID& strNodeName, const math::Matrix& matOffset, Type emType)
			: pInstance(pInstance)
			, strNodeName(strNodeName)
			, matOffset(matOffset)
			, emType(emType)
		{
		}

		ModelInstance::ModelInstance(IModel* pModel)
			: m_isVisible(true)
			, m_isAttachment(false)
			, m_fElapsedTime(0.f)
			, m_pModel(pModel)
		{
		}

		ModelInstance::~ModelInstance()
		{
			m_pModel = nullptr;
		}

		void ModelInstance::UpdateTransformations()
		{
			if (m_isVisible == false)
				return;

			if (IsLoadComplete() == false)
				return;

			if (IsAttachment() == true)
				return;

			if (m_skeletonInstance.IsValid() == true)
			{
				m_motionSystem.Update(m_fElapsedTime);
				m_skeletonInstance.Update(m_matParent);
			}

			for (auto iter = m_vecAttachmentNode.begin(); iter != m_vecAttachmentNode.end();)
			{
				AttachmentNode& node = *iter;
				switch (node.emType)
				{
				case AttachmentNode::Type::eNone:
				{
					node.pInstance->Update(m_fElapsedTime, node.matOffset * m_matParent);
					node.pInstance->UpdateModel();

					++iter;
					continue;
				}
				break;
				case AttachmentNode::Type::eBone:
				{
					ISkeletonInstance::IBone* pBone = m_skeletonInstance.GetBone(node.strNodeName);
					if (pBone != nullptr)
					{
						node.pInstance->Update(m_fElapsedTime, node.matOffset * pBone->GetGlobalMatrix());
						node.pInstance->UpdateModel();

						++iter;

						continue;
					}
				}
				break;
				}

				iter = m_vecAttachmentNode.erase(iter);
			}
		}

		void ModelInstance::UpdateModel()
		{
			if (m_isVisible == false)
				return;

			if (IsLoadComplete() == false)
				return;

			if (IsAttachment() == true)
				return;

			m_pModel->Update(m_fElapsedTime, m_matParent, &m_skeletonInstance, &m_materialInstance);
		}

		void ModelInstance::Update(float fElapsedTime, const math::Matrix& matParent)
		{
			m_fElapsedTime = fElapsedTime;
			m_matParent = m_pModel->GetLocalMatrix() * matParent;
		}

		bool ModelInstance::Attachment(IModelInstance* pInstance, const String::StringID& strNodeName, const math::Matrix& matOffset)
		{
			if (IsLoadComplete() == false)
				return false;

			ISkeletonInstance::IBone* pBone = m_skeletonInstance.GetBone(strNodeName);
			if (pBone != nullptr)
			{
				ModelInstance* pModelInstance = static_cast<ModelInstance*>(pInstance);
				pModelInstance->SetAttachment(true);

				m_vecAttachmentNode.emplace_back(pModelInstance, strNodeName, matOffset, AttachmentNode::Type::eBone);

				return true;
			}

			return false;
		}

		bool ModelInstance::Attachment(IModelInstance* pInstance, const math::Matrix& matOffset)
		{
			if (IsLoadComplete() == false)
				return false;

			ModelInstance* pModelInstance = static_cast<ModelInstance*>(pInstance);
			pModelInstance->SetAttachment(true);

			m_vecAttachmentNode.emplace_back(pModelInstance, StrID::Unregistered, matOffset, AttachmentNode::Type::eNone);

			return true;
		}

		bool ModelInstance::Dettachment(IModelInstance* pInstance)
		{
			auto iter = std::find_if(m_vecAttachmentNode.begin(), m_vecAttachmentNode.end(), [&pInstance](const AttachmentNode& attachmentNode)
			{
				return attachmentNode.pInstance == pInstance;
			});

			if (iter != m_vecAttachmentNode.end())
			{
				iter->pInstance->SetAttachment(false);

				m_vecAttachmentNode.erase(iter);
				return true;
			}

			return false;
		}

		void ModelInstance::LoadCompleteCallback(bool isSuccess)
		{
			if (isSuccess == true)
			{
				if (m_pModel->GetSkeleton() != nullptr)
				{
					m_skeletonInstance.Initialize(m_pModel->GetSkeleton());
					m_motionSystem.Initialize(&m_skeletonInstance);
				}
			}
		}

		bool ModelInstance::IsLoadComplete() const
		{
			return m_pModel->GetState() == IResource::eComplete;
		}

		void ModelInstance::ChangeMaterial(const String::StringID& strNodeName, uint32_t nIndex, IMaterial* pMaterial)
		{
			m_materialInstance.AddMaterial(strNodeName, nIndex, pMaterial);
		}
	}
}