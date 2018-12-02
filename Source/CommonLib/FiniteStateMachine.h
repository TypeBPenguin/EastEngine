#pragma once

#include "StringTable.h"

namespace eastengine
{
	class FiniteStateMachine
	{
	public:
		FiniteStateMachine(uint32_t id, const string::StringID& name);
		FiniteStateMachine(FiniteStateMachine&& source) noexcept;
		~FiniteStateMachine();

	public:
		struct IEvent
		{
			const uint32_t eventID{ 0 };
			const float time{ 0.f };

			IEvent(uint32_t eventID, float time)
				: eventID(eventID)
				, time(time)
			{
			}
			virtual ~IEvent() = default;
		};

		class State
		{
		public:
			State(const string::StringID& name);
			State(State&& source) noexcept;
			~State();

			State& operator = (State&& source) noexcept;

		public:
			using FuncEnterState = std::function<void(FiniteStateMachine* pStateMachine, uint32_t prevStateID, const string::StringID& prevStateName)>;
			using FuncUpdateState = std::function<void(FiniteStateMachine* pStateMachine, float elapsedTime, float stateTime)>;
			using FuncLeaveState = std::function<void(FiniteStateMachine* pStateMachine, uint32_t nextStateID, const string::StringID& nextStateName)>;
			using FuncEventState = std::function<void(FiniteStateMachine* pStateMachine, const IEvent* const* ppEvents, size_t eventCount)>;

		public:
			void Enter(FiniteStateMachine* pStateMachine, uint32_t prevStateID, const string::StringID& prevStateName);
			void Update(FiniteStateMachine* pStateMachine, float elapsedTime, float stateTime);
			void Leave(FiniteStateMachine* pStateMachine, uint32_t nextStateID, const string::StringID& nextStateName);

		public:
			const string::StringID& GetName() const { return m_name; }

		public:
			State& SetEnterState(FuncEnterState funcEnterState) { m_funcEnterState = funcEnterState; return *this; }
			State& SetUpdateState(FuncUpdateState funcUpdateState) { m_funcUpdateState = funcUpdateState; return *this; }
			State& SetLeaveState(FuncLeaveState funcLeaveState) { m_funcLeaveState = funcLeaveState; return *this; }
			State& SetEventState(FuncEventState funcEventState) { m_funcEventState = funcEventState; return *this; }

			State& SetRelation(uint32_t toStateID) { m_relationships.emplace(toStateID); return *this; }
			bool IsRelation(uint32_t stateID) const { return m_relationships.find(stateID) != m_relationships.end(); }

			State& AddEnterEvent(std::unique_ptr<IEvent> pEvent) { m_enterEvents.emplace_back(std::move(pEvent)); return *this; }
			State& AddUpdateEvent(std::unique_ptr<IEvent> pUpdateEvent) { m_updateEvents.emplace_back(std::move(pUpdateEvent)); return *this; }
			State& AddLeaveEvent(std::unique_ptr<IEvent> pEvent) { m_leaveEvents.emplace_back(std::move(pEvent)); return *this; }

		private:
			string::StringID m_name;

			FuncEnterState m_funcEnterState;
			FuncUpdateState m_funcUpdateState;
			FuncLeaveState m_funcLeaveState;
			FuncEventState m_funcEventState;

			std::set<uint32_t> m_relationships;

			std::vector<std::unique_ptr<IEvent>> m_enterEvents;
			std::vector<std::unique_ptr<IEvent>> m_updateEvents;
			std::vector<std::unique_ptr<IEvent>> m_leaveEvents;
		};

		State& AddState(uint32_t stateID, const string::StringID& name);
		State* GetState(uint32_t stateID);
		void SetDefaultState(uint32_t stateID) { m_defaultStateID = stateID; }
		void Transition(uint32_t stateID);

	public:
		void Update(float elapsedTime);

	private:
		const uint32_t m_id{ 0 };
		const string::StringID m_name;

		enum : uint32_t
		{
			eInvalidStateID = std::numeric_limits<uint32_t>::max(),
		};
		uint32_t m_curStateID{ eInvalidStateID };
		uint32_t m_transitStateID{ eInvalidStateID };
		uint32_t m_defaultStateID{ eInvalidStateID };
		float m_stateTime{ 0.f };
		tsl::robin_map<uint32_t, State> m_rmapStates;
	};
}