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
			virtual void Update(float elapsedTime);

			virtual IComponent* CreateComponent(IComponent::Type emComponentType) override;
			virtual void DestroyComponent(IComponent::Type emComponentType) override;

			virtual IComponent* GetComponent(IComponent::Type emComponentType) override;

		public:
			virtual const string::StringID& GetName() const override { return m_name; }
			virtual void SetName(const string::StringID& name) override { m_name = name; }

			virtual const math::Matrix* GetWorldMatrixPtr() const override { return &m_matWorld; }
			virtual const math::Matrix& GetWorldMatrix() const override { return m_matWorld; }
			virtual const math::Matrix& CalcWorldMatrix() override { m_isDirtyWorldMatrix = false; m_matWorld = m_transform.Compose(); return m_matWorld; }

			virtual const math::float3& GetPosition() const override { return m_transform.position; }
			virtual void SetPosition(const math::float3& f3Pos) override { m_isDirtyWorldMatrix = true;  m_prevTransform.position = m_transform.position; m_transform.position = f3Pos; }
			virtual const math::float3& GetPrevPosition() const override { return m_prevTransform.position; }

			virtual const math::float3& GetScale() const override { return m_transform.scale; }
			virtual void SetScale(const math::float3& f3Scale) override { m_isDirtyWorldMatrix = true; m_prevTransform.scale = m_transform.scale; m_transform.scale = f3Scale; }
			virtual const math::float3& GetPrevScale() const override { return m_prevTransform.scale; }

			virtual const math::Quaternion& GetRotation() const override { return m_transform.rotation; }
			virtual void SetRotation(const math::Quaternion& quat) override { m_isDirtyWorldMatrix = true; m_prevTransform.rotation = m_transform.rotation; m_transform.rotation = quat; }
			virtual const math::Quaternion& GetPrevRotation() const override { return m_prevTransform.rotation; }

			virtual const math::float3& GetVelocity() const override { return m_f3Velocity; }
			virtual void SetVelocity(const math::float3& f3Velocity) override { m_f3Velocity = f3Velocity; }

			virtual void SetVisible(bool bVisible) override { m_isVisible = bVisible; }
			virtual bool IsVisible() const override { return m_isVisible; }

		public:
			void SetDestroy(bool isDestroy) { m_isDestroy = isDestroy; }
			bool IsDestroy() const { return m_isDestroy; }

		private:
			std::array<IComponent*, IComponent::TypeCount> m_pComponents{ nullptr };

			string::StringID m_name;

			math::Matrix m_matWorld;
			math::Transform m_transform;
			math::Transform m_prevTransform;

			math::float3 m_f3Velocity;

			bool m_isDestroy{ false };
			bool m_isVisible{ true };
			bool m_isDirtyWorldMatrix{ true };
		};
	}
}