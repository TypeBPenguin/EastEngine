#include "stdafx.h"
#include "Actor.h"

#include "ComponentTimer.h"
#include "ComponentBehaviorTree.h"
#include "ComponentFiniteStateMachine.h"
#include "ComponentPhysics.h"
#include "ComponentModel.h"
#include "ComponentCamera.h"

namespace est
{
	namespace gameobject
	{
		Actor::Actor(const Handle& handle)
			: IActor(handle)
		{
		}

		Actor::Actor(Actor&& source) noexcept
			: IActor(source.GetHandle())
			, m_isDestroy(std::move(source.m_isDestroy))
			, m_isVisible(std::move(source.m_isVisible))
			, m_isDirtyWorldMatrix(std::move(source.m_isDirtyWorldMatrix))
			, m_pComponents(std::move(source.m_pComponents))
			, m_name(std::move(source.m_name))
			, m_matWorld(std::move(source.m_matWorld))
			, m_transform(std::move(source.m_transform))
			, m_prevTransform(std::move(source.m_prevTransform))
		{
		}

		Actor::~Actor()
		{
		}
		
		IComponent* Actor::CreateComponent(IComponent::Type emComponentType)
		{
			if (m_pComponents[emComponentType] != nullptr)
				return m_pComponents[emComponentType].get();

			switch (emComponentType)
			{
			case IComponent::eTimer:
				m_pComponents[emComponentType] = std::make_unique<ComponentTimer>(this);
				break;
			case IComponent::eBehaviorTree:
				m_pComponents[emComponentType] = std::make_unique<ComponentBehaviorTree>(this);
				break;
			case IComponent::eFiniteStateMachine:
				m_pComponents[emComponentType] = std::make_unique<ComponentFiniteStateMachine>(this);
				break;
			case IComponent::eModel:
				m_pComponents[emComponentType] = std::make_unique<ComponentModel>(this);
				break;
			case IComponent::ePhysics:
				m_pComponents[emComponentType] = std::make_unique<ComponentPhysics>(this);
				break;
			case IComponent::eCamera:
				m_pComponents[emComponentType] = std::make_unique<ComponentCamera>(this);
				break;
			default:
				return nullptr;
			}
			
			return m_pComponents[emComponentType].get();
		}

		void Actor::DestroyComponent(IComponent::Type emComponentType)
		{
			if (emComponentType >= IComponent::TypeCount)
				return;

			m_pComponents[emComponentType].reset();
		}

		IComponent* Actor::GetComponent(IComponent::Type emComponentType)
		{
			if (emComponentType >= IComponent::TypeCount)
				return nullptr;

			return m_pComponents[emComponentType].get();
		}

		const math::Matrix& Actor::GetWorldMatrix()
		{
			if (m_isDirtyWorldMatrix == true)
			{
				m_matWorld = m_transform.Compose();
			}
			return m_matWorld;
		}

		void Actor::Update(float elapsedTime, float lodThreshold)
		{
			if (m_isDestroy == true)
				return;

			for (auto& pComponent : m_pComponents)
			{
				if (pComponent == nullptr)
					continue;

				pComponent->Update(elapsedTime, lodThreshold);
			}
		}
	}
};