#pragma once

#include "CommonLib/Singleton.h"

#include "GameObject.h"

namespace est
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
			void Release();
			void Update(float elapsedTime);

		public:
			ActorPtr CreateActor(const string::StringID& actorName);
			void RemoveActor(IActor* pActor);

			IActor* GetActor(const IGameObject::Handle& handle);
			IActor* GetActor(size_t index);
			size_t GetActorCount() const;

			void ExecuteFunction(std::function<void(IActor*)> func);

		public:
			TerrainPtr CreateTerrain(const string::StringID& terrainName, const TerrainProperty& terrainProperty);
			TerrainPtr CreateTerrainAsync(const string::StringID& terrainName, const TerrainProperty& terrainProperty);
			void RemoveTerrain(ITerrain* pTerrain);

			ITerrain* GetTerrain(const IGameObject::Handle& handle);
			ITerrain* GetTerrain(size_t index);
			size_t GetTerrainCount() const;

			void ExecuteFunction(std::function<void(ITerrain*)> func);

		public:
			SkyboxPtr CreateSkybox(const string::StringID& skyboxName, const SkyboxProperty& skyProperty);
			void RemoveSkybox(ISkybox* pSkybox);

			ISkybox* GetSkybox(const IGameObject::Handle& handle);
			ISkybox* GetSkybox(size_t index);
			size_t GetSkyboxCount() const;

			void ExecuteFunction(std::function<void(ISkybox*)> func);

		private:
			class Impl;
			std::unique_ptr<Impl> m_pImpl;
		};
	}
}