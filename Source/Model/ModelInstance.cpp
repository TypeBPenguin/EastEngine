#include "stdafx.h"
#include "ModelInstance.h"

#include "Model.h"

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

		ModelInstance::RequestAttachmentNode::RequestAttachmentNode(ModelInstance* pInstance, const String::StringID& strNodeName)
			: pInstance(pInstance), strNodeName(strNodeName)
		{
		}

		ModelInstance::AttachmentNode::AttachmentNode(ModelInstance* pInstance, const Math::Matrix* pTargetMatrix)
			: pInstance(pInstance), pTargetMatrix(pTargetMatrix)
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

		void ModelInstance::Process()
		{
			if (m_isVisible == false)
				return;

			if (IsLoadComplete() == false)
				return;

			if (m_pMotionSystem != nullptr)
			{
				m_pMotionSystem->Update(m_fElapsedTime, m_pSkeletonInstance);
			}

			if (m_pSkeletonInstance != nullptr)
			{
				m_pSkeletonInstance->Update(m_matParent);
			}

			m_pModel->Update(m_fElapsedTime, m_matParent, m_pSkeletonInstance, m_pMaterialInstance);

			for (const auto& node : m_vecAttachmentNode)
			{
				if (node.pTargetMatrix != nullptr)
				{
					node.pInstance->Update(m_fElapsedTime, *node.pTargetMatrix);
					node.pInstance->Process();
				}
			}
		}

		void ModelInstance::Update(float fElapsedTime, const Math::Matrix& matParent)
		{
			m_fElapsedTime = fElapsedTime;
			m_matParent = matParent;
		}

		bool ModelInstance::Attachment(IModelInstance* pInstance, const String::StringID& strNodeName)
		{
			if (IsLoadComplete() == false)
			{
				ModelInstance* pModelInstance = static_cast<ModelInstance*>(pInstance);
				m_listRequestAttachmentNode.emplace_back(pModelInstance, strNodeName);

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
						m_vecAttachmentNode.emplace_back(pModelInstance, &pBone->GetGlobalTransform());
						return true;
					}
				}

				IModelNode* pNode = m_pModel->GetNode(strNodeName);
				if (pNode == nullptr)
					return false;

				ModelInstance* pModelInstance = static_cast<ModelInstance*>(pInstance);
				m_vecAttachmentNode.emplace_back(pModelInstance, pNode->GetWorldMatrixPtr());

				return true;
			}
		}

		bool ModelInstance::Dettachment(IModelInstance* pInstance)
		{
			if (IsLoadComplete() == false)
			{
				auto iter = std::find_if(m_listRequestAttachmentNode.begin(), m_listRequestAttachmentNode.end(), [&pInstance](const RequestAttachmentNode& requestAttachmentNode)
				{
					return requestAttachmentNode.pInstance == pInstance;
				});

				if (iter != m_listRequestAttachmentNode.end())
				{
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
					m_vecAttachmentNode.erase(iter);
					return true;
				}

				return false;
			}
		}

		void ModelInstance::PlayMotion(IMotion* pMotion, const MotionState* pMotionState)
		{
			if (m_pMotionSystem == nullptr)
				return;

			m_pMotionSystem->Play(pMotion, pMotionState);
		}

		void ModelInstance::StopMotion(float fStopTime)
		{
			if (m_pMotionSystem == nullptr)
				return;

			m_pMotionSystem->Stop(fStopTime);
		}

		IMotion* ModelInstance::GetMotion()
		{
			if (m_pMotionSystem == nullptr)
				return nullptr;

			return m_pMotionSystem->GetMotion();
		}

		void ModelInstance::LoadCompleteCallback(bool isSuccess)
		{
			if (isSuccess == true)
			{
				if (m_pModel->IsSkinningModel() == true)
				{
					m_pMotionSystem = IMotionSystem::Create(m_pModel);
					m_pSkeletonInstance = ISkeleton::CreateInstance(m_pModel->GetSkeleton());
				}

				for (const auto& node : m_listRequestAttachmentNode)
				{
					Attachment(node.pInstance, node.strNodeName);
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