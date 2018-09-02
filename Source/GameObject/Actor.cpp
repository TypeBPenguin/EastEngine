#include "stdafx.h"
#include "Actor.h"

#include "ComponentActionState.h"
#include "ComponentModel.h"
#include "ComponentTimer.h"
#include "ComponentPhysics.h"
#include "ComponentCamera.h"

namespace eastengine
{
	namespace gameobject
	{
		Actor::Actor(const Handle& handle)
			: IActor(handle)
			, m_f3Scale(math::Vector3::One)
			, m_f3PrevScale(math::Vector3::One)
			, m_isDestroy(false)
			, m_isVisible(true)
			, m_isDirtyWorldMatrix(true)
		{
			m_pComponents.fill(nullptr);
		}

		Actor::~Actor()
		{
			for (IComponent* pComponent : m_pComponents)
			{
				SafeDelete(pComponent);
			}
			m_pComponents.fill(nullptr);
		}

		void Actor::Update(float fElapsedTime)
		{
			if (m_isDestroy == true)
				return;

			if (m_isDirtyWorldMatrix == true)
			{
				math::Matrix::Compose(m_f3Scale, m_quatPrevRotation, m_f3Pos, m_matWorld);

				m_isDirtyWorldMatrix = false;
			}
			
			for (IComponent* pComponent : m_pComponents)
			{
				if (pComponent == nullptr)
					continue;

				pComponent->Update(fElapsedTime);
			}

			m_f3Velocity = math::Vector3::Zero;
		}
		
		IComponent* Actor::CreateComponent(EmComponent::Type emComponentType)
		{
			if (m_pComponents[emComponentType] != nullptr)
				return m_pComponents[emComponentType];

			switch (emComponentType)
			{
			case EmComponent::eActionState:
				m_pComponents[emComponentType] = new ComponentActionState(this);
				break;
			case EmComponent::eTimer:
				m_pComponents[emComponentType] = new ComponentTimer(this);
				break;
			case EmComponent::eModel:
				m_pComponents[emComponentType] = new ComponentModel(this);
				break;
			case EmComponent::ePhysics:
				m_pComponents[emComponentType] = new ComponentPhysics(this);
				break;
			case EmComponent::eCamera:
				m_pComponents[emComponentType] = new ComponentCamera(this);
				break;
			default:
				return nullptr;
			}
			
			return m_pComponents[emComponentType];
		}

		void Actor::DestroyComponent(EmComponent::Type emComponentType)
		{
			if (emComponentType >= EmComponent::TypeCount)
				return;

			SafeDelete(m_pComponents[emComponentType]);
		}

		IComponent* Actor::GetComponent(EmComponent::Type emComponentType)
		{
			if (emComponentType >= EmComponent::TypeCount)
				return nullptr;

			return m_pComponents[emComponentType];
		}
	}
}