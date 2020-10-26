#include "stdafx.h"
#include "ParticleDecal.h"

namespace est
{
	namespace graphics
	{
		ParticleDecal::ParticleDecal(const ParticleDecalAttributes& attributes, const MaterialPtr& pMaterial)
			: m_attributes(attributes)
			, m_pMaterial(pMaterial)
		{
		}

		ParticleDecal::~ParticleDecal()
		{
			ReleaseResource(m_pMaterial);
		}

		void ParticleDecal::Update(float fElapsedTime, const math::Matrix& matInvView, const math::Matrix& matViewProjection, const collision::Frustum& frustum)
		{
			if (m_isDirtyMatrix == true)
			{
				m_matWorld = m_attributes.transform.Compose();
				m_isDirtyMatrix = false;
			}

			//RenderSubsetParticleDecal renderSubset(m_matWorld * matViewProjection, m_matWorld, m_pMaterial);
			//RendererManager::GetInstance()->AddRender(renderSubset);
		}
	}
}