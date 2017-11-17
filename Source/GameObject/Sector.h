#pragma once

namespace EastEngine
{
	namespace GameObject
	{
		class IActor;
		class SectorMgr;

		namespace EmSector
		{
			enum Dir
			{
				eUp = 0,
				eRightUp,
				eRightDown,
				eDown,
				eLeftDown,
				eLeftUp,

				eCount,
			};
		}

		class Sector
		{
		public:
			Sector(SectorMgr* pSectorMgr, float fRadius, int nCoordinateX, int nCoordinateY);
			~Sector();

			void Enter(IActor* pActor);
			void Leave(IActor* pActor);

			void Update(float fElapsedTime);

			bool MoveToNearSector(EmSector::Dir emSectorDir, IActor* pActor);

			float GetRadius() { return m_fRadius; }

			bool IsVisibleTile() { return m_isVisibleTile; }
			void SetVisibleTile(bool bVisibleTile) { m_isVisibleTile = bVisibleTile; }

			void SetNearSector(EmSector::Dir emSectorDir, Sector* pSector) { if (pSector == nullptr) return; m_vecNearSector[emSectorDir] = pSector; }

		public:
			const Math::Int2& GetCoordinate() { return m_n2Coordinate; }

		private:
			SectorMgr* m_pSectorMgr;
			IActor* m_pActor;

			float m_fRadius;
			Math::Int2 m_n2Coordinate;

			std::vector<Sector*> m_vecNearSector;
			boost::unordered_map<uint32_t, IActor*> m_umapActor;

			bool m_isVisibleTile;
		};
	}
}