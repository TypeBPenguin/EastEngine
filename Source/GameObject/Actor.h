#pragma once

#include "GameObject.h"

namespace eastengine
{
	namespace gameobject
	{
		class Actor : public IActor
		{
		public:
			Actor(const Handle& handle);
			virtual ~Actor();

		public:
			virtual void Update(float fElapsedTime);

			virtual IComponent* CreateComponent(EmComponent::Type emComponentType) override;
			virtual void DestroyComponent(EmComponent::Type emComponentType) override;

			virtual IComponent* GetComponent(EmComponent::Type emComponentType) override;

		public:
			virtual const string::StringID& GetName() const override { return m_strActorName; }
			virtual void SetName(const string::StringID& strActorName) override { m_strActorName = strActorName; }

			virtual const math::Matrix* GetWorldMatrixPtr() const override { return &m_matWorld; }
			virtual const math::Matrix& GetWorldMatrix() const override { return m_matWorld; }
			virtual const math::Matrix& CalcWorldMatrix() override { m_isDirtyWorldMatrix = false; m_matWorld = math::Matrix::Compose(m_f3Scale, m_quatRotation, m_f3Pos); return m_matWorld; }

			virtual const math::float3& GetPosition() const override { return m_f3Pos; }
			virtual void SetPosition(const math::float3& f3Pos) override { m_isDirtyWorldMatrix = true;  m_f3PrevPos = m_f3Pos; m_f3Pos = f3Pos; }
			virtual const math::float3& GetPrevPosition() const override { return m_f3PrevPos; }

			virtual const math::float3& GetScale() const override { return m_f3Scale; }
			virtual void SetScale(const math::float3& f3Scale) override { m_isDirtyWorldMatrix = true; m_f3PrevScale = m_f3Scale; m_f3Scale = f3Scale; }
			virtual const math::float3& GetPrevScale() const override { return m_f3PrevScale; }

			virtual const math::Quaternion& GetRotation() const override { return m_quatRotation; }
			virtual void SetRotation(const math::Quaternion& quat) override { m_isDirtyWorldMatrix = true; m_quatPrevRotation = m_quatRotation; m_quatRotation = quat; }
			virtual const math::Quaternion& GetPrevRotation() const override { return m_quatPrevRotation; }

			virtual const math::float3& GetVelocity() const override { return m_f3Velocity; }
			virtual void SetVelocity(const math::float3& f3Velocity) override { m_f3Velocity = f3Velocity; }

			virtual void SetVisible(bool bVisible) override { m_isVisible = bVisible; }
			virtual bool IsVisible() const override { return m_isVisible; }

		public:
			void SetDestroy(bool isDestroy) { m_isDestroy = isDestroy; }
			bool IsDestroy() const { return m_isDestroy; }

		private:
			std::array<IComponent*, EmComponent::TypeCount> m_pComponents;

			string::StringID m_strActorName;

			math::Matrix m_matWorld;
			math::float3 m_f3Pos;
			math::float3 m_f3PrevPos;
			math::float3 m_f3Scale;
			math::float3 m_f3PrevScale;
			math::Quaternion m_quatRotation;
			math::Quaternion m_quatPrevRotation;

			math::float3 m_f3Velocity;

			bool m_isDestroy;
			bool m_isVisible;
			bool m_isDirtyWorldMatrix;
		};
	}
}