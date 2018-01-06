#include "stdafx.h"
#include "ModelInstance.h"

#include "Model.h"
#include "ModelManager.h"

namespace EastEngine
{
	namespace Graphics
	{
		MaterialInstance::MaterialInstance()
		{
		}

		MaterialInstance::~MaterialInstance()
		{
			for (auto& nodeMaterial : m_vecMaterials)
			{
				IMaterial::Destroy(&nodeMaterial.pMaterial);
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

		ModelInstance::AttachmentNode::AttachmentNode(ModelInstance* pInstance, const String::StringID& strNodeName, const Math::Matrix& matOffset, EmAttachNodeType emAttachNodeType)
			: pInstance(pInstance)
			, strNodeName(strNodeName)
			, matOffset(matOffset)
			, emAttachNodeType(emAttachNodeType)
		{
		}

		ModelInstance::ModelInstance(IModel* pModel)
			: m_isVisible(true)
			, m_isAttachment(false)
			, m_fElapsedTime(0.f)
			, m_pModel(pModel)
			, m_pMotionSystem(nullptr)
			, m_pSkeletonInstance(nullptr)
			, m_pMaterialInstance(new MaterialInstance)
		{
		}

		ModelInstance::~ModelInstance()
		{
			IMotionSystem::Destroy(&m_pMotionSystem);
			ISkeleton::DestroyInstance(&m_pSkeletonInstance);
			SafeDelete(m_pMaterialInstance);
			m_pModel = nullptr;
		}

		void ModelInstance::Ready()
		{
			if (m_isVisible == false)
				return;

			if (IsLoadComplete() == false)
				return;

			if (IsAttachment() == true)
				return;

			if (m_pModel->GetSkeleton() != nullptr)
			{
				ModelManager::GetInstance()->PushJobUpdateTransformations(this);
			}

			ModelManager::GetInstance()->PushJobUpdateModels(this);
		}

		void ModelInstance::UpdateTransformations()
		{
			m_pMotionSystem->Update(m_fElapsedTime);
			m_pSkeletonInstance->Update(m_matParent);

			for (const auto& node : m_vecAttachmentNode)
			{
				if (node.emAttachNodeType == AttachmentNode::EmAttachNodeType::eBone)
				{
					ISkeletonInstance::IBone* pBone = m_pSkeletonInstance->GetBone(node.strNodeName);
					if (pBone != nullptr)
					{
						node.pInstance->Update(m_fElapsedTime, node.matOffset * pBone->GetGlobalTransform());
						node.pInstance->UpdateModel();
					}
				}
			}
		}

		void ModelInstance::UpdateModel()
		{
			m_pModel->Update(m_fElapsedTime, m_matParent, m_pSkeletonInstance, m_pMaterialInstance);
		}

		void ModelInstance::Update(float fElapsedTime, const Math::Matrix& matParent)
		{
			m_fElapsedTime = fElapsedTime;
			m_matParent = matParent;
		}

		bool ModelInstance::Attachment(IModelInstance* pInstance, const String::StringID& strNodeName, const Math::Matrix& matOffset)
		{
			if (IsLoadComplete() == false)
			{
				ModelInstance* pModelInstance = static_cast<ModelInstance*>(pInstance);
				pModelInstance->SetAttachment(true);

				m_listRequestAttachmentNode.emplace_back(pModelInstance, strNodeName, matOffset, AttachmentNode::EmAttachNodeType::eNone);

				return true;
			}
			else
			{
				if (m_pSkeletonInstance != nullptr)
				{
					ISkeletonInstance::IBone* pBone =  m_pSkeletonInstance->GetBone(strNodeName);
					if (pBone != nullptr)
					{
						ModelInstance* pModelInstance = static_cast<ModelInstance*>(pInstance);
						pModelInstance->SetAttachment(true);

						m_vecAttachmentNode.emplace_back(pModelInstance, strNodeName, matOffset, AttachmentNode::EmAttachNodeType::eBone);

						return true;
					}
				}

				return true;
			}
		}

		bool ModelInstance::Dettachment(IModelInstance* pInstance)
		{
			if (IsLoadComplete() == false)
			{
				auto iter = std::find_if(m_listRequestAttachmentNode.begin(), m_listRequestAttachmentNode.end(), [&pInstance](const AttachmentNode& requestAttachmentNode)
				{
					return requestAttachmentNode.pInstance == pInstance;
				});

				if (iter != m_listRequestAttachmentNode.end())
				{
					iter->pInstance->SetAttachment(false);

					m_listRequestAttachmentNode.erase(iter);
					return true;
				}

				return false;
			}
			else
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
		}

		void ModelInstance::LoadCompleteCallback(bool isSuccess)
		{
			if (isSuccess == true)
			{
				if (m_pModel->GetSkeleton() != nullptr)
				{
					m_pSkeletonInstance = ISkeleton::CreateInstance(m_pModel->GetSkeleton());
					m_pMotionSystem = IMotionSystem::Create(m_pSkeletonInstance);
				}

				for (const auto& node : m_listRequestAttachmentNode)
				{
					Attachment(node.pInstance, node.strNodeName, node.matOffset);
				}
				m_listRequestAttachmentNode.clear();
			}
		}

		bool ModelInstance::IsLoadComplete()
		{
			return m_pModel->GetLoadState() == EmLoadState::eComplete;
		}

		void ModelInstance::ChangeMaterial(const String::StringID& strNodeName, uint32_t nIndex, IMaterial* pMaterial)
		{
			if (m_pMaterialInstance == nullptr)
				return;

			m_pMaterialInstance->AddMaterial(strNodeName, nIndex, pMaterial);
		}
	}
}