#include "stdafx.h"
#include "SectorMgr.h"

namespace EastEngine
{
	namespace GameObject
	{
		SectorMgr::SectorMgr()
			: m_isInit(false)
		{
		}

		SectorMgr::~SectorMgr()
		{
			Release();
		}

		bool SectorMgr::Init(SectorInitInfo& sectorInitInfo)
		{
			if (m_isInit == true)
				return true;

			for (int i = 0; i < 6; ++i)
			{
				if (sectorInitInfo.nSectorsCount[i] < 0)
					return false;
			}

			Math::Int2 startPoint(-sectorInitInfo.nLeftDown, -sectorInitInfo.nDown);
			Math::Int2 endPoint(sectorInitInfo.nRightUp, sectorInitInfo.nRightUp);

			float fSqrt3 = sqrtf(3.f);

			Math::Vector3 f3SectorAreas[7] =
			{
				{ 0.f, 0.f, (float)(sectorInitInfo.nSectorsCount[0]) * 2.f },
				{ fSqrt3 * 0.5f * (float)(sectorInitInfo.nSectorsCount[1]) * 2.f, 0.f, (float)(sectorInitInfo.nSectorsCount[1]) },
				{ fSqrt3 * 0.5f * (float)(sectorInitInfo.nSectorsCount[2]) * 2.f, 0.f, -(float)(sectorInitInfo.nSectorsCount[2]) },
				{ 0.f, 0.f, -(float)(sectorInitInfo.nSectorsCount[3]) * 2.f },
				{ fSqrt3 * 0.5f * -(float)(sectorInitInfo.nSectorsCount[4]) * 2.f, 0.f, -(float)(sectorInitInfo.nSectorsCount[4]) },
				{ fSqrt3 * 0.5f * -(float)(sectorInitInfo.nSectorsCount[5]) * 2.f, 0.f, (float)(sectorInitInfo.nSectorsCount[5]) },
				{ 0.f, 0.f, (float)(sectorInitInfo.nSectorsCount[0]) * 2.f },
			};

			for (auto& area : f3SectorAreas)
			{
				area *= sectorInitInfo.fRadius;
			}

			Math::Int2 n2Around[6] =
			{
				{ 0, 1 },
				{ 1, 1 },
				{ 1, 0 },
				{ 0, -1 },
				{ -1, -1 },
				{ -1, 0 },
			};

			EmSector::Dir emDir[6] =
			{
				EmSector::eUp,
				EmSector::eRightUp,
				EmSector::eRightDown,
				EmSector::eDown,
				EmSector::eLeftDown,
				EmSector::eLeftUp,
			};

			EmSector::Dir emDirReverse[6] =
			{
				EmSector::eDown,
				EmSector::eLeftDown,
				EmSector::eLeftUp,
				EmSector::eUp,
				EmSector::eRightDown,
				EmSector::eRightUp,
			};

			// 매우 느린 작업이 될 수도 있기 때문에
			// 많이 느릴 경우, 최적화 할 수 있는 방안을 마련해야함
			for (int y = startPoint.y; y <= endPoint.y; ++y)
			{
				for (int x = startPoint.x; x <= endPoint.x; ++x)
				{
					float fPosX = (float)(x)* sectorInitInfo.fRadius * 1.5f;
					float fPosY = ((float)(y)+(float)(x)* -0.5f) * sectorInitInfo.fRadius * 2.f;

					Collision::Ray ray(Math::Vector3(fPosX, -1.f, fPosY), Math::Vector3(0.f, 1.f, 0.f));

					for (int i = 0; i < 6; ++i)
					{
						float fDist = 0.f;
						if (ray.Intersects(Math::Vector3::Zero,
							f3SectorAreas[i],
							f3SectorAreas[i + 1],
							fDist))
						{
							SectorKey key(x, y);

							auto iter = m_umapSector.find(key);
							if (iter != m_umapSector.end())
								break;

							Sector* pNewSector = new Sector(this, sectorInitInfo.fRadius, x, y);

							for (int j = 0; j < 6; ++j)
							{
								Sector* pAroundSector = static_cast<Sector*>(GetSector(x + n2Around[j].x, y + n2Around[j].y));
								if (pAroundSector == nullptr)
									continue;

								pNewSector->SetNearSector(emDir[j], pAroundSector);

								pAroundSector->SetNearSector(emDirReverse[j], pNewSector);
							}

							m_umapSector.emplace(key, pNewSector);
							break;
						}
					}
				}
			}

			m_isInit = true;

			return true;
		}

		void SectorMgr::Release()
		{
			if (m_isInit == false)
				return;

			std::for_each(m_umapSector.begin(), m_umapSector.end(), DeleteSTLMapObject());
			m_umapSector.clear();

			m_listEnterLeaveSectorActor.clear();

			m_isInit = false;
		}

		void SectorMgr::Update(float fElapsedTime)
		{
			{
				auto iter = m_listEnterLeaveSectorActor.begin();
				while (iter != m_listEnterLeaveSectorActor.end())
				{
					const EnterLeaveSectorInfo& moveSectorInfo = *iter;

					if (moveSectorInfo.pActor == nullptr)
						continue;

					if (moveSectorInfo.pLeaveSector != nullptr)
						moveSectorInfo.pLeaveSector->Leave(moveSectorInfo.pActor);

					if (moveSectorInfo.pEnterSector != nullptr)
						moveSectorInfo.pEnterSector->Enter(moveSectorInfo.pActor);
				}

				m_listEnterLeaveSectorActor.clear();
			}

			for (auto& iter : m_umapSector)
			{
				iter.second->Update(fElapsedTime);
			}
		}

		Sector* SectorMgr::GetSector(int nCoordinateX, int nCoordinateY)
		{
			SectorKey key(nCoordinateX, nCoordinateY);
			auto iter = m_umapSector.find(key);
			if (iter == m_umapSector.end())
				return nullptr;

			return iter->second;
		}
	}
}