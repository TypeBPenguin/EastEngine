#pragma once

#include "GameObject.h"

namespace est
{
	namespace gameobject
	{
		class Actor : public IActor
		{
		public:
			Actor(const Handle& handle);
			Actor(const Actor& source) = delete;
			Actor(Actor&& source) noexcept;
			virtual ~Actor();

		public:
			virtual void Update(float elapsedTime);

			virtual IComponent* CreateComponent(IComponent::Type emComponentType) override;
			virtual void DestroyComponent(IComponent::Type emComponentType) override;

			virtual IComponent* GetComponent(IComponent::Type emComponentType) override;

		public:
			virtual const string::StringID& GetName() const override { return m_name; }
			virtual void SetName(const string::StringID& name) override { m_name = name; }

			virtual const math::Matrix& GetWorldMatrix() override;

			virtual const math::float3& GetPosition() const override { return m_transform.position; }
			virtual void SetPosition(const math::float3& f3Pos) override { m_isDirtyWorldMatrix = true;  m_prevTransform.position = m_transform.position; m_transform.position = f3Pos; }
			virtual const math::float3& GetPrevPosition() const override { return m_prevTransform.position; }

			virtual const math::float3& GetScale() const override { return m_transform.scale; }
			virtual void SetScale(const math::float3& f3Scale) override { m_isDirtyWorldMatrix = true; m_prevTransform.scale = m_transform.scale; m_transform.scale = f3Scale; }
			virtual const math::float3& GetPrevScale() const override { return m_prevTransform.scale; }

			virtual const math::Quaternion& GetRotation() const override { return m_transform.rotation; }
			virtual void SetRotation(const math::Quaternion& quat) override { m_isDirtyWorldMatrix = true; m_prevTransform.rotation = m_transform.rotation; m_transform.rotation = quat; }
			virtual const math::Quaternion& GetPrevRotation() const override { return m_prevTransform.rotation; }

			virtual void SetVisible(bool bVisible) override { m_isVisible = bVisible; }
			virtual bool IsVisible() const override { return m_isVisible; }

		public:
			void SetDestroy(bool isDestroy) { m_isDestroy = isDestroy; }
			bool IsDestroy() const { return m_isDestroy; }

		private:
			bool m_isDestroy{ false };
			bool m_isVisible{ true };
			bool m_isDirtyWorldMatrix{ true };

			string::StringID m_name;

			math::Matrix m_matWorld;
			math::Transform m_transform;
			math::Transform m_prevTransform;

			std::array<std::unique_ptr<IComponent>, IComponent::TypeCount> m_pComponents;
		};
	}
}