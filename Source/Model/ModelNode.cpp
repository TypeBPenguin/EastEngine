#include "stdafx.h"
#include "ModelNode.h"

#include "GeometryModel.h"

#include "CommonLib/FileStream.h"
#include "CommonLib/Config.h"

namespace eastengine
{
	namespace graphics
	{
		ModelNode::ModelNode(EmModelNode::Type emModelNodeType)
			: m_emModelNodeType(emModelNodeType)
			, m_isVisible(true)
			, m_pParentNode(nullptr)
			, m_nLod(0)
			, m_nLodMax(0)
			, m_fDistanceFromCamera(0.f)
		{
			m_pVertexBuffer.fill(nullptr);
			m_pIndexBuffer.fill(nullptr);
		}

		ModelNode::~ModelNode()
		{
			for (auto& pVertexBuffer : m_pVertexBuffer)
			{
				ReleaseResource(&pVertexBuffer);
			}

			for (auto& pIndexBuffer : m_pIndexBuffer)
			{
				ReleaseResource(&pIndexBuffer);
			}

			std::for_each(m_vecMaterial.begin(), m_vecMaterial.end(), [](IMaterial* pMaterial)
			{
				ReleaseResource(&pMaterial);
			});
			m_vecMaterial.clear();

			size_t nSize = m_vecChildModelNode.size();
			for (size_t i = 0; i < nSize; ++i)
			{
				if (m_vecChildModelNode[i] != nullptr)
				{
					ModelNode* pModelNode = static_cast<ModelNode*>(m_vecChildModelNode[i]);
					SafeDelete(pModelNode);
				}
			}
			m_vecChildModelNode.clear();
		}

		IVertexBuffer* ModelNode::GetVertexBuffer(uint32_t nLod) const
		{
			return m_pVertexBuffer[math::Min(nLod, m_nLodMax)];
		}

		void ModelNode::SetVertexBuffer(IVertexBuffer* pVertexBuffer, uint32_t nLod)
		{
			ReleaseResource(&m_pVertexBuffer[math::Min(nLod, m_nLodMax)]);

			pVertexBuffer->IncreaseReference();
			m_pVertexBuffer[math::Min(nLod, m_nLodMax)] = pVertexBuffer;
		}

		IIndexBuffer* ModelNode::GetIndexBuffer(uint32_t nLod) const
		{
			return m_pIndexBuffer[math::Min(nLod, m_nLodMax)];
		}

		void ModelNode::SetIndexBuffer(IIndexBuffer* pIndexBuffer, uint32_t nLod)
		{
			ReleaseResource(&m_pIndexBuffer[math::Min(nLod, m_nLodMax)]);

			pIndexBuffer->IncreaseReference();
			m_pIndexBuffer[math::Min(nLod, m_nLodMax)] = pIndexBuffer;
		}

		void ModelNode::AddMaterial(IMaterial* pMaterial)
		{
			pMaterial->IncreaseReference();
			m_vecMaterial.push_back(pMaterial);
		}

		void ModelNode::AddMaterialArray(IMaterial** ppMaterials, size_t nCount)
		{
			m_vecMaterial.reserve(m_vecMaterial.size() + nCount);

			for (size_t i = 0; i < nCount; ++i)
			{
				ppMaterials[i]->IncreaseReference();
				m_vecMaterial.emplace_back(ppMaterials[i]);
			}
		}

		void ModelNode::AddModelSubset(ModelSubset& modelPiece, uint32_t nLod)
		{
			m_vecModelSubsets[math::Min(nLod, m_nLodMax)].push_back(modelPiece);
		}

		void ModelNode::AddModelSubsets(const std::vector<ModelSubset>& vecModelPiece, uint32_t nLod)
		{
			std::copy(vecModelPiece.begin(), vecModelPiece.end(), std::back_inserter(m_vecModelSubsets[math::Min(nLod, m_nLodMax)]));
		}

		IMaterial* ModelNode::GetMaterial(const String::StringID& strMaterialName, uint32_t& nMaterialID_out) const
		{
			const size_t nSize = m_vecMaterial.size();
			for (size_t i = 0; i < nSize; ++i)
			{
				if (m_vecMaterial[i]->GetName() == strMaterialName)
				{
					nMaterialID_out = static_cast<uint32_t>(i);
					return m_vecMaterial[i];
				}
			}

			nMaterialID_out = ModelSubset::eInvalidMaterialID;
			return nullptr;
		}

		void ModelNode::SetOriginAABB(const Collision::AABB& aabb)
		{
			m_originAABB = aabb;
			m_originAABB.Extents = math::Vector3::Max(m_originAABB.Extents, math::Vector3(0.01f));
		}
	}
}