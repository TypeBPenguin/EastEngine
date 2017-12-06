#include "stdafx.h"
#include "TerrainManager.h"

namespace EastEngine
{
	namespace GameObject
	{
		TerrainManager::TerrainManager()
		{
			m_colonyTerrain.reserve(16);
		}

		TerrainManager::~TerrainManager()
		{
			Release();
		}

		void TerrainManager::Release()
		{
			m_colonyTerrain.clear();
		}

		void TerrainManager::Update(float fElapsedTime)
		{
			auto iter = m_colonyTerrain.begin();
			auto iter_end = m_colonyTerrain.end();
			while (iter != iter_end)
			{
				Terrain& terrain = *iter;

				if (terrain.IsDestroy() == true)
				{
					iter = m_colonyTerrain.erase(iter);
					continue;
				}

				terrain.Update(fElapsedTime);

				++iter;
			}
		}

		ITerrain* TerrainManager::CreateTerrain(const String::StringID& strTerrainName, const TerrainProperty* pTerrainProperty)
		{
			auto iter = m_colonyTerrain.emplace();
			iter->Init(pTerrainProperty, false);
			iter->SetName(strTerrainName);

			return &(*iter);
		}

		ITerrain* TerrainManager::CreateTerrainAsync(const String::StringID& strTerrainName, const TerrainProperty* pTerrainProperty)
		{
			auto iter = m_colonyTerrain.emplace();
			iter->Init(pTerrainProperty, true);
			iter->SetName(strTerrainName);

			return &(*iter);
		}

		ITerrain* TerrainManager::GetTerrain(uint32_t nIndex)
		{
			auto iter = m_colonyTerrain.begin();
			m_colonyTerrain.advance(iter, nIndex);

			return &(*iter);
		}
	}
}