#include "stdafx.h"
#include "ComponentFiniteStateMachine.h"

namespace eastengine
{
	namespace gameobject
	{
		ComponentFiniteStateMachine::ComponentFiniteStateMachine(IActor* pOwner)
			: IComponent(pOwner, IComponent::eFiniteStateMachine)
		{
		}

		ComponentFiniteStateMachine::~ComponentFiniteStateMachine()
		{
		}

		void ComponentFiniteStateMachine::Update(float elapsedTime)
		{
			for (auto iter = m_umapStateMachines.begin(); iter != m_umapStateMachines.end(); ++iter)
			{
				iter.value().Update(elapsedTime);
			}
		}

		bool ComponentFiniteStateMachine::LoadFile(file::Stream& file)
		{
			return true;
		}

		bool ComponentFiniteStateMachine::SaveFile(file::Stream& file)
		{
			return true;
		}

		FiniteStateMachine& ComponentFiniteStateMachine::CreateStateMachine(uint32_t id, const string::StringID& name)
		{
			auto iter = m_umapStateMachines.find(id);
			if (iter != m_umapStateMachines.end())
				return iter.value();

			auto iter_result = m_umapStateMachines.emplace(id, FiniteStateMachine(id, name));
			return iter_result.first.value();
		}

		FiniteStateMachine* ComponentFiniteStateMachine::GetStateMachine(uint32_t id)
		{
			auto iter = m_umapStateMachines.find(id);
			if (iter != m_umapStateMachines.end())
				return &iter.value();

			return nullptr;
		}
	}
}