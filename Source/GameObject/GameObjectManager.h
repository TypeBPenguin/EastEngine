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
			void Update(float elapsedTime);

		public:
			IActor* CreateActor(const string::StringID& strActorName);

			IActor* GetActor(const IGameObject::Handle& handle);
			IActor* GetActor(size_t nIndex);
			size_t GetActorCount() const;

			void ExecuteFunction(std::function<void(IActor*)> func);

		public:
			ISkybox* CreateSkybox(const string::StringID& strName, const SkyboxProperty& property);

			ISkybox* GetSkybox(const IGameObject::Handle& handle);
			ISkybox* GetSkybox(size_t nIndex);
			size_t GetSkyboxCount() const;

			void ExecuteFunction(std::function<void(ISkybox*)> func);

		public:
			ITerrain * CreateTerrain(const string::StringID& strTerrainName, const TerrainProperty& terrainProperty);
			ITerrain* CreateTerrainAsync(const string::StringID& strTerrainName, const TerrainProperty& terrainProperty);

			ITerrain* GetTerrain(const IGameObject::Handle& handle);
			ITerrain* GetTerrain(size_t nIndex);
			size_t GetTerrainCount() const;

			void ExecuteFunction(std::function<void(ITerrain*)> func);

		private:
			class Impl;
			std::unique_ptr<Impl> m_pImpl;
		};
	}
}