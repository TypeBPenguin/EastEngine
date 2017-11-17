#pragma once

#include "ActorInterface.h"

namespace EastEngine
{
	namespace GameObject
	{
		class Actor : public IActor
		{
		public:
			Actor();
			virtual ~Actor();

		public:
			virtual void Update(float fElapsedTime);

			virtual IComponent* CreateComponent(EmComponent::Type emComponentType) override;
			virtual void DestroyComponent(EmComponent::Type emComponentType) override;

			virtual IComponent* GetComponent(EmComponent::Type emComponentType) override;

		public:
			virtual uint32_t GetActorID() const override { return reinterpret_cast<uint32_t>(this); }

			virtual const String::StringID& GetName() const override { return m_strActorName; }
			virtual void SetName(const String::StringID& strActorName) override { m_strActorName = strActorName; }

			virtual const Math::Matrix* GetWorldMatrixPtr() const override { return &m_matWorld; }
			virtual const Math::Matrix& GetWorldMatrix() const override { return m_matWorld; }
			virtual const Math::Matrix& CalcWorldMatrix() override { m_isDirtyWorldMatrix = false; m_matWorld = Math::Matrix::Compose(m_f3Scale, m_quatRotation, m_f3Pos); return m_matWorld; }

			virtual const Math::Vector3& GetPosition() const override { return m_f3Pos; }
			virtual void SetPosition(const Math::Vector3& f3Pos) override { m_isDirtyWorldMatrix = true;  m_f3PrevPos = m_f3Pos; m_f3Pos = f3Pos; }
			virtual const Math::Vector3& GetPrevPosition() const override { return m_f3PrevPos; }

			virtual const Math::Vector3& GetScale() const override { return m_f3Scale; }
			virtual void SetScale(const Math::Vector3& f3Scale) override { m_isDirtyWorldMatrix = true; m_f3PrevScale = m_f3Scale; m_f3Scale = f3Scale; }
			virtual const Math::Vector3& GetPrevScale() const override { return m_f3PrevScale; }

			virtual const Math::Quaternion& GetRotation() const override { return m_quatRotation; }
			virtual void SetRotation(const Math::Quaternion& quat) override { m_isDirtyWorldMatrix = true; m_quatPrevRotation = m_quatRotation; m_quatRotation = quat; }
			virtual const Math::Quaternion& GetPrevRotation() const override { return m_quatPrevRotation; }

			virtual const Math::Vector3& GetVelocity() const override { return m_f3Velocity; }
			virtual void SetVelocity(const Math::Vector3& f3Velocity) override { m_f3Velocity = f3Velocity; }

			virtual void SetVisible(bool bVisible) override { m_isVisible = bVisible; }
			virtual bool IsVisible() const override { return m_isVisible; }

		public:
			void SetDestroy(bool isDestroy) { m_isDestroy = isDestroy; }
			bool IsDestroy() const { return m_isDestroy; }

		private:
			std::array<IComponent*, EmComponent::TypeCount> m_pComponents;

			String::StringID m_strActorName;

			Math::Matrix m_matWorld;
			Math::Vector3 m_f3Pos;
			Math::Vector3 m_f3PrevPos;
			Math::Vector3 m_f3Scale;
			Math::Vector3 m_f3PrevScale;
			Math::Quaternion m_quatRotation;
			Math::Quaternion m_quatPrevRotation;

			Math::Vector3 m_f3Velocity;

			bool m_isDestroy;
			bool m_isVisible;
			bool m_isDirtyWorldMatrix;
		};
	}
}