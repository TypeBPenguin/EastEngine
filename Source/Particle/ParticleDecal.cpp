#include "stdafx.h"
#include "ParticleDecal.h"

#include "ParticleMgr.h"

#include "DirectX/D3DInterface.h"
#include "Renderer/RendererManager.h"

namespace EastEngine
{
	namespace Graphics
	{
		ParticleDecal::ParticleDecal()
			: m_isDirtyWorldMatrix(true)
			, m_fTime(0.f)
			, m_fLastUpdate(0.f)
			, m_pMaterial(nullptr)
		{
		}

		ParticleDecal::~ParticleDecal()
		{
			IMaterial::Destroy(&m_pMaterial);
		}

		bool ParticleDecal::Init(const ParticleDecalAttributes& attributes)
		{
			m_pMaterial = IMaterial::Create(attributes.pMaterialInfo);
			if (m_pMaterial == nullptr)
				return false;

			m_pMaterial->IncreaseReference();

			m_attributes = attributes;

			SetAlive(true);

			ParticleManager::GetInstance()->AddParticle(this);

			return true;
		}

		void ParticleDecal::Update(float fElapsedTime, const Math::Matrix& matView, const Math::Matrix& matViewProjection, const Collision::Frustum& frustum)
		{
			if (m_isDirtyWorldMatrix == true)
			{
				m_matWorld = Math::Matrix::Compose(m_attributes.f3Scale, m_attributes.quatRot, m_attributes.f3Pos);

				m_isDirtyWorldMatrix = false;
			}

			RenderSubsetParticleDecal renderSubset(m_matWorld * matViewProjection, m_matWorld, m_pMaterial);
			RendererManager::GetInstance()->AddRender(renderSubset);
		}
	}
}