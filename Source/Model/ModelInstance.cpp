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

		ModelInstance::RequestAttachmentNode::RequestAttachmentNode(IModelInstance* pInstance, const String::StringID& strNodeName)
			: pInstance(pInstance), strNodeName(strNodeName)
		{
		}

		ModelInstance::AttachmentNode::AttachmentNode(IModelInstance* pInstance, const Math::Matrix* pTargetMatrix)
			: pInstance(pInstance), pTargetMatrix(pTargetMatrix)
		{
		}

		ModelInstance::ModelInstance(IModel* pModel)
			: m_isVisible(true)
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

			bool isPlayingMotion = false;
			if (m_pMotionSystem != nullptr)
			{
				m_pMotionSystem->Update(m_fUpdateTimeData, m_pSkeletonInstance);

				isPlayingMotion = m_pMotionSystem->IsPlayingMotion();
			}

			if (m_pSkeletonInstance != nullptr)
			{
				m_pSkeletonInstance->Update(isPlayingMotion);
			}

			m_pModel->Update(m_fUpdateTimeData, m_matUpdateData, m_pSkeletonInstance, m_pMaterialInstance);

			for (const auto& node : m_vecAttachmentNode)
			{
				if (node.pTargetMatrix != nullptr)
				{
					node.pInstance->Update(m_fUpdateTimeData, *node.pTargetMatrix);
				}
			}
		}

		bool ModelInstance::Attachment(IModelInstance* pInstance, const String::StringID& strNodeName)
		{
			if (IsLoadComplete() == true)
			{
				m_listRequestAttachmentNode.emplace_back(pInstance, strNodeName);
			}
			else
			{
				IModelNode* pNode = m_pModel->GetNode(strNodeName);
				if (pNode == nullptr)
					return false;

				m_vecAttachmentNode.emplace_back(pInstance, pNode->GetWorldMatrixPtr());
			}

			return true;
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