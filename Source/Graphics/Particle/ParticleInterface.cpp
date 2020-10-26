#include "stdafx.h"
#include "ParticleInterface.h"

#include "ParticleManager.h"

namespace est
{
	namespace graphics
	{
		ParticleEmitterPtr CreateParticle(const ParticleEmitterAttributes& attributes)
		{
			return ParticleManager::GetInstance()->CreateParticle(attributes);
		}

		ParticleDecalPtr CreateParticle(const ParticleDecalAttributes& attributes)
		{
			return ParticleManager::GetInstance()->CreateParticle(attributes);
		}

		void DestroyParticle(ParticleEmitterPtr& pParticle)
		{
			if (pParticle == nullptr)
				return;

			ParticleManager::GetInstance()->DestroyParticle(pParticle);
			pParticle = nullptr;
		}

		void DestroyParticle(ParticleDecalPtr& pParticle)
		{
			if (pParticle == nullptr)
				return;

			ParticleManager::GetInstance()->DestroyParticle(pParticle);
			pParticle = nullptr;
		}
	}
}