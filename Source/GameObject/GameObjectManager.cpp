#include "stdafx.h"
#include "GameObjectManager.h"

#include "CommonLib/Lock.h"
#include "CommonLib/ObjectPool.h"

#include "Actor.h"
#include "Skybox.h"
#include "Terrain.h"

namespace est
{
	namespace gameobject
	{
		class GameObjectManager::Impl
		{
		public:
			Impl();
			~Impl();

		public:
			void Release();
			void Update(float elapsedTime);

		public:
			ActorPtr CreateActor(const string::StringID& actorName);
			void RemoveActor(IActor* pActor);

			IActor* GetActor(const IGameObject::Handle& handle);
			IActor* GetActor(size_t index);
			size_t GetActorCount() const { return m_actors.size(); }

			void ExecuteFunction(std::function<void(IActor*)> func)
			{
				thread::SRWWriteLock writeLock(&m_srwLockObjects[eActor]);
				std::for_each(m_actors.begin(), m_actors.end(), [&](Actor* pActor)
				{
					func(pActor);
				});
			}

		public:
			TerrainPtr CreateTerrain(const string::StringID& terrainName, const TerrainProperty& terrainProperty);
			TerrainPtr CreateTerrainAsync(const string::StringID& terrainName, const TerrainProperty& terrainProperty);
			void RemoveTerrain(ITerrain* pTerrain);

			ITerrain* GetTerrain(const IGameObject::Handle& handle);
			ITerrain* GetTerrain(size_t index);
			size_t GetTerrainCount() const { return m_terrains.size(); }

			void ExecuteFunction(std::function<void(ITerrain*)> func)
			{
				thread::SRWWriteLock writeLock(&m_srwLockObjects[eTerrain]);
				std::for_each(m_terrains.begin(), m_terrains.end(), [&](Terrain* pTerrain)
				{
					func(pTerrain);
				});
			}

		public:
			SkyboxPtr CreateSkybox(const string::StringID& name, const SkyboxProperty& skyProperty);
			void RemoveSkybox(ISkybox* pSkybox);

			ISkybox* GetSkybox(const IGameObject::Handle& handle);
			ISkybox* GetSkybox(size_t index);
			size_t GetSkyboxCount() const { return m_skyboxs.size(); }

			void ExecuteFunction(std::function<void(ISkybox*)> func)
			{
				thread::SRWWriteLock writeLock(&m_srwLockObjects[eSky]);
				std::for_each(m_skyboxs.begin(), m_skyboxs.end(), [&](Skybox* pSkybox)
					{
						func(pSkybox);
					});
			}

		private:
			void Optimize();

		private:
			size_t m_allocateIndex{ 0 };
			float m_optimizeTime{ 0.f };

			memory::ObjectPool<Actor, 128> m_poolActor;
			std::vector<Actor*> m_actors;
			tsl::robin_map<IGameObject::Handle, Actor*> m_rmapActors;

			memory::ObjectPool<Skybox, 4> m_poolSkybox;
			std::vector<Skybox*> m_skyboxs;
			tsl::robin_map<IGameObject::Handle, Skybox*> m_rmapSkyboxs;

			memory::ObjectPool<Terrain, 4> m_poolTerrain;
			std::vector<Terrain*> m_terrains;
			tsl::robin_map<IGameObject::Handle, Terrain*> m_rmapTerrains;

			std::array<bool, ObjectType::eTypeCount> m_isDirty{ false };
			std::array<thread::SRWLock, ObjectType::eTypeCount> m_srwLockObjects;
		};

		GameObjectManager::Impl::Impl()
		{
		}

		GameObjectManager::Impl::~Impl()
		{
		}

		void GameObjectManager::Impl::Release()
		{
			{
				thread::SRWWriteLock writeLock(&m_srwLockObjects[eActor]);
				for (auto pActor : m_actors)
				{
					m_poolActor.Destroy(pActor);
				}
				m_actors.clear();
				m_rmapActors.clear();
				m_poolActor.ReleaseEmptyChunk(0);
			}

			{
				thread::SRWWriteLock writeLock(&m_srwLockObjects[eSky]);
				for (auto pSkybox : m_skyboxs)
				{
					m_poolSkybox.Destroy(pSkybox);
				}
				m_skyboxs.clear();
				m_rmapSkyboxs.clear();
				m_poolSkybox.ReleaseEmptyChunk(0);
			}

			{
				thread::SRWWriteLock writeLock(&m_srwLockObjects[eTerrain]);
				for (auto pTerrain : m_terrains)
				{
					m_poolTerrain.Destroy(pTerrain);
				}
				m_terrains.clear();
				m_rmapTerrains.clear();
				m_poolTerrain.ReleaseEmptyChunk(0);
			}
		}

		void GameObjectManager::Impl::Update(float elapsedTime)
		{
			TRACER_EVENT(__FUNCTIONW__);

			// Optimize, for cache friendly
			{
				m_optimizeTime += elapsedTime;

				constexpr float OptimizeInterval = 10.f;
				if (m_optimizeTime >= OptimizeInterval)
				{
					m_optimizeTime = 0.f;

					Optimize();
				}
			}

			// Sky
			{
				TRACER_EVENT(L"Skybox");

				thread::SRWWriteLock writeLock(&m_srwLockObjects[eSky]);

				m_skyboxs.erase(std::remove_if(m_skyboxs.begin(), m_skyboxs.end(), [&](Skybox* pSkybox)
					{
						if (pSkybox->IsDestroy() == true)
						{
							m_poolSkybox.Destroy(pSkybox);
							return true;
						}
						else
						{
							pSkybox->Update(elapsedTime);
							return false;
						}
					}), m_skyboxs.end());
			}

			// Terrain
			{
				TRACER_EVENT(L"Terrain");

				thread::SRWWriteLock writeLock(&m_srwLockObjects[eTerrain]);

				m_terrains.erase(std::remove_if(m_terrains.begin(), m_terrains.end(), [&](Terrain* pTerrain)
					{
						if (pTerrain->IsDestroy() == true)
						{
							m_poolTerrain.Destroy(pTerrain);
							return true;
						}
						else
						{
							pTerrain->Update(elapsedTime);
							return false;
						}
					}), m_terrains.end());
			}

			// Actor
			{
				TRACER_EVENT(L"Actor");

				thread::SRWWriteLock writeLock(&m_srwLockObjects[eActor]);

				m_actors.erase(std::remove_if(m_actors.begin(), m_actors.end(), [&](Actor* pActor)
					{
						if (pActor->IsDestroy() == true)
						{
							m_poolActor.Destroy(pActor);
							return true;
						}
						else
						{
							pActor->Update(elapsedTime);
							return false;
						}
					}), m_actors.end());
			}
		}

		ActorPtr GameObjectManager::Impl::CreateActor(const string::StringID& actorName)
		{
			thread::SRWWriteLock writeLock(&m_srwLockObjects[eActor]);

			const IGameObject::Handle handle(m_allocateIndex++);
			Actor* pActor = m_poolActor.Allocate(handle);
			pActor->SetName(actorName);

			m_actors.emplace_back(pActor);
			m_rmapActors.emplace(handle, pActor);

			m_isDirty[ObjectType::eActor] = true;

			return ActorPtr(pActor);
		}

		void GameObjectManager::Impl::RemoveActor(IActor* pActor)
		{
			static_cast<Actor*>(pActor)->SetDestroy(true);
		}

		IActor* GameObjectManager::Impl::GetActor(const IGameObject::Handle& handle)
		{
			thread::SRWReadLock readLock(&m_srwLockObjects[eActor]);

			auto iter = m_rmapActors.find(handle);
			if (iter != m_rmapActors.end())
				return iter->second;

			return nullptr;
		}

		IActor* GameObjectManager::Impl::GetActor(size_t index)
		{
			thread::SRWReadLock readLock(&m_srwLockObjects[eActor]);

			if (index >= m_actors.size())
				return nullptr;

			return m_actors[index];
		}

		TerrainPtr GameObjectManager::Impl::CreateTerrain(const string::StringID& terrainName, const TerrainProperty& terrainProperty)
		{
			thread::SRWWriteLock writeLock(&m_srwLockObjects[eTerrain]);

			const IGameObject::Handle handle(m_allocateIndex++);
			Terrain* pTerrain = m_poolTerrain.Allocate(handle);
			pTerrain->Initialize(terrainProperty, false);

			pTerrain->SetName(terrainName);

			m_terrains.emplace_back(pTerrain);
			m_rmapTerrains.emplace(handle, pTerrain);

			m_isDirty[ObjectType::eTerrain] = true;

			return TerrainPtr(pTerrain);
		}

		TerrainPtr GameObjectManager::Impl::CreateTerrainAsync(const string::StringID& terrainName, const TerrainProperty& terrainProperty)
		{
			thread::SRWWriteLock writeLock(&m_srwLockObjects[eTerrain]);

			const IGameObject::Handle handle(m_allocateIndex++);
			Terrain* pTerrain = m_poolTerrain.Allocate(handle);
			pTerrain->Initialize(terrainProperty, true);
			pTerrain->SetName(terrainName);

			m_terrains.emplace_back(pTerrain);
			m_rmapTerrains.emplace(handle, pTerrain);

			m_isDirty[ObjectType::eTerrain] = true;

			return TerrainPtr(pTerrain);
		}

		void GameObjectManager::Impl::RemoveTerrain(ITerrain* pTerrain)
		{
			static_cast<Terrain*>(pTerrain)->SetDestroy(true);
		}

		ITerrain* GameObjectManager::Impl::GetTerrain(const IGameObject::Handle& handle)
		{
			thread::SRWReadLock readLock(&m_srwLockObjects[eTerrain]);

			auto iter = m_rmapTerrains.find(handle);
			if (iter != m_rmapTerrains.end())
				return iter->second;

			return nullptr;
		}

		ITerrain* GameObjectManager::Impl::GetTerrain(size_t index)
		{
			thread::SRWReadLock readLock(&m_srwLockObjects[eTerrain]);

			if (index >= m_terrains.size())
				return nullptr;

			return m_terrains[index];
		}

		SkyboxPtr GameObjectManager::Impl::CreateSkybox(const string::StringID& name, const SkyboxProperty& skyProperty)
		{
			thread::SRWWriteLock writeLock(&m_srwLockObjects[eSky]);

			const IGameObject::Handle handle(m_allocateIndex++);
			Skybox* pSkybox = m_poolSkybox.Allocate(handle);
			pSkybox->Initialize(skyProperty);
			pSkybox->SetName(name);

			m_skyboxs.emplace_back(pSkybox);
			m_rmapSkyboxs.emplace(handle, pSkybox);

			m_isDirty[ObjectType::eSky] = true;

			return SkyboxPtr(pSkybox);
		}

		void GameObjectManager::Impl::RemoveSkybox(ISkybox* pSkybox)
		{
			static_cast<Skybox*>(pSkybox)->SetDestroy(true);
		}

		ISkybox* GameObjectManager::Impl::GetSkybox(const IGameObject::Handle& handle)
		{
			thread::SRWReadLock readLock(&m_srwLockObjects[eSky]);

			auto iter = m_rmapSkyboxs.find(handle);
			if (iter != m_rmapSkyboxs.end())
				return iter->second;

			return nullptr;
		}

		ISkybox* GameObjectManager::Impl::GetSkybox(size_t index)
		{
			thread::SRWReadLock readLock(&m_srwLockObjects[eSky]);

			if (index >= m_skyboxs.size())
				return nullptr;

			return m_skyboxs[index];
		}

		void GameObjectManager::Impl::Optimize()
		{
			if (m_isDirty[ObjectType::eActor] == true)
			{
				std::sort(m_actors.begin(), m_actors.end());
				m_isDirty[ObjectType::eActor] = false;
			}

			if (m_isDirty[ObjectType::eSky] == true)
			{
				std::sort(m_skyboxs.begin(), m_skyboxs.end());
				m_isDirty[ObjectType::eSky] = false;
			}

			if (m_isDirty[ObjectType::eTerrain] == true)
			{
				std::sort(m_terrains.begin(), m_terrains.end());
				m_isDirty[ObjectType::eTerrain] = false;
			}
		}

		GameObjectManager::GameObjectManager()
			: m_pImpl{ std::make_unique<Impl>() }
		{
		}

		GameObjectManager::~GameObjectManager()
		{
		}

		void GameObjectManager::Release()
		{
			m_pImpl->Release();
		}

		void GameObjectManager::Update(float elapsedTime)
		{
			m_pImpl->Update(elapsedTime);
		}

		ActorPtr GameObjectManager::CreateActor(const string::StringID& actorName)
		{
			return m_pImpl->CreateActor(actorName);
		}

		void GameObjectManager::RemoveActor(IActor* pActor)
		{
			return m_pImpl->RemoveActor(pActor);
		}

		IActor* GameObjectManager::GetActor(const IGameObject::Handle& handle)
		{
			return m_pImpl->GetActor(handle);
		}

		IActor* GameObjectManager::GetActor(size_t index)
		{
			return m_pImpl->GetActor(index);
		}

		size_t GameObjectManager::GetActorCount() const
		{
			return m_pImpl->GetActorCount();
		}

		void GameObjectManager::ExecuteFunction(std::function<void(IActor*)> func)
		{
			m_pImpl->ExecuteFunction(func);
		}

		TerrainPtr GameObjectManager::CreateTerrain(const string::StringID& terrainName, const TerrainProperty& terrainProperty)
		{
			return m_pImpl->CreateTerrain(terrainName, terrainProperty);
		}

		TerrainPtr GameObjectManager::CreateTerrainAsync(const string::StringID& terrainName, const TerrainProperty& terrainProperty)
		{
			return m_pImpl->CreateTerrainAsync(terrainName, terrainProperty);
		}

		void GameObjectManager::RemoveTerrain(ITerrain* pTerrain)
		{
			return m_pImpl->RemoveTerrain(pTerrain);
		}

		ITerrain* GameObjectManager::GetTerrain(const IGameObject::Handle& handle)
		{
			return m_pImpl->GetTerrain(handle);
		}

		ITerrain* GameObjectManager::GetTerrain(size_t index)
		{
			return m_pImpl->GetTerrain(index);
		}

		size_t GameObjectManager::GetTerrainCount() const
		{
			return m_pImpl->GetTerrainCount();
		}

		void GameObjectManager::ExecuteFunction(std::function<void(ITerrain*)> func)
		{
			m_pImpl->ExecuteFunction(func);
		}

		SkyboxPtr GameObjectManager::CreateSkybox(const string::StringID& skyboxName, const SkyboxProperty& skyProperty)
		{
			return m_pImpl->CreateSkybox(skyboxName, skyProperty);
		}

		void GameObjectManager::RemoveSkybox(ISkybox* pSkybox)
		{
			return m_pImpl->RemoveSkybox(pSkybox);
		}

		ISkybox* GameObjectManager::GetSkybox(const IGameObject::Handle& handle)
		{
			return m_pImpl->GetSkybox(handle);
		}

		ISkybox* GameObjectManager::GetSkybox(size_t index)
		{
			return m_pImpl->GetSkybox(index);
		}

		size_t GameObjectManager::GetSkyboxCount() const
		{
			return m_pImpl->GetSkyboxCount();
		}

		void GameObjectManager::ExecuteFunction(std::function<void(ISkybox*)> func)
		{
			m_pImpl->ExecuteFunction(func);
		}
	}
}