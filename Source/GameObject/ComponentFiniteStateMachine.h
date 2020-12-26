#pragma once

#include "CommonLib/FiniteStateMachine.h"
#include "ComponentInterface.h"

namespace est
{
	namespace gameobject
	{
		class ComponentFiniteStateMachine : public IComponent
		{
		public:
			ComponentFiniteStateMachine(IActor* pOwner);
			virtual ~ComponentFiniteStateMachine();

		public:
			virtual void Update(float elapsedTime, float lodThreshold) override;

		public:
			FiniteStateMachine& CreateStateMachine(uint32_t id, const string::StringID& name);
			FiniteStateMachine* GetStateMachine(uint32_t id);

		private:
			tsl::robin_map<uint32_t, FiniteStateMachine> m_umapStateMachines;
		};
	}
}