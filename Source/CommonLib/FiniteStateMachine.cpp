#include "stdafx.h"
#include "FiniteStateMachine.h"

#include "Log.h"

namespace est
{
	FiniteStateMachine::FiniteStateMachine(uint32_t id, const string::StringID& name)
		: m_id(id)
		, m_name(name)
	{
	}

	FiniteStateMachine::FiniteStateMachine(FiniteStateMachine&& source) noexcept
		: m_id(std::move(source.m_id))
		, m_name(std::move(source.m_name))
		, m_curStateID(std::move(source.m_curStateID))
		, m_transitStateID(std::move(source.m_transitStateID))
		, m_defaultStateID(std::move(source.m_defaultStateID))
		, m_stateTime(std::move(source.m_stateTime))
		, m_rmapStates(std::move(source.m_rmapStates))
	{
	}

	FiniteStateMachine::~FiniteStateMachine()
	{
	}

	FiniteStateMachine& FiniteStateMachine::operator = (FiniteStateMachine&& source) noexcept
	{
		m_id = std::move(source.m_id);
		m_name = std::move(source.m_name);
		m_curStateID = std::move(source.m_curStateID);
		m_transitStateID = std::move(source.m_transitStateID);
		m_defaultStateID = std::move(source.m_defaultStateID);
		m_stateTime = std::move(source.m_stateTime);
		m_rmapStates = std::move(source.m_rmapStates);

		return *this;
	}

	FiniteStateMachine::State::State(const string::StringID& name)
		: m_name(name)
	{
	}

	FiniteStateMachine::State::State(State&& source) noexcept
	{
		*this = std::move(source);
	}

	FiniteStateMachine::State::~State()
	{
	}

	FiniteStateMachine::State& FiniteStateMachine::State::operator = (State&& source) noexcept
	{
		m_name= std::move(source.m_name);
		m_funcEnterState= std::move(source.m_funcEnterState);
		m_funcUpdateState= std::move(source.m_funcUpdateState);
		m_funcLeaveState= std::move(source.m_funcLeaveState);
		m_funcEventState= std::move(source.m_funcEventState);
		m_relationships= std::move(source.m_relationships);
		m_enterEvents= std::move(source.m_enterEvents);
		m_updateEvents= std::move(source.m_updateEvents);
		m_leaveEvents= std::move(source.m_leaveEvents);
		return *this;
	}

	void FiniteStateMachine::State::Enter(FiniteStateMachine* pStateMachine, uint32_t prevStateID, const string::StringID& prevStateName)
	{
		if (m_funcEventState != nullptr && m_enterEvents.empty() == false)
		{
			std::vector<const IEvent*> events;
			for (auto& pEvent : m_enterEvents)
			{
				events.emplace_back(pEvent.get());
			}
			std::sort(events.begin(), events.end(), [](const IEvent* pA, const IEvent* pB)
			{
				return pA->time < pB->time;
			});
			m_funcEventState(pStateMachine, events.data(), events.size());
		}

		if (m_funcEnterState != nullptr)
		{
			m_funcEnterState(pStateMachine, prevStateID, prevStateName);
		}
	}

	void FiniteStateMachine::State::Update(FiniteStateMachine* pStateMachine, float elapsedTime, float stateTime)
	{
		if (m_funcEventState != nullptr && m_updateEvents.empty() == false)
		{
			const float prevStateTime = stateTime - elapsedTime;

			std::vector<const IEvent*> events;
			for (auto& pEvent : m_updateEvents)
			{
				if (prevStateTime <= pEvent->time && pEvent->time < stateTime)
				{
					events.emplace_back(pEvent.get());
				}
			}
			std::sort(events.begin(), events.end(), [](const IEvent* pA, const IEvent* pB)
			{
				return pA->time < pB->time;
			});
			m_funcEventState(pStateMachine, events.data(), events.size());
		}

		if (m_funcUpdateState != nullptr)
		{
			m_funcUpdateState(pStateMachine, elapsedTime, stateTime);
		}
	}

	void FiniteStateMachine::State::Leave(FiniteStateMachine* pStateMachine, uint32_t nextStateID, const string::StringID& nextStateName)
	{
		if (m_funcEventState != nullptr && m_leaveEvents.empty() == false)
		{
			std::vector<const IEvent*> events;
			for (auto& pEvent : m_leaveEvents)
			{
				events.emplace_back(pEvent.get());
			}
			std::sort(events.begin(), events.end(), [](const IEvent* pA, const IEvent* pB)
			{
				return pA->time < pB->time;
			});
			m_funcEventState(pStateMachine, events.data(), events.size());
		}

		if (m_funcLeaveState != nullptr)
		{
			m_funcLeaveState(pStateMachine, nextStateID, nextStateName);
		}
	}

	FiniteStateMachine::State& FiniteStateMachine::AddState(uint32_t stateID, const string::StringID& name)
	{
		assert(stateID != eInvalidStateID);
		auto iter = m_rmapStates.find(stateID);
		if (iter != m_rmapStates.end())
			return iter.value();

		auto iter_result = m_rmapStates.emplace(stateID, name);
		return iter_result.first.value();
	}

	FiniteStateMachine::State* FiniteStateMachine::GetState(uint32_t stateID)
	{
		auto iter = m_rmapStates.find(stateID);
		if (iter != m_rmapStates.end())
			return &iter.value();

		return nullptr;
	}

	void FiniteStateMachine::Transition(uint32_t stateID)
	{
		if (m_curStateID != eInvalidStateID)
		{
			State* pState = GetState(m_curStateID);
			if (pState->IsRelation(stateID) == true)
			{
				m_transitStateID = stateID;
			}
			else
			{
				State* pTransitState = GetState(stateID);
				if (pTransitState != nullptr)
				{
					LOG_ERROR(L"The transition to an unrelated state : Machine[%d, %s], CurState[%d, %s], TransitionState[%d, %s]", m_id, m_name.c_str(), m_curStateID, pState->GetName().c_str(), stateID, pTransitState->GetName().c_str());
				}
				else
				{
					LOG_ERROR(L"The transition target does not exist : Machine[%d, %s], CurState[%d, %s], TransitionState[%d]", m_id, m_name.c_str(), m_curStateID, pState->GetName().c_str(), stateID);
				}

				if (m_defaultStateID == eInvalidStateID)
				{
					LOG_ERROR(L"Please set default stateID");
				}
				else
				{
					m_transitStateID = m_defaultStateID;
				}
			}
		}
		else
		{
			State* pTransitState = GetState(stateID);
			if (pTransitState == nullptr)
			{
				LOG_ERROR(L"The transition target does not exist : Machine[%d, %s], TransitionState[%d]", m_id, m_name.c_str(), stateID);

				if (m_defaultStateID == eInvalidStateID)
				{
					LOG_ERROR(L"Please set default stateID");
				}
				else
				{
					m_transitStateID = m_defaultStateID;
				}
			}
			else
			{
				m_transitStateID = stateID;
			}
		}
	}

	void FiniteStateMachine::Update(float elapsedTime)
	{
		if (m_transitStateID != eInvalidStateID)
		{
			const uint32_t transitStateID = m_transitStateID;

			State* pState = GetState(m_curStateID);
			if (pState != nullptr)
			{
				if (pState->IsRelation(m_transitStateID) == true)
				{
					State* pTransitState = GetState(m_transitStateID);
					assert(pTransitState != nullptr);

					if (pTransitState != nullptr)
					{
						m_stateTime = 0.f;

						pState->Leave(this, m_transitStateID, pTransitState->GetName());
						pTransitState->Enter(this, m_curStateID, pState->GetName());
					}
				}
			}
			else
			{
				State* pTransitState = GetState(m_transitStateID);
				assert(pTransitState != nullptr);

				if (pTransitState != nullptr)
				{
					m_stateTime = 0.f;
					pTransitState->Enter(this, eInvalidStateID, L"");
				}
			}

			m_curStateID = transitStateID;
			m_transitStateID = eInvalidStateID;
		}
		else
		{
			State* pState = GetState(m_curStateID);
			if (pState != nullptr)
			{
				m_stateTime += elapsedTime;

				pState->Update(this, elapsedTime, m_stateTime);
			}
		}
	}
}