#include "stdafx.h"
#include "ModelInstance.h"

#include "Model.h"
#include "ModelManager.h"

namespace est
{
	namespace graphics
	{
		MaterialInstance::MaterialInstance()
		{
		}

		MaterialInstance::~MaterialInstance()
		{
			for (auto& nodeMaterial : m_materials)
			{
				ReleaseResource(nodeMaterial.pMaterial);
			}
			m_materials.clear();
		}

		MaterialPtr MaterialInstance::GetMaterial(const string::StringID& nodeName, uint32_t index) const
		{
			auto iter = std::find_if(m_materials.begin(), m_materials.end(), [nodeName, index](const NodeMaterial& nodeMaterial)
			{
				return nodeMaterial.nodeName == nodeName && nodeMaterial.index == index;
			});

			if (iter != m_materials.end())
				return iter->pMaterial;

			return nullptr;
		}

		void MaterialInstance::AddMaterial(const string::StringID& nodeName, uint32_t index, const MaterialPtr& pMaterial)
		{
			auto iter = std::find_if(m_materials.begin(), m_materials.end(), [nodeName, index](const NodeMaterial& nodeMaterial)
			{
				return nodeMaterial.nodeName == nodeName && nodeMaterial.index == index;
			});

			if (iter != m_materials.end())
				return;

			m_materials.emplace_back(nodeName, index, pMaterial);
		}

		MaterialInstance::NodeMaterial::NodeMaterial(const string::StringID& nodeName, uint32_t index, const MaterialPtr& pMaterial)
			: nodeName(nodeName)
			, index(index)
			, pMaterial(pMaterial)
		{
		}

		TransformInstance::TransformInstance()
		{
		}

		TransformInstance::~TransformInstance()
		{
		}

		void TransformInstance::SetTransform(const string::StringID& nodeName, const math::Matrix& matTransform)
		{
			m_rmapPrevTransforms[nodeName] = matTransform;
		}

		const math::Matrix& TransformInstance::GetPrevTransform(const string::StringID& nodeName) const
		{
			auto iter = m_rmapPrevTransforms.find(nodeName);
			if (iter != m_rmapPrevTransforms.end())
				return iter->second;

			return math::Matrix::Identity;
		}

		void TransformInstance::SetVTFID(const string::StringID& nodeName, uint32_t VTFID)
		{
			m_rmapPrevVTFIDs[nodeName] = VTFID;
		}

		uint32_t TransformInstance::GetPrevVTFID(const string::StringID& nodeName) const
		{
			auto iter = m_rmapPrevVTFIDs.find(nodeName);
			if (iter != m_rmapPrevVTFIDs.end())
				return iter->second;

			return IVTFManager::eInvalidVTFID;
		}

		ModelInstance::AttachmentNode::AttachmentNode(uint32_t id, ModelInstancePtr pInstance, const string::StringID& nodeName, const math::Matrix& matOffset, Type emType)
			: id(id)
			, pInstance(std::move(pInstance))
			, nodeName(nodeName)
			, matOffset(matOffset)
			, emType(emType)
		{
		}

		ModelInstance::ModelInstance(IModel* pModel)
			: m_pModel(pModel)
		{
		}

		ModelInstance::ModelInstance(ModelInstance&& source) noexcept
		{
			*this = std::move(source);
		}

		ModelInstance::~ModelInstance()
		{
		}

		ModelInstance& ModelInstance::operator = (ModelInstance&& source) noexcept
		{
			m_isVisible = std::move(source.m_isVisible);
			m_isAttachment = std::move(source.m_isAttachment);
			m_pModel = std::move(source.m_pModel);
			m_elapsedTime = std::move(source.m_elapsedTime);
			m_matParent = std::move(source.m_matParent);
			m_motionSystem = std::move(source.m_motionSystem);
			m_skeletonInstance = std::move(source.m_skeletonInstance);
			m_materialInstance = std::move(source.m_materialInstance);
			m_attachmentNode = std::move(source.m_attachmentNode);

			return *this;
		}

		void ModelInstance::UpdateTransformations()
		{
			TRACER_EVENT(__FUNCTIONW__);
			if (m_isVisible == false)
				return;

			if (IsLoadComplete() == false)
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
					ModelInstance* pModelInstance = static_cast<ModelInstance*>(node.pInstance.get());
					pModelInstance->Update(m_elapsedTime, node.matOffset * m_matParent);
					pModelInstance->UpdateModel();

					++iter;
					continue;
				}
				break;
				case AttachmentNode::Type::eBone:
				{
					ISkeletonInstance::IBone* pBone = m_skeletonInstance.GetBone(node.nodeName);
					if (pBone != nullptr)
					{
						ModelInstance* pModelInstance = static_cast<ModelInstance*>(node.pInstance.get());
						pModelInstance->Update(m_elapsedTime, node.matOffset * pBone->GetGlobalMatrix());
						pModelInstance->UpdateModel();

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

			UpdateTransformations();
			m_pModel->Update(m_elapsedTime, m_matParent, &m_skeletonInstance, &m_materialInstance, &m_transformInstance);
		}

		void ModelInstance::Update(float elapsedTime, const math::Matrix& matParent)
		{
			m_elapsedTime = elapsedTime;
			m_matParent = m_pModel->GetLocalMatrix() * matParent;
		}

		bool ModelInstance::Attachment(uint32_t id, ModelInstancePtr pInstance, const string::StringID& nodeName, const math::Matrix& matOffset)
		{
			if (IsLoadComplete() == false)
				return false;

			ISkeletonInstance::IBone* pBone = m_skeletonInstance.GetBone(nodeName);
			if (pBone == nullptr)
				return false;

			ModelInstance* pModelInstance = static_cast<ModelInstance*>(pInstance.get());
			pModelInstance->SetAttachment(true);

			m_attachmentNode.emplace_back(id, std::move(pInstance), nodeName, matOffset, AttachmentNode::Type::eBone);

			return true;
		}

		bool ModelInstance::Attachment(uint32_t id, ModelInstancePtr pInstance, const math::Matrix& matOffset)
		{
			if (IsLoadComplete() == false)
				return false;

			ModelInstance* pModelInstance = static_cast<ModelInstance*>(pInstance.get());
			pModelInstance->SetAttachment(true);

			m_attachmentNode.emplace_back(id, std::move(pInstance), sid::EmptyString, matOffset, AttachmentNode::Type::eNone);

			return true;
		}

		IModelInstance* ModelInstance::GetAttachment(uint32_t id) const
		{
			auto iter = std::find_if(m_attachmentNode.begin(), m_attachmentNode.end(), [id](const AttachmentNode& node)
			{
				return node.id == id;
			});

			if (iter == m_attachmentNode.end())
				return nullptr;

			return iter->pInstance.get();
		}

		bool ModelInstance::Dettachment(uint32_t id)
		{
			auto iter = std::find_if(m_attachmentNode.begin(), m_attachmentNode.end(), [id](const AttachmentNode& attachmentNode)
			{
				return attachmentNode.id == id;
			});

			if (iter != m_attachmentNode.end())
			{
				ModelInstance* pModelInstance = static_cast<ModelInstance*>(iter->pInstance.get());
				pModelInstance->SetAttachment(false);

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

		void ModelInstance::ChangeMaterial(const string::StringID& nodeName, uint32_t index, const MaterialPtr& pMaterial)
		{
			m_materialInstance.AddMaterial(nodeName, index, pMaterial);
		}
	}
}