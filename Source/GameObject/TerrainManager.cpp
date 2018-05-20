#include "stdafx.h"
#include "TerrainManager.h"

#include "CommonLib/plf_colony.h"

#include "Terrain.h"

namespace eastengine
{
	namespace gameobject
	{
		class TerrainManager::Impl
		{
		public:
			Impl();
			~Impl();

		public:
			void Update(float fElapsedTime);

		public:
			ITerrain * CreateTerrain(const String::StringID& strTerrainName, const TerrainProperty& terrainProperty);
			ITerrain* CreateTerrainAsync(const String::StringID& strTerrainName, const TerrainProperty& terrainProperty);

			ITerrain* GetTerrain(size_t nIndex);
			size_t GetTerrainCount() const;

		private:
			plf::colony<Terrain> m_colonyTerrain;
		};

		TerrainManager::Impl::Impl()
		{
			m_colonyTerrain.reserve(16);
		}

		TerrainManager::Impl::~Impl()
		{
		}

		void TerrainManager::Impl::Update(float fElapsedTime)
		{
			TRACER_EVENT("TerrainManager::Update");
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

		ITerrain* TerrainManager::Impl::CreateTerrain(const String::StringID& strTerrainName, const TerrainProperty& terrainProperty)
		{
			auto iter = m_colonyTerrain.emplace();
			iter->Init(terrainProperty, false);
			iter->SetName(strTerrainName);

			return &(*iter);
		}

		ITerrain* TerrainManager::Impl::CreateTerrainAsync(const String::StringID& strTerrainName, const TerrainProperty& terrainProperty)
		{
			auto iter = m_colonyTerrain.emplace();
			iter->Init(terrainProperty, true);
			iter->SetName(strTerrainName);

			return &(*iter);
		}

		ITerrain* TerrainManager::Impl::GetTerrain(size_t nIndex)
		{
			auto iter = m_colonyTerrain.begin();
			m_colonyTerrain.advance(iter, nIndex);

			return &(*iter);
		}

		size_t TerrainManager::Impl::GetTerrainCount() const
		{
			return m_colonyTerrain.size();
		}

		TerrainManager::TerrainManager()
			: m_pImpl{ std::make_unique<Impl>() }
		{
		}

		TerrainManager::~TerrainManager()
		{
		}

		void TerrainManager::Update(float fElapsedTime)
		{
			m_pImpl->Update(fElapsedTime);
		}

		ITerrain* TerrainManager::CreateTerrain(const String::StringID& strTerrainName, const TerrainProperty& terrainProperty)
		{
			return m_pImpl->CreateTerrain(strTerrainName, terrainProperty);
		}

		ITerrain* TerrainManager::CreateTerrainAsync(const String::StringID& strTerrainName, const TerrainProperty& terrainProperty)
		{
			return m_pImpl->CreateTerrainAsync(strTerrainName, terrainProperty);
		}

		ITerrain* TerrainManager::GetTerrain(size_t nIndex)
		{
			return m_pImpl->GetTerrain(nIndex);
		}

		size_t TerrainManager::GetTerrainCount() const
		{
			return m_pImpl->GetTerrainCount();
		}
	}
}