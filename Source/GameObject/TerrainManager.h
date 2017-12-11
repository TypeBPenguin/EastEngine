#pragma once

#include "CommonLib/Singleton.h"
#include "CommonLib/plf_colony.h"

#include "Terrain.h"

namespace EastEngine
{
	namespace GameObject
	{
		class TerrainManager : public Singleton<TerrainManager>
		{
			friend Singleton<TerrainManager>;
		private:
			TerrainManager();
			virtual ~TerrainManager();

		public:
			void Release();

			void Update(float fElapsedTime);

		public:
			ITerrain* CreateTerrain(const String::StringID& strTerrainName, const TerrainProperty& terrainProperty);
			ITerrain* CreateTerrainAsync(const String::StringID& strTerrainName, const TerrainProperty& terrainProperty);

			ITerrain* GetTerrain(uint32_t nIndex);
			uint32_t GetTerrainCount() const { return m_colonyTerrain.size(); }

		private:
			plf::colony<Terrain> m_colonyTerrain;
		};
	}
}