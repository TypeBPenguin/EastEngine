#include "stdafx.h"
#include "TerrainCells.h"

#include "ComponentPhysics.h"

#include "DirectX/DebugUtil.h"
#include "DirectX/CameraManager.h"

namespace EastEngine
{
	namespace GameObject
	{
		TerrainCells::TerrainCells()
			: m_pVertexBuffer(nullptr)
			, m_pIndexBuffer(nullptr)
			, m_pLineVertexBuffer(nullptr)
			, m_pLineIndexBuffer(nullptr)
			, m_isCulling(false)
		{
		}

		TerrainCells::~TerrainCells()
		{
			Release();
		}

		bool TerrainCells::Init(const std::vector<Graphics::VertexPosTexNorCol>& vecModel, int nNodeIdxX, int nNodeIdxY, int nCellWidth, int nCellHeight, int nTerrainWidth)
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

		void TerrainCells::Update(float fElapsedTime, std::vector<Graphics::IMaterial*>& vecMaterial, bool isEnableShowCellLine)
		{
			Graphics::RenderSubsetHeightField subset(this, m_pVertexBuffer, m_pIndexBuffer, vecMaterial[0], m_matWorld, 0, m_pIndexBuffer->GetIndexNum(), 0.f, m_boundingSphere);
			Graphics::RendererManager::GetInstance()->AddRender(subset);

			if (isEnableShowCellLine == false)
				return;

			m_matWorld = Math::Matrix::Compose((m_f3MaxSize - m_f3MinSize), Math::Quaternion::Identity, m_f3Pos);

			Graphics::RenderSubsetLine renderSubsetLine(m_pLineVertexBuffer, m_pLineIndexBuffer, m_matWorld);
			Graphics::RendererManager::GetInstance()->AddRender(renderSubsetLine);
		}

		bool TerrainCells::buildCell(const std::vector<Graphics::VertexPosTexNorCol>& vecModel, int nNodeIdxX, int nNodeIdxY, int nCellWidth, int nCellHeight, int nTerrainWidth)
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

			calcCellDimensions(vecVertices);

			return true;
		}

		bool TerrainCells::buildLine()
		{
			m_pLineVertexBuffer = Graphics::Debug::GetLineBoxVertexBuffer();
			m_pLineIndexBuffer = Graphics::Debug::GetLineBoxIndexBuffer();

			return true;
		}

		void TerrainCells::calcCellDimensions(const std::vector<Graphics::VertexPosTexNorCol>& vecVertexPos)
		{
			m_f3MaxSize.x = std::numeric_limits<float>::min();
			m_f3MaxSize.y = std::numeric_limits<float>::min();
			m_f3MaxSize.z = std::numeric_limits<float>::min();

			m_f3MinSize.x = std::numeric_limits<float>::max();
			m_f3MinSize.y = std::numeric_limits<float>::max();
			m_f3MinSize.z = std::numeric_limits<float>::max();

			const uint32_t nVertexCount = m_pVertexBuffer->GetVertexNum();
			for (uint32_t i = 0; i < nVertexCount; ++i)
			{
				const Math::Vector3& f3Size = vecVertexPos[i].pos;

				// Check if the width exceeds the minimum or maximum.
				m_f3MaxSize = Math::Vector3::Max(m_f3MaxSize, f3Size);
				m_f3MinSize = Math::Vector3::Min(m_f3MinSize, f3Size);
			}

			// Calculate the center position of this cell.
			m_f3Pos.x = ((m_f3MaxSize.x - m_f3MinSize.x) * 0.5f) + m_f3MinSize.x;
			m_f3Pos.y = ((m_f3MaxSize.y - m_f3MinSize.y) * 0.5f) + m_f3MinSize.y;
			m_f3Pos.z = ((m_f3MaxSize.z - m_f3MinSize.z) * 0.5f) + m_f3MinSize.z;

			Math::Vector3 vExtents;

			vExtents.x = (m_f3MaxSize.x - m_f3MinSize.x) * 0.5f;
			vExtents.y = (m_f3MaxSize.y - m_f3MinSize.y) * 0.5f;
			vExtents.z = (m_f3MaxSize.z - m_f3MinSize.z) * 0.5f;

			m_boundingBox = Collision::AABB(m_f3Pos, vExtents);
			m_boundingSphere.Transform(m_boundingSphere, m_matWorld);
		}
	}
}