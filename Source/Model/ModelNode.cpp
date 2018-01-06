#include "stdafx.h"
#include "ModelNode.h"

#include "GeometryModel.h"

#include "CommonLib/FileStream.h"
#include "CommonLib/Config.h"
#include "Renderer/RendererManager.h"

namespace EastEngine
{
	namespace Graphics
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
				SafeDelete(pVertexBuffer);
			}

			for (auto& pIndexBuffer : m_pIndexBuffer)
			{
				SafeDelete(pIndexBuffer);
			}

			std::for_each(m_vecMaterial.begin(), m_vecMaterial.end(), [](IMaterial* pMaterial)
			{
				IMaterial::Destroy(&pMaterial);
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

		IVertexBuffer* ModelNode::GetVertexBuffer(uint32_t nLod)
		{
			return m_pVertexBuffer[Math::Min(nLod, m_nLodMax)];
		}

		void ModelNode::SetVertexBuffer(IVertexBuffer* pVertexBuffer, uint32_t nLod)
		{
			m_pVertexBuffer[Math::Min(nLod, m_nLodMax)] = pVertexBuffer;
		}

		IIndexBuffer* ModelNode::GetIndexBuffer(uint32_t nLod)
		{
			return m_pIndexBuffer[Math::Min(nLod, m_nLodMax)];
		}

		void ModelNode::SetIndexBuffer(IIndexBuffer* pIndexBuffer, uint32_t nLod)
		{
			m_pIndexBuffer[Math::Min(nLod, m_nLodMax)] = pIndexBuffer;
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
			m_vecModelSubsets[Math::Min(nLod, m_nLodMax)].push_back(modelPiece);
		}

		void ModelNode::AddModelSubsets(const std::vector<ModelSubset>& vecModelPiece, uint32_t nLod)
		{
			std::copy(vecModelPiece.begin(), vecModelPiece.end(), std::back_inserter(m_vecModelSubsets[Math::Min(nLod, m_nLodMax)]));
		}

		void ModelNode::BuildBoundingBox(const Collision::AABB& aabb)
		{
			m_originAABB = aabb;
			m_originAABB.Extents = Math::Vector3::Max(m_originAABB.Extents, Math::Vector3(0.01f));
		}
	}
}