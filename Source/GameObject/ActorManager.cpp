#include "stdafx.h"
#include "ActorManager.h"

#include "CommonLib/plf_colony.h"

#include "Actor.h"

namespace EastEngine
{
	namespace GameObject
	{
		class ActorManager::Impl
		{
		public:
			Impl();
			~Impl();

		public:
			void Update(float fElapsedTime);

		public:
			IActor* CreateActor(const String::StringID& strActorName);

			IActor* GetActor(size_t nIndex);
			size_t GetActorCount() const;

		private:
			plf::colony<Actor> m_colonyActor;
		};

		ActorManager::Impl::Impl()
		{
			m_colonyActor.reserve(128);
		}

		ActorManager::Impl::~Impl()
		{
		}

		void ActorManager::Impl::Update(float fElapsedTime)
		{
			PERF_TRACER_EVENT("ActorManager::Update", "");
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

				actor.Update(fElapsedTime);

				++iter;
			}
		}

		IActor* ActorManager::Impl::CreateActor(const String::StringID& strActorName)
		{
			auto iter = m_colonyActor.emplace();
			iter->SetName(strActorName);

			return &(*iter);
		}

		IActor* ActorManager::Impl::GetActor(size_t nIndex)
		{
			auto iter = m_colonyActor.begin();
			m_colonyActor.advance(iter, nIndex);

			return &(*iter);
		}

		size_t ActorManager::Impl::GetActorCount() const
		{
			return m_colonyActor.size();
		}

		ActorManager::ActorManager()
			: m_pImpl{ std::make_unique<Impl>() }
		{
		}

		ActorManager::~ActorManager()
		{
		}

		void ActorManager::Update(float fElapsedTime)
		{
			m_pImpl->Update(fElapsedTime);
		}

		IActor* ActorManager::CreateActor(const String::StringID& strActorName)
		{
			return m_pImpl->CreateActor(strActorName);
		}

		IActor* ActorManager::GetActor(size_t nIndex)
		{
			return m_pImpl->GetActor(nIndex);
		}
		size_t ActorManager::GetActorCount() const
		{
			return m_pImpl->GetActorCount();
		}
	}
}