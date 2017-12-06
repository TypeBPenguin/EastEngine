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

			bool Init(const std::vector<Graphics::VertexPosTexNorCol>& vecModel, int nNodeIdxX, int nNodeIdxY, int nCellWidth, int nCellHeight, int nTerrainWidth);
			void Release();

			void Update(float fElapsedTime, std::vector<Graphics::IMaterial*>& vecMaterials, bool isEnableShowCellLine);

		public:
			const Math::UInt2& GetIndex() const { return m_n2NodeIdx; }
			const Collision::AABB& GetBoundingBox() const { return m_boundingBox; }
			Collision::EmContainment::Type IsContainsByBoundingBox(const Math::Vector3& f3Pos) const { return m_boundingBox.Contains(f3Pos); }
			bool IsInsideCell(const Math::Vector3& f3Pos) const { return (f3Pos.x < m_f3MaxSize.x) && (f3Pos.x > m_f3MinSize.x) && (f3Pos.z < m_f3MaxSize.z) && (f3Pos.z > m_f3MinSize.z); }
			bool IsInsideDimensions(float fPosX, float fPosZ) const
			{
				if ((fPosX < m_f3MaxSize.x) && (fPosX > m_f3MinSize.x) && (fPosZ < m_f3MaxSize.z) && (fPosZ > m_f3MinSize.z))
					return true;

				return false;
			}

			void GetCellDimensions(Math::Vector3& f3Min, Math::Vector3& f3Max) const { f3Min = m_f3MinSize; f3Max = m_f3MaxSize; }

		private:
			bool buildCell(const std::vector<Graphics::VertexPosTexNorCol>& vecModel, int nNodeIdxX, int nNodeIdxY, int nCellWidth, int nCellHeight, int nTerrainWidth);
			bool buildLine();
			void calcCellDimensions(const std::vector<Graphics::VertexPosTexNorCol>& vecVertexPos);

		private:
			Math::UInt2 m_n2NodeIdx;

			Graphics::IVertexBuffer* m_pVertexBuffer;
			Graphics::IIndexBuffer* m_pIndexBuffer;

			Graphics::IVertexBuffer* m_pLineVertexBuffer;
			Graphics::IIndexBuffer* m_pLineIndexBuffer;

			Collision::AABB m_boundingBox;
			Collision::Sphere m_boundingSphere;

			Math::Vector3 m_f3MaxSize;
			Math::Vector3 m_f3MinSize;

			Math::Vector3 m_f3Pos;
			Math::Matrix m_matWorld;

			bool m_isCulling;
		};
	}
}