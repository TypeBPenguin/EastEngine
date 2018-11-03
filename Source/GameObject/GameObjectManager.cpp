#include "stdafx.h"
#include "GameObjectManager.h"

#include "CommonLib/plf_colony.h"
#include "CommonLib/Lock.h"

#include "Actor.h"
#include "Skybox.h"
#include "Terrain.h"

namespace eastengine
{
	namespace gameobject
	{
		class GameObjectManager::Impl
		{
		public:
			Impl();
			~Impl();

		public:
			void Update(float fElapsedTime);

		public:
			IActor* CreateActor(const string::StringID& strActorName);

			IActor* GetActor(const IGameObject::Handle& handle);
			IActor* GetActor(size_t nIndex);
			size_t GetActorCount() const { return m_colonyActor.size(); }

			void ExecuteFunction(std::function<void(IActor*)> func)
			{
				thread::SRWWriteLock writeLock(&m_srwLockObjects[eActor]);
				std::for_each(m_colonyActor.begin(), m_colonyActor.end(), [&](Actor& actor)
				{
					func(&actor);
				});
			}

		public:
			ISkybox* CreateSkybox(const string::StringID& strName, const SkyboxProperty& property);

			ISkybox* GetSkybox(const IGameObject::Handle& handle);
			ISkybox* GetSkybox(size_t nIndex);
			size_t GetSkyboxCount() const { return m_colonySkybox.size(); }

			void ExecuteFunction(std::function<void(ISkybox*)> func)
			{
				thread::SRWWriteLock writeLock(&m_srwLockObjects[eSky]);
				std::for_each(m_colonySkybox.begin(), m_colonySkybox.end(), [&](Skybox& skybox)
				{
					func(&skybox);
				});
			}

		public:
			ITerrain* CreateTerrain(const string::StringID& strTerrainName, const TerrainProperty& terrainProperty);
			ITerrain* CreateTerrainAsync(const string::StringID& strTerrainName, const TerrainProperty& terrainProperty);

			ITerrain* GetTerrain(const IGameObject::Handle& handle);
			ITerrain* GetTerrain(size_t nIndex);
			size_t GetTerrainCount() const { return m_colonyTerrain.size(); }

			void ExecuteFunction(std::function<void(ITerrain*)> func)
			{
				thread::SRWWriteLock writeLock(&m_srwLockObjects[eTerrain]);
				std::for_each(m_colonyTerrain.begin(), m_colonyTerrain.end(), [&](Terrain& terrain)
				{
					func(&terrain);
				});
			}

		private:
			size_t m_nAllocateIndex{ 0 };

			plf::colony<Actor> m_colonyActor;
			std::unordered_map<IGameObject::Handle, Actor*> m_umapActors;

			plf::colony<Skybox> m_colonySkybox;
			std::unordered_map<IGameObject::Handle, Skybox*> m_umapSkybox;

			plf::colony<Terrain> m_colonyTerrain;
			std::unordered_map<IGameObject::Handle, Terrain*> m_umapTerrain;

			std::array<thread::SRWLock, ObjectType::eTypeCount> m_srwLockObjects;
		};

		GameObjectManager::Impl::Impl()
		{
			m_colonyActor.reserve(128);
			m_colonySkybox.reserve(16);
			m_colonyTerrain.reserve(16);
		}

		GameObjectManager::Impl::~Impl()
		{
		}

		void GameObjectManager::Impl::Update(float fElapsedTime)
		{
			// Sky
			{
				TRACER_EVENT("SkyboxUpdate");

				thread::SRWWriteLock writeLock(&m_srwLockObjects[eSky]);

				auto iter = m_colonySkybox.begin();
				auto iter_end = m_colonySkybox.end();
				while (iter != iter_end)
				{
					Skybox& skybox = *iter;

					if (skybox.IsDestroy() == true)
					{
						iter = m_colonySkybox.erase(iter);
						continue;
					}
					else
					{
						skybox.Update(fElapsedTime);
						++iter;
					}
				}
			}

			// Terrain
			{
				TRACER_EVENT("TerrainManager::Update");

				thread::SRWWriteLock writeLock(&m_srwLockObjects[eTerrain]);

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
					else
					{
						terrain.Update(fElapsedTime);
						++iter;
					}
				}
			}

			// Actor
			{
				TRACER_EVENT("ActorUpdate");

				thread::SRWWriteLock writeLock(&m_srwLockObjects[eActor]);

				auto iter = m_colonyActor.begin();
				auto iter_end = m_colonyActor.end();
				while (iter != iter_end)
				{
					Actor& actor = *iter;

					if (actor.IsDestroy() == true)
					{
						iter = m_colonyActor.erase(iter);
						continue;
					}
					else
					{
						actor.Update(fElapsedTime);
						++iter;
					}
				}
			}
		}

		IActor* GameObjectManager::Impl::CreateActor(const string::StringID& strActorName)
		{
			thread::SRWWriteLock writeLock(&m_srwLockObjects[eActor]);

			IGameObject::Handle handle(m_nAllocateIndex++);

			auto iter = m_colonyActor.emplace(handle);
			iter->SetName(strActorName);

			m_umapActors.emplace(handle, &(*iter));

			return &(*iter);
		}

		IActor* GameObjectManager::Impl::GetActor(const IGameObject::Handle& handle)
		{
			thread::SRWReadLock readLock(&m_srwLockObjects[eActor]);

			auto iter = m_umapActors.find(handle);
			if (iter != m_umapActors.end())
				return iter->second;

			return nullptr;
		}

		IActor* GameObjectManager::Impl::GetActor(size_t nIndex)
		{
			thread::SRWReadLock readLock(&m_srwLockObjects[eActor]);

			auto iter = m_colonyActor.begin();
			m_colonyActor.advance(iter, nIndex);

			return &(*iter);
		}

		ISkybox* GameObjectManager::Impl::CreateSkybox(const string::StringID& strName, const SkyboxProperty& property)
		{
			thread::SRWWriteLock writeLock(&m_srwLockObjects[eSky]);

			IGameObject::Handle handle(m_nAllocateIndex++);

			auto iter = m_colonySkybox.emplace(handle);
			iter->Init(property);
			iter->SetName(strName);

			m_umapSkybox.emplace(handle, &(*iter));

			return &(*iter);
		}

		ISkybox* GameObjectManager::Impl::GetSkybox(const IGameObject::Handle& handle)
		{
			thread::SRWReadLock readLock(&m_srwLockObjects[eSky]);

			auto iter = m_umapSkybox.find(handle);
			if (iter != m_umapSkybox.end())
				return iter->second;

			return nullptr;
		}

		ISkybox* GameObjectManager::Impl::GetSkybox(size_t nIndex)
		{
			thread::SRWReadLock readLock(&m_srwLockObjects[eSky]);

			auto iter = m_colonySkybox.begin();
			m_colonySkybox.advance(iter, nIndex);

			return &(*iter);
		}

		ITerrain* GameObjectManager::Impl::CreateTerrain(const string::StringID& strTerrainName, const TerrainProperty& terrainProperty)
		{
			thread::SRWWriteLock writeLock(&m_srwLockObjects[eTerrain]);

			IGameObject::Handle handle(m_nAllocateIndex++);

			auto iter = m_colonyTerrain.emplace(handle);
			iter->Init(terrainProperty, false);
			iter->SetName(strTerrainName);

			m_umapTerrain.emplace(handle, &(*iter));

			return &(*iter);
		}

		ITerrain* GameObjectManager::Impl::CreateTerrainAsync(const string::StringID& strTerrainName, const TerrainProperty& terrainProperty)
		{
			thread::SRWWriteLock writeLock(&m_srwLockObjects[eTerrain]);

			IGameObject::Handle handle(m_nAllocateIndex++);

			auto iter = m_colonyTerrain.emplace(handle);
			iter->Init(terrainProperty, true);
			iter->SetName(strTerrainName);

			m_umapTerrain.emplace(handle, &(*iter));

			return &(*iter);
		}

		ITerrain* GameObjectManager::Impl::GetTerrain(const IGameObject::Handle& handle)
		{
			thread::SRWReadLock readLock(&m_srwLockObjects[eTerrain]);

			auto iter = m_umapTerrain.find(handle);
			if (iter != m_umapTerrain.end())
				return iter->second;

			return nullptr;
		}

		ITerrain* GameObjectManager::Impl::GetTerrain(size_t nIndex)
		{
			thread::SRWReadLock readLock(&m_srwLockObjects[eTerrain]);

			auto iter = m_colonyTerrain.begin();
			m_colonyTerrain.advance(iter, nIndex);

			return &(*iter);
		}

		GameObjectManager::GameObjectManager()
			: m_pImpl{ std::make_unique<Impl>() }
		{
		}

		GameObjectManager::~GameObjectManager()
		{
		}

		void GameObjectManager::Update(float fElapsedTime)
		{
			m_pImpl->Update(fElapsedTime);
		}

		IActor* GameObjectManager::CreateActor(const string::StringID& strActorName)
		{
			return m_pImpl->CreateActor(strActorName);
		}

		IActor* GameObjectManager::GetActor(const IGameObject::Handle& handle)
		{
			return m_pImpl->GetActor(handle);
		}

		IActor* GameObjectManager::GetActor(size_t nIndex)
		{
			return m_pImpl->GetActor(nIndex);
		}

		size_t GameObjectManager::GetActorCount() const
		{
			return m_pImpl->GetActorCount();
		}

		void GameObjectManager::ExecuteFunction(std::function<void(IActor*)> func)
		{
			m_pImpl->ExecuteFunction(func);
		}

		ISkybox* GameObjectManager::CreateSkybox(const string::StringID& strName, const SkyboxProperty& property)
		{
			return m_pImpl->CreateSkybox(strName, property);
		}

		ISkybox* GameObjectManager::GetSkybox(const IGameObject::Handle& handle)
		{
			return m_pImpl->GetSkybox(handle);
		}

		ISkybox* GameObjectManager::GetSkybox(size_t nIndex)
		{
			return m_pImpl->GetSkybox(nIndex);
		}

		size_t GameObjectManager::GetSkyboxCount() const
		{
			return m_pImpl->GetSkyboxCount();
		}

		void GameObjectManager::ExecuteFunction(std::function<void(ISkybox*)> func)
		{
			m_pImpl->ExecuteFunction(func);
		}

		ITerrain* GameObjectManager::CreateTerrain(const string::StringID& strTerrainName, const TerrainProperty& terrainProperty)
		{
			return m_pImpl->CreateTerrain(strTerrainName, terrainProperty);
		}

		ITerrain* GameObjectManager::CreateTerrainAsync(const string::StringID& strTerrainName, const TerrainProperty& terrainProperty)
		{
			return m_pImpl->CreateTerrainAsync(strTerrainName, terrainProperty);
		}

		ITerrain* GameObjectManager::GetTerrain(const IGameObject::Handle& handle)
		{
			return m_pImpl->GetTerrain(handle);
		}

		ITerrain* GameObjectManager::GetTerrain(size_t nIndex)
		{
			return m_pImpl->GetTerrain(nIndex);
		}

		size_t GameObjectManager::GetTerrainCount() const
		{
			return m_pImpl->GetTerrainCount();
		}

		void GameObjectManager::ExecuteFunction(std::function<void(ITerrain*)> func)
		{
			m_pImpl->ExecuteFunction(func);
		}
	}
}