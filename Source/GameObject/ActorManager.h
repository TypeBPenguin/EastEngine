#pragma once

#include "CommonLib/Singleton.h"

#include "GameObject.h"

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
			void Update(float fElapsedTime);

		public:
			IActor* CreateActor(const String::StringID& strActorName);

			IActor* GetActor(size_t nIndex);
			size_t GetActorCount() const;

		private:
			class Impl;
			std::unique_ptr<Impl> m_pImpl;
		};
	}
}