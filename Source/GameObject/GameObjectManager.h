#pragma once

#include "CommonLib/Singleton.h"

#include "GameObject.h"

namespace eastengine
{
	namespace gameobject
	{
		class GameObjectManager : public Singleton<GameObjectManager>
		{
			friend Singleton<GameObjectManager>;
		private:
			GameObjectManager();
			virtual ~GameObjectManager();

		public:
			void Update(float fElapsedTime);

		public:
			IActor* CreateActor(const String::StringID& strActorName);

			IActor* GetActor(const IGameObject::Handle& handle);
			IActor* GetActor(size_t nIndex);
			size_t GetActorCount() const;

		public:
			ISkybox* CreateSkybox(const String::StringID& strName, const SkyboxProperty& property);

			ISkybox* GetSkybox(const IGameObject::Handle& handle);
			ISkybox* GetSkybox(size_t nIndex);
			size_t GetSkyboxCount() const;

		public:
			ITerrain * CreateTerrain(const String::StringID& strTerrainName, const TerrainProperty& terrainProperty);
			ITerrain* CreateTerrainAsync(const String::StringID& strTerrainName, const TerrainProperty& terrainProperty);

			ITerrain* GetTerrain(const IGameObject::Handle& handle);
			ITerrain* GetTerrain(size_t nIndex);
			size_t GetTerrainCount() const;

		private:
			class Impl;
			std::unique_ptr<Impl> m_pImpl;
		};
	}
}