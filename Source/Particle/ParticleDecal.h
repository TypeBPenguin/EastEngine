#pragma once

#include "ParticleInterface.h"

namespace eastengine
{
	namespace graphics
	{
		class IMaterial;

		class ParticleDecal : public IParticleDecal
		{
		public:
			ParticleDecal();
			virtual ~ParticleDecal();

		public:
			bool Init(const ParticleDecalAttributes& attributes);

			virtual void Update(float fElapsedTime, const math::Matrix& matView, const math::Matrix& matViewProjection, const Collision::Frustum& frustum) override;

		public:
			const math::Vector3& GetScale() { return m_attributes.f3Scale; }
			void SetScale(const math::Vector3 f3Scale) { m_attributes.f3Scale = f3Scale; m_isDirtyWorldMatrix = true; }
			const math::Vector3& GetPosition() { return m_attributes.f3Pos; }
			void SetPosition(const math::Vector3& f3Pos) { m_attributes.f3Pos = f3Pos; m_isDirtyWorldMatrix = true; }
			const math::Quaternion& GetRotation() { return m_attributes.quatRot; }
			void SetRotation(const math::Quaternion& quat) { m_attributes.quatRot = quat; m_isDirtyWorldMatrix = true; }

		private:
			bool m_isDirtyWorldMatrix;

			float m_fTime;
			float m_fLastUpdate;
			float m_fSafeTime;

			math::Matrix m_matWorld;

			ParticleDecalAttributes m_attributes;

			IMaterial* m_pMaterial;
		};
	}
}