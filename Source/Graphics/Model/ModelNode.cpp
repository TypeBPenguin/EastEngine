#include "stdafx.h"
#include "ModelNode.h"

#include "GeometryModel.h"

#include "CommonLib/FileStream.h"

namespace est
{
	namespace graphics
	{
		ModelNode::ModelNode(LOD emMaxLod)
			: m_emMaxLod(emMaxLod)
		{
		}

		ModelNode::ModelNode(Type emType, const wchar_t* filePath, BinaryReader& binaryReader)
		{
			m_nodeName = binaryReader.ReadString();
			m_parentNodeName = binaryReader.ReadString();
			m_attachedBoneName = binaryReader.ReadString();

			const collision::AABB& aabb = binaryReader;
			SetOriginAABB(aabb);

			m_isVisible = binaryReader;

			const uint32_t subsetCount = binaryReader;
			m_modelSubsets->resize(subsetCount);

			for (uint32_t i = 0; i < subsetCount; ++i)
			{
				m_modelSubsets[eLv0][i].name = binaryReader.ReadString();

				m_modelSubsets[eLv0][i].startIndex = binaryReader;
				m_modelSubsets[eLv0][i].indexCount = binaryReader;
				m_modelSubsets[eLv0][i].materialID = binaryReader;
				m_modelSubsets[eLv0][i].pimitiveType = binaryReader;
			}

			const uint32_t vertexCount = binaryReader;
			m_rawVertices.resize(vertexCount);

			const uint8_t* pVertices = nullptr;
			size_t vertexFormatSize = 0;
			switch (emType)
			{
			case eStatic:
				pVertices = reinterpret_cast<const uint8_t*>(&binaryReader.Read<VertexPosTexNor>(vertexCount));
				vertexFormatSize = sizeof(VertexPosTexNor);
				break;
			case eSkinned:
				pVertices = reinterpret_cast<const uint8_t*>(&binaryReader.Read<VertexPosTexNorWeiIdx>(vertexCount));
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

			VertexBufferPtr pVertexBuffer = CreateVertexBuffer(pVertices, vertexCount, vertexFormatSize, true);
			if (pVertexBuffer == nullptr)
			{
				LOG_ERROR(L"¹öÅØ½º ¹öÆÛ »ý¼º ½ÇÆÐÇß½¿µÂ");
			}
			SetVertexBuffer(pVertexBuffer);

			const uint32_t indexCount = binaryReader;
			const uint32_t* pIndices = &binaryReader.Read<uint32_t>(indexCount);

			IndexBufferPtr pIndexBuffer = CreateIndexBuffer(reinterpret_cast<const uint8_t*>(pIndices), indexCount, sizeof(uint32_t), true);
			if (pIndexBuffer == nullptr)
			{
				LOG_ERROR(L"ÀÎµ¦½º ¹öÆÛ »ý¼º ½ÇÆÐÇß½¿µÂ");
			}
			SetIndexBuffer(pIndexBuffer);

			SetRawIndices(pIndices, indexCount);

			const uint32_t materialCount = binaryReader;
			for (uint32_t i = 0; i < materialCount; ++i)
			{
				std::wstring fileName = string::MultiToWide(binaryReader.ReadString());
				fileName += L".emtl";

				MaterialPtr pMaterial = CreateMaterial(fileName.c_str(), filePath);
				AddMaterial(pMaterial);
			}
		}

		ModelNode::~ModelNode()
		{
			for (auto& pVertexBuffer : m_pVertexBuffer)
			{
				ReleaseResource(pVertexBuffer);
			}
			m_pVertexBuffer.fill(nullptr);

			for (auto& pIndexBuffer : m_pIndexBuffer)
			{
				ReleaseResource(pIndexBuffer);
			}
			m_pIndexBuffer.fill(nullptr);

			for (auto& pMaterial : m_materials)
			{
				ReleaseResource(pMaterial);
			}
			m_materials.clear();

			m_childNodes.clear();
		}

		VertexBufferPtr ModelNode::GetVertexBuffer(LOD emLod) const
		{
			if (emLod > m_emMaxLod)
				return nullptr;

			return m_pVertexBuffer[emLod];
		}

		void ModelNode::SetVertexBuffer(const VertexBufferPtr& pVertexBuffer, LOD emLod)
		{
			if (emLod > m_emMaxLod)
				return;

			ReleaseResource(m_pVertexBuffer[emLod]);
			m_pVertexBuffer[emLod] = pVertexBuffer;
		}

		IndexBufferPtr ModelNode::GetIndexBuffer(LOD emLod) const
		{
			if (emLod > m_emMaxLod)
				return nullptr;

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

		void ModelNode::SetIndexBuffer(const IndexBufferPtr& pIndexBuffer, LOD emLod)
		{
			if (emLod > m_emMaxLod)
				return;

			ReleaseResource(m_pIndexBuffer[emLod]);
			m_pIndexBuffer[emLod] = pIndexBuffer;
		}

		void ModelNode::AddMaterial(const MaterialPtr& pMaterial)
		{
			if (pMaterial != nullptr)
			{
				m_materials.push_back(pMaterial);
			}
		}

		void ModelNode::AddMaterialArray(const MaterialPtr* ppMaterials, size_t count)
		{
			m_materials.reserve(m_materials.size() + count);
			for (size_t i = 0; i < count; ++i)
			{
				AddMaterial(ppMaterials[i]);
			}
		}

		void ModelNode::AddModelSubset(ModelSubset& modelPiece, LOD emLod)
		{
			assert(emLod <= m_emMaxLod);
			m_modelSubsets[emLod].push_back(modelPiece);
		}

		void ModelNode::AddModelSubsets(const std::vector<ModelSubset>& vecModelPiece, LOD emLod)
		{
			assert(emLod <= m_emMaxLod);
			std::copy(vecModelPiece.begin(), vecModelPiece.end(), std::back_inserter(m_modelSubsets[emLod]));
		}

		MaterialPtr ModelNode::GetMaterial(const string::StringID& strMaterialName, uint32_t& nMaterialID_out) const
		{
			const size_t nSize = m_materials.size();
			for (size_t i = 0; i < nSize; ++i)
			{
				if (m_materials[i]->GetName() == strMaterialName)
				{
					nMaterialID_out = static_cast<uint32_t>(i);
					return m_materials[i];
				}
			}

			nMaterialID_out = ModelSubset::eInvalidMaterialID;
			return nullptr;
		}

		void ModelNode::SetOriginAABB(const collision::AABB& aabb)
		{
			m_originAABB = aabb;
			m_originAABB.Extents = math::float3::Max(m_originAABB.Extents, math::float3(0.01f));
		}
	}
}