#pragma once

#include "Sector.h"

namespace EastEngine
{
	namespace GameObject
	{
		struct SectorInitInfo
		{
			union
			{
				struct
				{
					int nUp;
					int nRightUp;
					int nRightDown;
					int nDown;
					int nLeftDown;
					int nLeftUp;
				};

				int nSectorsCount[6];
			};

			float fRadius;

			SectorInitInfo() : fRadius(1.f)
			{
				for (int i = 0; i < 6; ++i)
				{
					nSectorsCount[i] = 6;
				}
			}
		};

		struct EnterLeaveSectorInfo
		{
			Sector* pEnterSector = nullptr;
			Sector* pLeaveSector = nullptr;
			IActor* pActor = nullptr;
		};

		class SectorMgr
		{
		public:
			SectorMgr();
			~SectorMgr();

		public:
			bool Init(SectorInitInfo& sectorInitInfo);
			void Release();

			void Update(float fElapsedTime);

			void EnterLeaveSector(EnterLeaveSectorInfo& moveSectorInfo) { m_listEnterLeaveSectorActor.emplace_back(moveSectorInfo); }

		public:
			Sector* GetSector(int nCoordinateX, int nCoordinateY);

		private:
			std::unordered_map<SectorKey, Sector*> m_umapSector;
			std::list<EnterLeaveSectorInfo>	m_listEnterLeaveSectorActor;

			bool m_isInit;
		};
	}
}