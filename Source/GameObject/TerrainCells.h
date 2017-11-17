#pragma once

namespace EastEngine
{
	namespace Graphics
	{
		class IVertexBuffer;
		class IIndexBuffer;
	}

	namespace GameObject
	{
		class TerrainCells
		{
		public:
			TerrainCells();
			~TerrainCells();

			bool Init(std::vector<Graphics::VertexPosTexNorCol>& vecModel, int nNodeIdxX, int nNodeIdxY, int nCellWidth, int nCellHeight, int nTerrainWidth);
			void Release();

			void Update(float fElapsedTime, std::vector<Graphics::IMaterial*>& vecMaterials, bool bShowCellLine);

		public:
			const Math::UInt2& GetIndex() { return m_n2NodeIdx; }
			const Collision::AABB& GetBoundingBox() { return m_boundingBox; }
			Collision::EmContainment::Type IsContainsByBoundingBox(const Math::Vector3& vPos) { return m_boundingBox.Contains(vPos); }
			bool IsInsideCell(const Math::Vector3& vPos) { return (vPos.x < m_vMaxSize.x) && (vPos.x > m_vMinSize.x) && (vPos.z < m_vMaxSize.z) && (vPos.z > m_vMinSize.z); }

			void GetCellDimensions(Math::Vector3& vMin, Math::Vector3& vMax) { vMin = m_vMinSize; vMax = m_vMaxSize; }

		private:
			bool buildCell(std::vector<Graphics::VertexPosTexNorCol>& vecModel, int nNodeIdxX, int nNodeIdxY, int nCellWidth, int nCellHeight, int nTerrainWidth);
			bool buildLine();
			void calcCellDimensions(std::vector<Graphics::VertexPosTexNorCol>& vecVertexPos);

		private:
			Math::UInt2 m_n2NodeIdx;

			Graphics::IVertexBuffer* m_pVertexBuffer;
			Graphics::IIndexBuffer* m_pIndexBuffer;

			Graphics::IVertexBuffer* m_pLineVertexBuffer;
			Graphics::IIndexBuffer* m_pLineIndexBuffer;

			Graphics::ModelSubset m_modelPiece;

			Collision::AABB m_boundingBox;

			Math::Vector3 m_vMaxSize;
			Math::Vector3 m_vMinSize;

			Math::Vector3 m_vPos;
			Math::Matrix m_matWorld;

			bool m_bCulling;
		};
	}
}