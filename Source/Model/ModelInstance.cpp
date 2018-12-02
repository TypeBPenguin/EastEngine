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

		IMaterial* MaterialInstance::GetMaterial(const string::StringID& strNodeName, uint32_t nIndex) const
		{
			auto iter = std::find_if(m_vecMaterials.begin(), m_vecMaterials.end(), [strNodeName, nIndex](const NodeMaterial& nodeMaterial)
			{
				return nodeMaterial.strNodeName == strNodeName && nodeMaterial.nIndex == nIndex;
			});

			if (iter != m_vecMaterials.end())
				return iter->pMaterial;

			return nullptr;
		}

		void MaterialInstance::AddMaterial(const string::StringID& strNodeName, uint32_t nIndex, IMaterial* pMaterial)
		{
			pMaterial->IncreaseReference();

			m_vecMaterials.emplace_back(strNodeName, nIndex, pMaterial);
		}

		MaterialInstance::NodeMaterial::NodeMaterial(const string::StringID& strNodeName, uint32_t nIndex, IMaterial* pMaterial)
			: strNodeName(strNodeName)
			, nIndex(nIndex)
			, pMaterial(pMaterial)
		{
		}

		ModelInstance::AttachmentNode::AttachmentNode(ModelInstance* pInstance, const string::StringID& nodeName, const math::Matrix& matOffset, Type emType)
			: pInstance(pInstance)
			, nodeName(nodeName)
			, matOffset(matOffset)
			, emType(emType)
		{
		}

		ModelInstance::ModelInstance(IModel* pModel)
			: m_pModel(pModel)
		{
		}

		ModelInstance::ModelInstance(const ModelInstance& source)
		{
			*this = source;
		}

		ModelInstance::ModelInstance(ModelInstance&& source) noexcept
		{
			*this = std::move(source);
		}

		ModelInstance::~ModelInstance()
		{
		}

		ModelInstance& ModelInstance::operator = (const ModelInstance& source)
		{
			m_isVisible = source.m_isVisible;
			m_isAttachment = source.m_isAttachment;
			m_pModel = source.m_pModel;
			m_elapsedTime = source.m_elapsedTime;
			m_matParent = source.m_matParent;
			m_matWorld = source.m_matWorld;
			m_motionSystem = source.m_motionSystem;
			m_skeletonInstance = source.m_skeletonInstance;
			m_materialInstance = source.m_materialInstance;
			m_attachmentNode = source.m_attachmentNode;

			return *this;
		}

		ModelInstance& ModelInstance::operator = (ModelInstance&& source) noexcept
		{
			m_isVisible = std::move(source.m_isVisible);
			m_isAttachment = std::move(source.m_isAttachment);
			m_pModel = std::move(source.m_pModel);
			m_elapsedTime = std::move(source.m_elapsedTime);
			m_matParent = std::move(source.m_matParent);
			m_matWorld = std::move(source.m_matWorld);
			m_motionSystem = std::move(source.m_motionSystem);
			m_skeletonInstance = std::move(source.m_skeletonInstance);
			m_materialInstance = std::move(source.m_materialInstance);
			m_attachmentNode = std::move(source.m_attachmentNode);

			return *this;
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
				m_motionSystem.Update(m_elapsedTime);
				m_skeletonInstance.Update(m_matParent);
			}

			for (auto iter = m_attachmentNode.begin(); iter != m_attachmentNode.end();)
			{
				AttachmentNode& node = *iter;
				switch (node.emType)
				{
				case AttachmentNode::Type::eNone:
				{
					node.pInstance->Update(m_elapsedTime, node.matOffset * m_matParent);
					node.pInstance->UpdateModel();

					++iter;
					continue;
				}
				break;
				case AttachmentNode::Type::eBone:
				{
					ISkeletonInstance::IBone* pBone = m_skeletonInstance.GetBone(node.nodeName);
					if (pBone != nullptr)
					{
						node.pInstance->Update(m_elapsedTime, node.matOffset * pBone->GetGlobalMatrix());
						node.pInstance->UpdateModel();

						++iter;

						continue;
					}
				}
				break;
				}

				iter = m_attachmentNode.erase(iter);
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

			UpdateTransformations();
			m_pModel->Update(m_elapsedTime, m_matParent, &m_skeletonInstance, &m_materialInstance);
		}

		void ModelInstance::Update(float elapsedTime, const math::Matrix& matParent)
		{
			m_elapsedTime = elapsedTime;
			m_matParent = m_pModel->GetLocalMatrix() * matParent;
		}

		bool ModelInstance::Attachment(IModelInstance* pInstance, const string::StringID& strNodeName, const math::Matrix& matOffset)
		{
			if (IsLoadComplete() == false)
				return false;

			ISkeletonInstance::IBone* pBone = m_skeletonInstance.GetBone(strNodeName);
			if (pBone != nullptr)
			{
				ModelInstance* pModelInstance = static_cast<ModelInstance*>(pInstance);
				pModelInstance->SetAttachment(true);

				m_attachmentNode.emplace_back(pModelInstance, strNodeName, matOffset, AttachmentNode::Type::eBone);

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

			m_attachmentNode.emplace_back(pModelInstance, StrID::EmptyString, matOffset, AttachmentNode::Type::eNone);

			return true;
		}

		bool ModelInstance::Dettachment(IModelInstance* pInstance)
		{
			auto iter = std::find_if(m_attachmentNode.begin(), m_attachmentNode.end(), [&pInstance](const AttachmentNode& attachmentNode)
			{
				return attachmentNode.pInstance == pInstance;
			});

			if (iter != m_attachmentNode.end())
			{
				iter->pInstance->SetAttachment(false);

				m_attachmentNode.erase(iter);
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

		void ModelInstance::ChangeMaterial(const string::StringID& strNodeName, uint32_t nIndex, IMaterial* pMaterial)
		{
			m_materialInstance.AddMaterial(strNodeName, nIndex, pMaterial);
		}
	}
}