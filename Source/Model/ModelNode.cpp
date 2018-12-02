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

		ModelNode::ModelNode(Type emType, const char* filePath, const BYTE** ppBuffer)
		{
			m_nodeName = file::Stream::ToString(ppBuffer);
			m_parentNodeName = file::Stream::ToString(ppBuffer);
			m_attachedBoneName = file::Stream::ToString(ppBuffer);

			const Collision::AABB* pAABB = file::Stream::To<Collision::AABB>(ppBuffer);
			SetOriginAABB(*pAABB);

			m_isVisible = *file::Stream::To<bool>(ppBuffer);

			const uint32_t subsetCount = *file::Stream::To<uint32_t>(ppBuffer);
			m_vecModelSubsets->resize(subsetCount);

			for (uint32_t i = 0; i < subsetCount; ++i)
			{
				m_vecModelSubsets[eLv0][i].strName = file::Stream::ToString(ppBuffer);

				m_vecModelSubsets[eLv0][i].nStartIndex = *file::Stream::To<uint32_t>(ppBuffer);
				m_vecModelSubsets[eLv0][i].nIndexCount = *file::Stream::To<uint32_t>(ppBuffer);
				m_vecModelSubsets[eLv0][i].nMaterialID = *file::Stream::To<uint32_t>(ppBuffer);

				const int* pPrimitiveType = file::Stream::To<int>(ppBuffer);
				m_vecModelSubsets[eLv0][i].emPrimitiveType = static_cast<EmPrimitive::Type>(*pPrimitiveType);
			}

			const uint32_t vertexCount = *file::Stream::To<uint32_t>(ppBuffer);
			m_rawVertices.resize(vertexCount);

			const uint8_t* pVertices = nullptr;
			size_t vertexFormatSize = 0;
			switch (emType)
			{
			case eStatic:
				pVertices = reinterpret_cast<const uint8_t*>(file::Stream::To<VertexPosTexNor>(ppBuffer, vertexCount));
				vertexFormatSize = sizeof(VertexPosTexNor);
				break;
			case eSkinned:
				pVertices = reinterpret_cast<const uint8_t*>(file::Stream::To<VertexPosTexNorWeiIdx>(ppBuffer, vertexCount));
				vertexFormatSize = sizeof(VertexPosTexNorWeiIdx);
				break;
			default:
				throw_line("unknown model node type");
				break;
			}

			for (uint32_t i = 0; i < vertexCount; ++i)
			{
				m_rawVertices[i].pos = *reinterpret_cast<const math::float3*>(pVertices + vertexFormatSize * i);
			}

			IVertexBuffer* pVertexBuffer = CreateVertexBuffer(pVertices, vertexCount, vertexFormatSize, true);
			if (pVertexBuffer == nullptr)
			{
				LOG_ERROR("¹öÅØ½º ¹öÆÛ »ý¼º ½ÇÆÐÇß½¿µÂ");
			}
			SetVertexBuffer(pVertexBuffer);
			ReleaseResource(&pVertexBuffer);

			const uint32_t indexCount = *file::Stream::To<uint32_t>(ppBuffer);
			const uint32_t* pIndices = file::Stream::To<uint32_t>(ppBuffer, indexCount);

			IIndexBuffer* pIndexBuffer = CreateIndexBuffer(reinterpret_cast<const uint8_t*>(pIndices), indexCount, sizeof(uint32_t), true);
			if (pIndexBuffer == nullptr)
			{
				LOG_ERROR("ÀÎµ¦½º ¹öÆÛ »ý¼º ½ÇÆÐÇß½¿µÂ");
			}
			SetIndexBuffer(pIndexBuffer);
			ReleaseResource(&pIndexBuffer);

			SetRawIndices(pIndices, indexCount);

			const uint32_t materialCount = *file::Stream::To<uint32_t>(ppBuffer);
			for (uint32_t i = 0; i < materialCount; ++i)
			{
				std::string fileName = file::Stream::ToString(ppBuffer);
				fileName += ".emtl";

				IMaterial* pMaterial = CreateMaterial(fileName.c_str(), filePath);
				AddMaterial(pMaterial);
				ReleaseResource(&pMaterial);
			}
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

			if (pVertexBuffer != nullptr)
			{
				pVertexBuffer->IncreaseReference();
			}
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

			if (pIndexBuffer != nullptr)
			{
				pIndexBuffer->IncreaseReference();
			}
			m_pIndexBuffer[emLod] = pIndexBuffer;
		}

		void ModelNode::AddMaterial(IMaterial* pMaterial)
		{
			if (pMaterial != nullptr)
			{
				pMaterial->IncreaseReference();
				m_vecMaterial.push_back(pMaterial);
			}
		}

		void ModelNode::AddMaterialArray(IMaterial** ppMaterials, size_t nCount)
		{
			m_vecMaterial.reserve(m_vecMaterial.size() + nCount);
			for (size_t i = 0; i < nCount; ++i)
			{
				AddMaterial(ppMaterials[i]);
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