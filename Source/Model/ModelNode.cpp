#include "stdafx.h"
#include "ModelNode.h"

#include "GeometryModel.h"

#include "CommonLib/FileStream.h"

namespace eastengine
{
	namespace graphics
	{
		ModelNode::ModelNode(LOD emMaxLod)
			: m_emMaxLod(emMaxLod)
		{
		}

		ModelNode::~ModelNode()
		{
			for (auto& pVertexBuffer : m_pVertexBuffer)
			{
				ReleaseResource(&pVertexBuffer);
			}
			m_pVertexBuffer.fill(nullptr);

			for (auto& pIndexBuffer : m_pIndexBuffer)
			{
				ReleaseResource(&pIndexBuffer);
			}
			m_pIndexBuffer.fill(nullptr);

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

		IVertexBuffer* ModelNode::GetVertexBuffer(LOD emLod) const
		{
			assert(emLod <= m_emMaxLod);
			return m_pVertexBuffer[emLod];
		}

		void ModelNode::SetVertexBuffer(IVertexBuffer* pVertexBuffer, LOD emLod)
		{
			assert(emLod <= m_emMaxLod);
			ReleaseResource(&m_pVertexBuffer[emLod]);

			pVertexBuffer->IncreaseReference();
			m_pVertexBuffer[emLod] = pVertexBuffer;
		}

		IIndexBuffer* ModelNode::GetIndexBuffer(LOD emLod) const
		{
			assert(emLod <= m_emMaxLod);
			return m_pIndexBuffer[emLod];
		}

		void ModelNode::GetRawVertices(const VertexPos** ppVertices, size_t& vertexCount) const
		{
			if (ppVertices == nullptr)
				return;

			*ppVertices = m_rawVertices.data();
			vertexCount = m_rawVertices.size();
		}

		void ModelNode::GetRawIndices(const uint32_t** ppIndices, size_t& indexCount) const
		{
			if (ppIndices == nullptr)
				return;

			*ppIndices = m_rawIndices.data();
			indexCount = m_rawIndices.size();
		}

		void ModelNode::SetIndexBuffer(IIndexBuffer* pIndexBuffer, LOD emLod)
		{
			assert(emLod <= m_emMaxLod);
			ReleaseResource(&m_pIndexBuffer[emLod]);

			pIndexBuffer->IncreaseReference();
			m_pIndexBuffer[emLod] = pIndexBuffer;
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

		void ModelNode::AddModelSubset(ModelSubset& modelPiece, LOD emLod)
		{
			assert(emLod <= m_emMaxLod);
			m_vecModelSubsets[emLod].push_back(modelPiece);
		}

		void ModelNode::AddModelSubsets(const std::vector<ModelSubset>& vecModelPiece, LOD emLod)
		{
			assert(emLod <= m_emMaxLod);
			std::copy(vecModelPiece.begin(), vecModelPiece.end(), std::back_inserter(m_vecModelSubsets[emLod]));
		}

		IMaterial* ModelNode::GetMaterial(const string::StringID& strMaterialName, uint32_t& nMaterialID_out) const
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
			m_originAABB.Extents = math::float3::Max(m_originAABB.Extents, math::float3(0.01f));
		}
	}
}