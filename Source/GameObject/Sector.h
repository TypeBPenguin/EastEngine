#pragma once

#include "GameObject.h"

namespace eastengine
{
	namespace gameobject
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

		struct SectorKey
		{
			const int x = 0;
			const int y = 0;

			SectorKey(int x, int y)
				: x(x)
				, y(y)
			{
			}

			bool operator == (const SectorKey& source) const
			{
				return x == source.x && y == source.y;
			}

			bool operator != (const SectorKey& source) const
			{
				return x != source.x || y != source.y;
			}
		};

		class Sector
		{
		public:
			Sector(SectorMgr* pSectorMgr, float fRadius, int nCoordinateX, int nCoordinateY);
			~Sector();

			void Enter(IActor* pActor);
			void Leave(IActor* pActor);

			void Update(float elapsedTime);

			bool MoveToNearSector(EmSector::Dir emSectorDir, IActor* pActor);

			float GetRadius() { return m_fRadius; }

			bool IsVisibleTile() { return m_isVisibleTile; }
			void SetVisibleTile(bool bVisibleTile) { m_isVisibleTile = bVisibleTile; }

			void SetNearSector(EmSector::Dir emSectorDir, Sector* pSector) { if (pSector == nullptr) return; m_vecNearSector[emSectorDir] = pSector; }

		public:
			const math::int2& GetCoordinate() { return m_n2Coordinate; }

		private:
			SectorMgr* m_pSectorMgr{ nullptr };
			IActor* m_pActor{ nullptr };

			float m_fRadius{ 0.f };
			math::int2 m_n2Coordinate;

			std::vector<Sector*> m_vecNearSector;
			tsl::robin_map<IGameObject::Handle, IActor*> m_umapActor;

			bool m_isVisibleTile{ false };
		};
	}
}

namespace std
{
	template <>
	struct hash<eastengine::gameobject::SectorKey>
	{
		std::size_t operator()(const eastengine::gameobject::SectorKey& key) const
		{
			return std::hash<uint64_t>{}((static_cast<uint64_t>(key.x) << 32) | key.y);
		}
	};
}