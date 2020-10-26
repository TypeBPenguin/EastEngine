#pragma once

#include "ParticleInterface.h"

namespace est
{
	namespace graphics
	{
		class IMaterial;

		class ParticleDecal : public IParticleDecal
		{
		public:
			ParticleDecal(const ParticleDecalAttributes& attributes, const MaterialPtr& pMaterial);
			virtual ~ParticleDecal();

		public:
			virtual void Update(float fElapsedTime, const math::Matrix& matView, const math::Matrix& matProjection, const collision::Frustum& frustum) override;

		public:
			virtual const math::float3& GetScale() const override { return m_attributes.transform.scale; }
			virtual void SetScale(const math::float3& scale) override { m_attributes.transform.scale = scale; m_isDirtyMatrix = true; }
			virtual const math::float3& GetPosition() const override { return m_attributes.transform.position; }
			virtual void SetPosition(const math::float3& position) override { m_attributes.transform.position = position; m_isDirtyMatrix = true; }
			virtual const math::Quaternion& GetRotation() const override { return m_attributes.transform.rotation; }
			virtual void SetRotation(const math::Quaternion& rotate) override { m_attributes.transform.rotation = rotate; m_isDirtyMatrix = true; }

		private:
			ParticleDecalAttributes m_attributes;
			MaterialPtr m_pMaterial{ nullptr };

			bool m_isDirtyMatrix{ true };

			float m_updateTime{ 0.f };
			float m_lastUpdateTime{ 0.f };

			math::Matrix m_matWorld;
		};
	}
}