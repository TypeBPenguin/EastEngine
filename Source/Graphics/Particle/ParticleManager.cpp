#include "stdafx.h"
#include "ParticleManager.h"

#include "CommonLib/Lock.h"
#include "CommonLib/ObjectPool.h"

#include "Graphics/Interface/Camera.h"

#include "ParticleEmitter.h"
#include "ParticleDecal.h"

namespace est
{
	namespace graphics
	{
		class ParticleManager::Impl
		{
		public:
			Impl();
			~Impl();

		public:
			void Update(Camera* pCamera, float elapsedTime);
			void Cleanup(float elapsedTime);

		public:
			ParticleEmitterPtr CreateParticle(const ParticleEmitterAttributes& attributes);
			ParticleDecalPtr CreateParticle(const ParticleDecalAttributes& attributes);

			void DestroyParticle(const ParticlePtr& pParticle);

		private:
			thread::SRWLock m_srwLock_emitter;
			memory::ObjectPool<ParticleEmitter, 256> m_poolParticleEmitter;
			std::vector<std::shared_ptr<ParticleEmitter>> m_particleEmitters;

			thread::SRWLock m_srwLock_decal;
			memory::ObjectPool<ParticleDecal, 64> m_poolParticleDecal;
			std::vector<std::shared_ptr<ParticleDecal>> m_particleDecals;
		};

		ParticleManager::Impl::Impl()
		{
		}

		ParticleManager::Impl::~Impl()
		{
		}

		void ParticleManager::Impl::Update(Camera* pCamera, float elapsedTime)
		{
			TRACER_EVENT(__FUNCTIONW__);

			const math::Matrix matInvView = pCamera->GetViewMatrix().Invert();
			const math::Matrix matViewProjection = pCamera->GetViewMatrix() * pCamera->GetProjectionMatrix();
			const collision::Frustum& frustum = pCamera->GetFrustum();

			{
				TRACER_EVENT(L"ParticleEmitter");
				thread::SRWWriteLock wirteLock(&m_srwLock_emitter);

				jobsystem::ParallelFor(m_particleEmitters.size(), [&](const size_t index)
					{
						const std::shared_ptr<ParticleEmitter>& pEmitter = m_particleEmitters[index];
						if (pEmitter->GetState() == IParticle::State::eRunning)
						{
							pEmitter->Update(elapsedTime, matInvView, matViewProjection, frustum);
						}
					});
			}
		}

		void ParticleManager::Impl::Cleanup(float elapsedTime)
		{
			TRACER_EVENT(__FUNCTIONW__);

			{
				TRACER_EVENT(L"Emitter");
				thread::SRWWriteLock wirteLock(&m_srwLock_emitter);

				m_particleEmitters.erase(std::remove_if(m_particleEmitters.begin(), m_particleEmitters.end(), [](std::shared_ptr<ParticleEmitter>& pEmitter)
					{
						return pEmitter->GetState() == IParticle::State::eStop;
					}), m_particleEmitters.end());
			}

			{
				TRACER_EVENT(L"Decal");
				thread::SRWWriteLock wirteLock(&m_srwLock_decal);

				m_particleDecals.erase(std::remove_if(m_particleDecals.begin(), m_particleDecals.end(), [](std::shared_ptr<ParticleDecal>& pDecal)
					{
						return pDecal->GetState() == IParticle::State::eStop;
					}), m_particleDecals.end());
			}
		}

		ParticleEmitterPtr ParticleManager::Impl::CreateParticle(const ParticleEmitterAttributes& attributes)
		{
			if (attributes.textureFilePath.empty() == true)
				return nullptr;

			TexturePtr pTexture = nullptr;
			if (attributes.isEnableAsyncLoadTexture == true)
			{
				pTexture = CreateTextureAsync(attributes.textureFilePath.c_str());
			}
			else
			{
				pTexture = CreateTexture(attributes.textureFilePath.c_str());
			}

			if (pTexture->GetState() == IResource::State::eInvalid)
			{
				ReleaseResource(pTexture);
				return nullptr;
			}

			thread::SRWWriteLock wirteLock(&m_srwLock_emitter);
			std::shared_ptr<ParticleEmitter> pParticle = std::shared_ptr<ParticleEmitter>(m_poolParticleEmitter.Allocate(attributes, pTexture), [&](ParticleEmitter* pParticle)
				{
					thread::SRWWriteLock wirteLock(&m_srwLock_emitter);
					m_poolParticleEmitter.Destroy(pParticle);
				});
			m_particleEmitters.emplace_back(pParticle);
			return pParticle;
		}

		ParticleDecalPtr ParticleManager::Impl::CreateParticle(const ParticleDecalAttributes& attributes)
		{
			MaterialPtr pMaterial = CreateMaterial(&attributes.materialData);
			if (pMaterial == nullptr)
				return nullptr;

			if (pMaterial->GetState() == IResource::State::eInvalid)
			{
				ReleaseResource(pMaterial);
				return nullptr;
			}

			thread::SRWWriteLock wirteLock(&m_srwLock_decal);
			std::shared_ptr<ParticleDecal> pParticle = std::shared_ptr<ParticleDecal>(m_poolParticleDecal.Allocate(attributes, pMaterial), [&](ParticleDecal* pParticle)
				{
					thread::SRWWriteLock wirteLock(&m_srwLock_decal);
					m_poolParticleDecal.Destroy(pParticle);
				});
			m_particleDecals.emplace_back(pParticle);
			return pParticle;
		}

		void ParticleManager::Impl::DestroyParticle(const ParticlePtr& pParticle)
		{
			pParticle->Stop();
		}

		ParticleManager::ParticleManager()
			: m_pImpl{ std::make_unique<Impl>() }
		{
		}

		ParticleManager::~ParticleManager()
		{
		}

		void ParticleManager::Update(Camera* pCamera, float elapsedTime)
		{
			m_pImpl->Update(pCamera, elapsedTime);
		}

		void ParticleManager::Cleanup(float elapsedTime)
		{
			m_pImpl->Cleanup(elapsedTime);
		}

		ParticleEmitterPtr ParticleManager::CreateParticle(const ParticleEmitterAttributes& attributes)
		{
			return m_pImpl->CreateParticle(attributes);
		}

		ParticleDecalPtr ParticleManager::CreateParticle(const ParticleDecalAttributes& attributes)
		{
			return m_pImpl->CreateParticle(attributes);
		}

		void ParticleManager::DestroyParticle(const ParticlePtr& pParticle)
		{
			m_pImpl->DestroyParticle(pParticle);
		}
	}
}