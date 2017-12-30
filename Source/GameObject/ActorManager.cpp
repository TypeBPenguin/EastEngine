#include "stdafx.h"
#include "ActorManager.h"

namespace StrID
{
	RegisterStringID(NoName);
}

namespace EastEngine
{
	namespace GameObject
	{
		ActorManager::ActorManager()
		{
			m_colonyActor.reserve(128);
		}

		ActorManager::~ActorManager()
		{
			Release();
		}

		void ActorManager::Release()
		{
			m_colonyActor.clear();
		}

		void ActorManager::Update(float fElapsedTime)
		{
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

		IActor* ActorManager::CreateActor(const String::StringID& strActorName)
		{
			auto iter = m_colonyActor.emplace();
			iter->SetName(strActorName);

			return &(*iter);
		}

		IActor* ActorManager::GetActor(size_t nIndex)
		{
			auto iter = m_colonyActor.begin();
			m_colonyActor.advance(iter, nIndex);

			return &(*iter);
		}
	}
}