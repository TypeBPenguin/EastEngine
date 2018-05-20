#pragma once

#include "CommonLib/Singleton.h"

#include "GameObject.h"

namespace eastengine
{
	namespace gameobject
	{
		class TerrainManager : public Singleton<TerrainManager>
		{
			friend Singleton<TerrainManager>;
		private:
			TerrainManager();
			virtual ~TerrainManager();

		public:
			void Update(float fElapsedTime);

		public:
			ITerrain* CreateTerrain(const String::StringID& strTerrainName, const TerrainProperty& terrainProperty);
			ITerrain* CreateTerrainAsync(const String::StringID& strTerrainName, const TerrainProperty& terrainProperty);

			ITerrain* GetTerrain(size_t nIndex);
			size_t GetTerrainCount() const;

		private:
			class Impl;
			std::unique_ptr<Impl> m_pImpl;
		};
	}
}