#include "stdafx.h"
#include "ComponentInterface.h"

namespace eastengine
{
	namespace gameobject
	{
		const char* IComponent::ToString(Type emType)
		{
			switch (emType)
			{
			case eTimer:
				return "Timer";
			case eBehaviorTree:
				return "BehaviorTree";
			case ePhysics:
				return "Physics";
			case eModel:
				return "Model";
			case eCamera:
				return "Camera";
			case eLight:
				return "Light";
			default:
				return nullptr;
			}
		}

		IComponent::Type IComponent::GetType(const char* strType)
		{
			if (string::IsEquals(strType, "BehaviorTree"))
			{
				return IComponent::eBehaviorTree;
			}
			else if (string::IsEquals(strType, "Timer"))
			{
				return IComponent::eTimer;
			}
			else if (string::IsEquals(strType, "Physics"))
			{
				return IComponent::ePhysics;
			}
			else if (string::IsEquals(strType, "Model"))
			{
				return IComponent::eModel;
			}
			else if (string::IsEquals(strType, "Camera"))
			{
				return IComponent::eCamera;
			}

			return IComponent::TypeCount;
		}

		IComponent::IComponent(IActor* pOwner, Type emCompType)
			: m_pOwner(pOwner)
			, m_emCompType(emCompType)
		{
		}

		IComponent::~IComponent()
		{
			std::for_each(m_umapChild.begin(), m_umapChild.end(), DeleteSTLMapObject());
			m_umapChild.clear();
		}

		IComponent* IComponent::AddComponent(IComponent* pComponent)
		{
			auto iter = m_umapChild.find(pComponent->GetComponentType());
			if (iter != m_umapChild.end())
			{
				SafeDelete(pComponent);
				return nullptr;
			}

			m_umapChild.insert(std::make_pair(pComponent->GetComponentType(), pComponent));

			return pComponent;
		}

		void IComponent::DelComponent(Type emComponentType)
		{
			auto iter = m_umapChild.find(emComponentType);
			if (iter == m_umapChild.end())
				return;

			m_umapChild.erase(iter);
		}

		IComponent* IComponent::GetComponent(Type emComponentType)
		{
			auto iter = m_umapChild.find(emComponentType);
			if (iter == m_umapChild.end())
				return nullptr;

			return iter->second;
		}
	}
}