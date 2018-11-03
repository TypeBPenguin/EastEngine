#include "stdafx.h"
#include "ComponentInterface.h"

namespace eastengine
{
	namespace gameobject
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
				if (string::IsEquals(strType, "ActionState"))
				{
					return gameobject::EmComponent::eActionState;
				}
				else if (string::IsEquals(strType, "Timer"))
				{
					return gameobject::EmComponent::eTimer;
				}
				else if (string::IsEquals(strType, "Physics"))
				{
					return gameobject::EmComponent::ePhysics;
				}
				else if (string::IsEquals(strType, "Model"))
				{
					return gameobject::EmComponent::eModel;
				}
				else if (string::IsEquals(strType, "Camera"))
				{
					return gameobject::EmComponent::eCamera;
				}

				return gameobject::EmComponent::TypeCount;
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