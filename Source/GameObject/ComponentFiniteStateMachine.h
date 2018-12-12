#pragma once

#include "CommonLib/FiniteStateMachine.h"
#include "ComponentInterface.h"

namespace eastengine
{
	namespace gameobject
	{
		class ComponentFiniteStateMachine : public IComponent
		{
		public:
			ComponentFiniteStateMachine(IActor* pOwner);
			virtual ~ComponentFiniteStateMachine();

		public:
			virtual void Update(float elapsedTime) override;

			virtual bool LoadFile(file::Stream& file);
			virtual bool SaveFile(file::Stream& file);

		public:
			FiniteStateMachine& CreateStateMachine(uint32_t id, const string::StringID& name);
			FiniteStateMachine* GetStateMachine(uint32_t id);

		private:
			tsl::robin_map<uint32_t, FiniteStateMachine> m_umapStateMachines;
		};
	}
}