#include "stdafx.h"
#include "ComponentInterface.h"

namespace EastEngine
{
	namespace GameObject
	{
		namespace EmComponent
		{
			const char* ToString(Type emType)
			{
				switch (emType)
				{
				case EmComponent::eActionState:
					return "ActionState";
					break;
				case EmComponent::eTimer:
					return "Timer";
					break;
				case EmComponent::ePhysics:
					return "Physics";
					break;
				case EmComponent::eModel:
					return "Model";
					break;
				case EmComponent::eCamera:
					return "Camera";
					break;
				}

				return nullptr;
			}

			Type GetType(const char* strType)
			{
				if (String::IsEquals(strType, "ActionState"))
				{
					return GameObject::EmComponent::eActionState;
				}
				else if (String::IsEquals(strType, "Timer"))
				{
					return GameObject::EmComponent::eTimer;
				}
				else if (String::IsEquals(strType, "Physics"))
				{
					return GameObject::EmComponent::ePhysics;
				}
				else if (String::IsEquals(strType, "Model"))
				{
					return GameObject::EmComponent::eModel;
				}
				else if (String::IsEquals(strType, "Camera"))
				{
					return GameObject::EmComponent::eCamera;
				}

				return GameObject::EmComponent::TypeCount;
			}
		}

		IComponent::IComponent(IActor* pOwner, EmComponent::Type emCompType)
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

		void IComponent::DelComponent(EmComponent::Type emComponentType)
		{
			auto iter = m_umapChild.find(emComponentType);
			if (iter == m_umapChild.end())
				return;

			m_umapChild.erase(iter);
		}

		IComponent* IComponent::GetComponent(EmComponent::Type emComponentType)
		{
			auto iter = m_umapChild.find(emComponentType);
			if (iter == m_umapChild.end())
				return nullptr;

			return iter->second;
		}
	}
}