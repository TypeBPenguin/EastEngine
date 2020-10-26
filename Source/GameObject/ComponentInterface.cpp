#include "stdafx.h"
#include "ComponentInterface.h"

namespace est
{
	namespace gameobject
	{
		const wchar_t* IComponent::ToString(Type emType)
		{
			switch (emType)
			{
			case eTimer:
				return L"Timer";
			case eBehaviorTree:
				return L"BehaviorTree";
			case ePhysics:
				return L"Physics";
			case eModel:
				return L"Model";
			case eCamera:
				return L"Camera";
			case eLight:
				return L"Light";
			default:
				return nullptr;
			}
		}

		IComponent::Type IComponent::GetType(const wchar_t* strType)
		{
			if (string::IsEquals(strType, L"BehaviorTree"))
			{
				return IComponent::eBehaviorTree;
			}
			else if (string::IsEquals(strType, L"Timer"))
			{
				return IComponent::eTimer;
			}
			else if (string::IsEquals(strType, L"Physics"))
			{
				return IComponent::ePhysics;
			}
			else if (string::IsEquals(strType, L"Model"))
			{
				return IComponent::eModel;
			}
			else if (string::IsEquals(strType, L"Camera"))
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
		}
	}
}