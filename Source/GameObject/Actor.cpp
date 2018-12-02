#include "stdafx.h"
#include "Actor.h"

#include "ComponentTimer.h"
#include "ComponentBehaviorTree.h"
#include "ComponentFiniteStateMachine.h"
#include "ComponentPhysics.h"
#include "ComponentModel.h"
#include "ComponentCamera.h"

namespace eastengine
{
	namespace gameobject
	{
		Actor::Actor(const Handle& handle)
			: IActor(handle)
		{
		}

		Actor::~Actor()
		{
		}

		void Actor::Update(float elapsedTime)
		{
			if (m_isDestroy == true)
				return;

			if (m_isDirtyWorldMatrix == true)
			{
				m_matWorld = m_transform.Compose();
				m_isDirtyWorldMatrix = false;
			}
			
			for (auto& pComponent : m_pComponents)
			{
				if (pComponent == nullptr)
					continue;

				pComponent->Update(elapsedTime);
			}

			m_f3Velocity = math::float3::Zero;
		}
		
		IComponent* Actor::CreateComponent(IComponent::Type emComponentType)
		{
			if (m_pComponents[emComponentType] != nullptr)
				return m_pComponents[emComponentType];

			switch (emComponentType)
			{
			case IComponent::eTimer:
				m_pComponents[emComponentType] = new ComponentTimer(this);
				break;
			case IComponent::eBehaviorTree:
				m_pComponents[emComponentType] = new ComponentBehaviorTree(this);
				break;
			case IComponent::eFiniteStateMachine:
				m_pComponents[emComponentType] = new ComponentFiniteStateMachine(this);
				break;
			case IComponent::eModel:
				m_pComponents[emComponentType] = new ComponentModel(this);
				break;
			case IComponent::ePhysics:
				m_pComponents[emComponentType] = new ComponentPhysics(this);
				break;
			case IComponent::eCamera:
				m_pComponents[emComponentType] = new ComponentCamera(this);
				break;
			default:
				return nullptr;
			}
			
			return m_pComponents[emComponentType];
		}

		void Actor::DestroyComponent(IComponent::Type emComponentType)
		{
			if (emComponentType >= IComponent::TypeCount)
				return;

			SafeDelete(m_pComponents[emComponentType]);
		}

		IComponent* Actor::GetComponent(IComponent::Type emComponentType)
		{
			if (emComponentType >= IComponent::TypeCount)
				return nullptr;

			return m_pComponents[emComponentType];
		}
	}
};