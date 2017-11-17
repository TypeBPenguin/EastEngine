#include "stdafx.h"
#include "TerrainCells.h"

#include "ComponentPhysics.h"

#include "../DirectX/DebugUtil.h"
#include "../DirectX/CameraManager.h"

namespace EastEngine
{
	namespace GameObject
	{
		TerrainCells::TerrainCells()
			: m_pVertexBuffer(nullptr)
			, m_pIndexBuffer(nullptr)
			, m_pLineVertexBuffer(nullptr)
			, m_pLineIndexBuffer(nullptr)
			, m_bCulling(false)
		{
		}

		TerrainCells::~TerrainCells()
		{
			Release();
		}

		bool TerrainCells::Init(std::vector<Graphics::VertexPosTexNorCol>& vecModel, int nNodeIdxX, int nNodeIdxY, int nCellWidth, int nCellHeight, int nTerrainWidth)
		{
			m_n2NodeIdx = Math::UInt2(nNodeIdxX, nNodeIdxY);

			if (buildCell(vecModel, nNodeIdxX, nNodeIdxY, nCellWidth, nCellHeight, nTerrainWidth) == false)
				return false;

			if (buildLine() == false)
				return false;

			return true;
		}

		void TerrainCells::Release()
		{
			SafeDelete(m_pVertexBuffer);
			SafeDelete(m_pIndexBuffer);
			m_pLineVertexBuffer = nullptr;
			m_pLineIndexBuffer = nullptr;
		}

		void TerrainCells::Update(float fElapsedTime, std::vector<Graphics::IMaterial*>& vecMaterial, bool bShowCellLine)
		{
			m_matWorld = Math::Matrix::Identity;

			//Graphics::RenderSubsetTerrain renderSubset(this, m_pVertexBuffer, m_pIndexBuffer, vecMaterial[m_modelPiece.nMaterialID], m_matWorld, m_modelPiece.nStartIndex, m_modelPiece.nIndexCount);
			//Graphics::RendererManager::GetInstance()->AddRender(renderSubset);

			if (bShowCellLine == false)
				return;

			Math::Matrix matWorld(Math::Matrix::Compose((m_vMaxSize - m_vMinSize), Math::Quaternion::Identity, m_vPos));

			Graphics::RenderSubsetLine renderSubsetLine(m_pLineVertexBuffer, m_pLineIndexBuffer, matWorld);
			Graphics::RendererManager::GetInstance()->AddRender(renderSubsetLine);
		}

		bool TerrainCells::buildCell(std::vector<Graphics::VertexPosTexNorCol>& vecModel, int nNodeIdxX, int nNodeIdxY, int nCellWidth, int nCellHeight, int nTerrainWidth)
		{
			uint32_t nVertexCount = (nCellHeight - 1) * (nCellWidth - 1) * 6;
			uint32_t nIndexCount = nVertexCount;

			std::vector<Graphics::VertexPosTexNorCol> vecVertices;
			vecVertices.reserve(nVertexCount);

			std::vector<uint32_t> vecIndices;
			vecIndices.reserve(nIndexCount);

			int nModelIdx = ((nNodeIdxX * (nCellWidth - 1)) + (nNodeIdxY * (nCellHeight - 1) * (nTerrainWidth - 1))) * 6;
			int nIdx = 0;

			int nWidth = (nCellWidth - 1) * 6;
			int nHeight = nCellHeight - 1;
			for (int i = 0; i < nHeight; ++i)
			{
				for (int j = 0; j < nWidth; ++j)
				{
					vecVertices.push_back(vecModel[nModelIdx]);
					vecIndices.push_back(nIdx);
					nModelIdx++;
					nIdx++;
				}
				nModelIdx += (nTerrainWidth * 6) - (nCellWidth * 6);
			}

			m_pVertexBuffer = Graphics::IVertexBuffer::Create(Graphics::VertexPosTexNorCol::Format(), vecVertices.size(), &vecVertices.front(), D3D11_USAGE_IMMUTABLE);
			m_pIndexBuffer = Graphics::IIndexBuffer::Create(vecIndices.size(), &vecIndices.front(), D3D11_USAGE_IMMUTABLE);

			if (m_pVertexBuffer == nullptr || m_pIndexBuffer == nullptr)
			{
				Release();
				return false;
			}

			m_modelPiece.nMaterialID = 0;
			m_modelPiece.nStartIndex = 0;
			m_modelPiece.nIndexCount = nVertexCount;

			calcCellDimensions(vecVertices);

			return true;
		}

		bool TerrainCells::buildLine()
		{
			m_pLineVertexBuffer = Graphics::Debug::GetLineBoxVertexBuffer();
			m_pLineIndexBuffer = Graphics::Debug::GetLineBoxIndexBuffer();

			return true;
		}

		void TerrainCells::calcCellDimensions(std::vector<Graphics::VertexPosTexNorCol>& vecVertexPos)
		{
			m_vMaxSize.x = (float)(-INT32_MAX);
			m_vMaxSize.y = (float)(-INT32_MAX);
			m_vMaxSize.z = (float)(-INT32_MAX);

			m_vMinSize.x = (float)(INT32_MAX);
			m_vMinSize.y = (float)(INT32_MAX);
			m_vMinSize.z = (float)(INT32_MAX);

			uint32_t nVertexCount = m_pVertexBuffer->GetVertexNum();
			for (uint32_t i = 0; i < nVertexCount; ++i)
			{
				Math::Vector3& vSize = vecVertexPos[i].pos;

				// Check if the width exceeds the minimum or maximum.
				if (vSize.x > m_vMaxSize.x)
				{
					m_vMaxSize.x = vSize.x;
				}
				if (vSize.x < m_vMinSize.x)
				{
					m_vMinSize.x = vSize.x;
				}

				// Check if the height exceeds the minimum or maximum.
				if (vSize.y > m_vMaxSize.y)
				{
					m_vMaxSize.y = vSize.y;
				}
				if (vSize.y < m_vMinSize.y)
				{
					m_vMinSize.y = vSize.y;
				}

				// Check if the depth exceeds the minimum or maximum.
				if (vSize.z > m_vMaxSize.z)
				{
					m_vMaxSize.z = vSize.z;
				}
				if (vSize.z < m_vMinSize.z)
				{
					m_vMinSize.z = vSize.z;
				}
			}

			// Calculate the center position of this cell.
			m_vPos.x = ((m_vMaxSize.x - m_vMinSize.x) * 0.5f) + m_vMinSize.x;
			m_vPos.y = ((m_vMaxSize.y - m_vMinSize.y) * 0.5f) + m_vMinSize.y;
			m_vPos.z = ((m_vMaxSize.z - m_vMinSize.z) * 0.5f) + m_vMinSize.z;

			Math::Vector3 vExtents;

			vExtents.x = (m_vMaxSize.x - m_vMinSize.x) * 0.5f;
			vExtents.y = (m_vMaxSize.y - m_vMinSize.y) * 0.5f;
			vExtents.z = (m_vMaxSize.z - m_vMinSize.z) * 0.5f;

			m_boundingBox = Collision::AABB(m_vPos, vExtents);
		}
	}
}