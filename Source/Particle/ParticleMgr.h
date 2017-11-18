#pragma once

#include "CommonLib/Singleton.h"

namespace EastEngine
{
	namespace Graphics
	{
		class IParticle;

		class ParticleManager : public Singleton<ParticleManager>
		{
			friend Singleton<ParticleManager>;
		private:
			ParticleManager();
			virtual ~ParticleManager();

		public:
			bool Init();
			void Release();

			void Update(float fElapsedTime);

			void AddParticle(IParticle* pParticle) { m_listParticle.emplace_back(pParticle); }

		private:
			bool m_isInit;

			std::list<IParticle*> m_listParticle;
		};
	}
}