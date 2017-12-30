#include "stdafx.h"
#include "ModelNode.h"

#include "CommonLib/FileStream.h"

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
			Memory::Clear(&m_pVertexBuffer, sizeof(m_pVertexBuffer));
			Memory::Clear(&m_pIndexBuffer, sizeof(m_pIndexBuffer));

			Math::Vector3 f3Min(-Math::Vector3::One);
			Math::Vector3 f3Max(Math::Vector3::One);

			Collision::AABB::CreateFromPoints(m_originAABB, f3Min, f3Max);

			Collision::AABB::CreateFromPoints(m_boundingAABB, f3Min, f3Max);
			Collision::Sphere::CreateFromAABB(m_boundingSphere, m_boundingAABB);
			Collision::OBB::CreateFromAABB(m_boundingOBB, m_boundingAABB);
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

		void ModelNode::AddMaterialArray(IMaterial** ppMaterials, uint32_t nCount)
		{
			m_vecMaterial.reserve(m_vecMaterial.size() + nCount);

			for (uint32_t i = 0; i < nCount; ++i)
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

			m_boundingAABB = m_originAABB;
			Collision::Sphere::CreateFromAABB(m_boundingSphere, m_originAABB);
			Collision::OBB::CreateFromAABB(m_boundingOBB, m_originAABB);
		}

		void ModelNode::UpdateBoundingBox()
		{
			BuildBoundingBox(m_originAABB);

			m_boundingAABB.Transform(m_boundingAABB, m_matWorld);
			m_boundingOBB.Transform(m_boundingOBB, m_matWorld);
			m_boundingSphere.Transform(m_boundingSphere, m_matWorld);
		}
	}
}