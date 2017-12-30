#pragma once

#include "CommonLib/Singleton.h"
#include "CommonLib/plf_colony.h"

#include "Actor.h"

namespace EastEngine
{
	namespace GameObject
	{
		class ActorManager : public Singleton<ActorManager>
		{
			friend Singleton<ActorManager>;
		private:
			ActorManager();
			virtual ~ActorManager();

		public:
			void Release();

			void Update(float fElapsedTime);

			IActor* CreateActor(const String::StringID& strActorName);

			IActor* GetActor(size_t nIndex);
			size_t GetActorCount() const { return m_colonyActor.size(); }

		private:
			plf::colony<Actor> m_colonyActor;
		};
	}
}