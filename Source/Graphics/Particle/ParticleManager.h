#pragma once

#include "CommonLib/Singleton.h"

#include "ParticleInterface.h"

namespace est
{
	namespace graphics
	{
		class Camera;

		class ParticleManager : public Singleton<ParticleManager>
		{
			friend Singleton<ParticleManager>;
		private:
			ParticleManager();
			virtual ~ParticleManager();

		public:
			void Update(Camera* pCamera, float elapsedTime);
			void Cleanup(float elapsedTime);

		public:
			ParticleEmitterPtr CreateParticle(const ParticleEmitterAttributes& attributes);
			ParticleDecalPtr CreateParticle(const ParticleDecalAttributes& attributes);

			void DestroyParticle(const ParticlePtr& pParticle);

		private:
			class Impl;
			std::unique_ptr<Impl> m_pImpl;
		};
	}
}