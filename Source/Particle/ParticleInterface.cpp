#include "stdafx.h"
#include "ParticleInterface.h"

#include "CommonLib/FileUtil.h"

#include "ParticleDecal.h"
#include "ParticleEmitter.h"

namespace EastEngine
{
	namespace Graphics
	{
		static boost::object_pool<ParticleEmitter> s_poolEmitter;
		static boost::object_pool<ParticleDecal> s_poolDecal;

		IParticle::IParticle(EmParticle::Type emEffectType)
			: m_emEffectType(emEffectType)
			, m_isStart(false)
			, m_isPause(false)
			, m_isAlive(false)
		{
		}

		void IParticle::Destroy(IParticle** ppParticle)
		{
			if (ppParticle == nullptr || *ppParticle == nullptr)
				return;

			if ((*ppParticle)->IsAlive() == true)
			{
				(*ppParticle)->SetAlive(false);
				*ppParticle = nullptr;
				return;
			}

			switch ((*ppParticle)->GetType())
			{
			case EmParticle::eEmitter:
			{
				IParticleEmitter* pEmitter = static_cast<IParticleEmitter*>(*ppParticle);
				IParticleEmitter::Destroy(&pEmitter);
			}
			break;
			case EmParticle::eDecal:
			{
				IParticleDecal* pDecal = static_cast<IParticleDecal*>(*ppParticle);
				IParticleDecal::Destroy(&pDecal);
			}
			break;
			}
		}

		IParticleEmitter::IParticleEmitter()
			: IParticle(EmParticle::eEmitter)
		{
		}

		IParticleEmitter* IParticleEmitter::Create(const ParticleEmitterAttributes& attributes)
		{
			ParticleEmitter* pEmitter = s_poolEmitter.construct();
			if (pEmitter->Init(attributes) == false)
			{
				s_poolEmitter.destroy(pEmitter);
				return nullptr;
			}

			return pEmitter;
		}

		void IParticleEmitter::Destroy(IParticleEmitter** ppParticle)
		{
			if (ppParticle == nullptr || *ppParticle == nullptr)
				return;

			if ((*ppParticle)->GetType() != EmParticle::eEmitter)
				return;

			ParticleEmitter* pEmitter = static_cast<ParticleEmitter*>(*ppParticle);

			s_poolEmitter.destroy(pEmitter);
			*ppParticle = nullptr;
		}

		IParticleDecal::IParticleDecal()
			: IParticle(EmParticle::eDecal)
		{
		}

		IParticleDecal* IParticleDecal::Create(const ParticleDecalAttributes& attributes)
		{
			ParticleDecal* pDecal = s_poolDecal.construct();
			if (pDecal->Init(attributes) == false)
			{
				s_poolDecal.destroy(pDecal);
				return nullptr;
			}

			return pDecal;
		}

		void IParticleDecal::Destroy(IParticleDecal** ppParticle)
		{
			if (ppParticle == nullptr || *ppParticle == nullptr)
				return;

			if ((*ppParticle)->GetType() != EmParticle::eDecal)
				return;

			ParticleDecal* pDecal = static_cast<ParticleDecal*>(*ppParticle);

			s_poolDecal.destroy(pDecal);

			*ppParticle = nullptr;
		}
	}
}