#include "stdafx.h"
#include "ParticleMgr.h"

#include "ParticleInterface.h"

#include "../DirectX/CameraManager.h"

namespace EastEngine
{
	namespace Graphics
	{
		ParticleManager::ParticleManager()
			: m_isInit(false)
		{
		}

		ParticleManager::~ParticleManager()
		{
		}

		bool ParticleManager::Init()
		{
			if (m_isInit == true)
				return true;

			m_isInit = true;

			return true;
		}

		void ParticleManager::Release()
		{
			if (m_isInit == false)
				return;

			std::for_each(m_listParticle.begin(), m_listParticle.end(), [](IParticle* pParticle)
			{
				IParticle::Destroy(&pParticle);
			});
			m_listParticle.clear();

			m_isInit = false;
		}

		void ParticleManager::Update(float fElapsedTime)
		{
			Camera* pCamera = CameraManager::GetInstance()->GetMainCamera();
			if (pCamera == nullptr)
				return;

			const Math::Matrix& matView = pCamera->GetViewMatrix();
			Math::Matrix matViewProjection = matView * pCamera->GetProjMatrix();
			Collision::Frustum frustum = pCamera->GetFrustum();

			auto iter = m_listParticle.begin();
			while (iter != m_listParticle.end())
			{
				IParticle* pParticle = *iter;

				if (pParticle->IsAlive() == false)
				{
					IParticle::Destroy(&pParticle);

					iter = m_listParticle.erase(iter);
					continue;
				}

				pParticle->Update(fElapsedTime, matView, matViewProjection, frustum);

				++iter;
			}
		}
	}
}