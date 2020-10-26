#include "stdafx.h"
#include "Shadow.h"

#include "Light.h"

namespace est
{
	namespace graphics
	{
		Shadow::Shadow()
		{
		}

		Shadow::~Shadow()
		{
		}

		void Shadow::Update(const ISpotLight* pSpotLight)
		{
			Update(pSpotLight->GetPosition(), pSpotLight->GetDirection(), pSpotLight->GetIntensity(), pSpotLight->GetAngle());
		}

		void Shadow::Update(const math::float3& position, const math::float3& direction, float intensity, float angle)
		{
			constexpr float attenuation = 0.01f;
			constexpr float invAttenuation = 1.f / attenuation;

			const float depth = std::sqrtf(invAttenuation * intensity * math::PI);
			const float fov = math::ToRadians(angle);

			const math::float3 viewPosition = position - (direction * depth * 0.1f);
			m_matView = math::Matrix::CreateLookAt(viewPosition, position, math::float3::Up);

			m_matProjection = math::Matrix::CreatePerspectiveFieldOfView(fov, 1.f, 0.01f, depth);

			collision::Frustum::CreateFromMatrix(m_frustum, m_matProjection);
			m_frustum.Transform(m_matView.Invert());

			// 아래 검증 필요
			m_calcDepthBias = m_config.depthBias * /*(1.f / depth * 0.01f) **/ (1.f / (depth * fov));
			//m_calcDepthBias = m_config.depthBias * (1.f / depth * 0.01f);
		}
	}
}