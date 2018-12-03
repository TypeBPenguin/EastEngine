#include "stdafx.h"
#include "Minion.h"

#include "CommonLib/FileUtil.h"

#include "GameObject/GameObject.h"
#include "GameObject/ComponentBehaviorTree.h"
#include "GameObject/ComponentFiniteStateMachine.h"
#include "GameObject/ComponentModel.h"
#include "GameObject/ComponentTimer.h"

#include "Input/InputInterface.h"

using namespace eastengine;

namespace StrID
{
	RegisterStringID(Minion);
	RegisterStringID(Action);
}

enum FSM
{
	eAction = 0,
};

enum FSM_Action
{
	eIdle = 0,
	eCombat_Idle,

	eAttack,

	eMove,
	eCombat_Move,
	eHit,
	eKnockUp,
	eStun,
	eDeath,
};

enum FSM_Event
{
	eTransitionState = 0,
	eMotionPlay,
};

struct EventTransitionState : public FiniteStateMachine::IEvent
{
	uint32_t transitionStateID{ 0 };

	EventTransitionState(float time, uint32_t transitionStateID)
		: IEvent(FSM_Event::eTransitionState, time)
		, transitionStateID(transitionStateID)
	{
	}
	virtual ~EventTransitionState() = default;
};

struct EventMotionPlay : public FiniteStateMachine::IEvent
{
	const string::StringID motionName;
	graphics::MotionLayers layer{ graphics::MotionLayers::eLayer1 };
	graphics::MotionPlaybackInfo playbackInfo;

	EventMotionPlay(float time, const string::StringID& motionName, graphics::MotionLayers layer)
		: IEvent(FSM_Event::eMotionPlay, time)
		, motionName(motionName)
		, layer(layer)
	{
	}
	virtual ~EventMotionPlay() = default;
};

Minion::Minion()
{
	auto ProcessEvent = [&](FiniteStateMachine* pStateMachine, const eastengine::FiniteStateMachine::IEvent* const* ppEvent, size_t eventCount)
	{
		for (size_t i = 0; i < eventCount; ++i)
		{
			switch (ppEvent[i]->eventID)
			{
			case FSM_Event::eTransitionState:
			{
				const EventTransitionState* pTransitionState = static_cast<const EventTransitionState*>(ppEvent[i]);
				pStateMachine->Transition(pTransitionState->transitionStateID);
			}
			break;
			case FSM_Event::eMotionPlay:
			{
				const EventMotionPlay* pMotionPlay = static_cast<const EventMotionPlay*>(ppEvent[i]);

				gameobject::ComponentModel* pModel = static_cast<gameobject::ComponentModel*>(m_pActor->CreateComponent(gameobject::IComponent::eModel));
				if (pModel != nullptr)
				{
					const string::StringID& motionName = pMotionPlay->motionName;

					std::string motionPath;
					motionPath.reserve(m_path.size() * 2);
					motionPath = m_path;
					motionPath += "Animations\\";
					motionPath += motionName.c_str();

					graphics::MotionLoader motionLoader;
					motionLoader.InitEast(motionName, motionPath.c_str());

					pModel->PlayMotion(graphics::MotionLayers::eLayer1, motionLoader, &pMotionPlay->playbackInfo);
				}
			}
			break;
			}
		}
	};

	m_path = file::GetDataPath();
	m_path += "Model\\Minion\\Melee\\";

	m_pActor = gameobject::IActor::Create(StrID::Minion);

	gameobject::ComponentModel* pModel = static_cast<gameobject::ComponentModel*>(m_pActor->CreateComponent(gameobject::IComponent::eModel));
	graphics::ModelLoader modelLoader;
	modelLoader.InitEast("Minion_Lane_Melee_Core_Dawn", (m_path + "Minion_Lane_Melee_Core_Dawn.emod").c_str());
	pModel->Initialize(&modelLoader);

	gameobject::ComponentTimer* pTimer = static_cast<gameobject::ComponentTimer*>(m_pActor->CreateComponent(gameobject::IComponent::eTimer));
	pTimer->StartTimeAction([&](uint32_t eventID, float elapsedTime, float processTime)
	{
		Update(eventID, elapsedTime, processTime);
	}, 0, 0);

	gameobject::ComponentFiniteStateMachine* pFiniteStateMachine = static_cast<gameobject::ComponentFiniteStateMachine*>(m_pActor->CreateComponent(gameobject::IComponent::eFiniteStateMachine));
	FiniteStateMachine& fsmAction = pFiniteStateMachine->CreateStateMachine(FSM::eAction, StrID::Action);
	
	std::unique_ptr<EventMotionPlay> pEventMotionPlay = std::make_unique<EventMotionPlay>(0.f, "NonCombat_Idle.emot", graphics::eLayer1);
	pEventMotionPlay->playbackInfo.loopCount = graphics::MotionPlaybackInfo::eMaxLoopCount;
	fsmAction.AddState(FSM_Action::eIdle, "Idle")
		.SetRelation(FSM_Action::eAttack)
		.SetRelation(FSM_Action::eMove)
		.SetRelation(FSM_Action::eHit)
		.SetRelation(FSM_Action::eKnockUp)
		.SetRelation(FSM_Action::eStun)
		.SetRelation(FSM_Action::eDeath)
		.SetEventState(ProcessEvent)
		.AddEnterEvent(std::move(pEventMotionPlay))
		.SetEnterState([&](FiniteStateMachine* pStateMachine, uint32_t prevStateID, const string::StringID& prevStateName)
	{
		LOG_MESSAGE("%s -> Idle", prevStateName.c_str());
	})
		.SetUpdateState([&](FiniteStateMachine* pStateMachine, float elapsedTime, float stateTime)
	{
		if (input::keyboard::IsKeyDown(input::keyboard::eUp) == true ||
			input::keyboard::IsKeyDown(input::keyboard::eDown) == true ||
			input::keyboard::IsKeyDown(input::keyboard::eLeft) == true ||
			input::keyboard::IsKeyDown(input::keyboard::eRight) == true)
		{
			pStateMachine->Transition(FSM_Action::eMove);
		}

		if (input::keyboard::IsKeyDown(input::keyboard::eS))
		{
			pStateMachine->Transition(FSM_Action::eAttack);
		}
	})
		.SetLeaveState([&](FiniteStateMachine* pStateMachine, uint32_t nextStateID, const string::StringID& nextStateName)
	{
	});

	pEventMotionPlay = std::make_unique<EventMotionPlay>(0.f, "CoreVibration.emot", graphics::eLayer1);
	pEventMotionPlay->playbackInfo.loopCount = graphics::MotionPlaybackInfo::eMaxLoopCount;
	fsmAction.AddState(FSM_Action::eCombat_Idle, "Combat_Idle")
		.SetRelation(FSM_Action::eIdle)
		.SetRelation(FSM_Action::eAttack)
		.SetRelation(FSM_Action::eCombat_Move)
		.SetRelation(FSM_Action::eHit)
		.SetRelation(FSM_Action::eKnockUp)
		.SetRelation(FSM_Action::eStun)
		.SetRelation(FSM_Action::eDeath)
		.SetEventState(ProcessEvent)
		.AddEnterEvent(std::move(pEventMotionPlay))
		.SetEnterState([&](FiniteStateMachine* pStateMachine, uint32_t prevStateID, const string::StringID& prevStateName)
	{
		LOG_MESSAGE("%s -> Combat_Idle", prevStateName.c_str());
	})
		.AddUpdateEvent(std::make_unique<EventTransitionState>(3.f, FSM_Action::eIdle))
		.SetUpdateState([&](FiniteStateMachine* pStateMachine, float elapsedTime, float stateTime)
	{
		if (input::keyboard::IsKeyDown(input::keyboard::eUp) == true ||
			input::keyboard::IsKeyDown(input::keyboard::eDown) == true ||
			input::keyboard::IsKeyDown(input::keyboard::eLeft) == true ||
			input::keyboard::IsKeyDown(input::keyboard::eRight) == true)
		{
			pStateMachine->Transition(FSM_Action::eCombat_Move);
		}

		if (input::keyboard::IsKeyDown(input::keyboard::eS))
		{
			pStateMachine->Transition(FSM_Action::eAttack);
		}
	})
		.SetLeaveState([&](FiniteStateMachine* pStateMachine, uint32_t nextStateID, const string::StringID& nextStateName)
	{
	});

	fsmAction.AddState(FSM_Action::eAttack, "Attack")
		.SetRelation(FSM_Action::eCombat_Idle)
		.SetRelation(FSM_Action::eAttack)
		.SetRelation(FSM_Action::eCombat_Move)
		.SetRelation(FSM_Action::eHit)
		.SetRelation(FSM_Action::eKnockUp)
		.SetRelation(FSM_Action::eStun)
		.SetRelation(FSM_Action::eDeath)
		.SetEventState(ProcessEvent)
		.SetEnterState([&](FiniteStateMachine* pStateMachine, uint32_t prevStateID, const string::StringID& prevStateName)
	{
		LOG_MESSAGE("%s -> Attack", prevStateName.c_str());
		m_isRequestAttack = false;

		gameobject::ComponentModel* pModel = static_cast<gameobject::ComponentModel*>(m_pActor->CreateComponent(gameobject::IComponent::eModel));

		if (m_emAttackStep >= m_emMaxAttackStep)
		{
			m_emAttackType = static_cast<AttackType>(math::Random<int>(AttackType::eNormal, AttackType::eSetB));

			std::string motionName = "Attack_A";
			switch (m_emAttackType)
			{
			case AttackType::eNormal:
				m_emAttackStep = AttackStep::eStep0;
				m_emMaxAttackStep = AttackStep::eStep3;
				motionName += ".emot";
				break;
			case AttackType::eSetA:
				m_emAttackStep = AttackStep::eStep0;
				m_emMaxAttackStep = AttackStep::eStep4;
				motionName += "_SetA.emot";
				break;
			case AttackType::eSetB:
				m_emAttackStep = AttackStep::eStep0;
				m_emMaxAttackStep = AttackStep::eStep4;
				motionName += "_SetB.emot";
				break;
			default:
				assert(false);
				break;
			}

			const std::string motionPath = m_path + "Animations\\" + motionName;

			graphics::MotionLoader motionLoader;
			motionLoader.InitEast(motionName.c_str(), motionPath.c_str());
			pModel->PlayMotion(graphics::MotionLayers::eLayer1, motionLoader);
		}
		else
		{
			m_emAttackStep = static_cast<AttackStep>(m_emAttackStep + 1);

			std::string motionName = "Attack_";
			switch (m_emAttackStep)
			{
			case AttackStep::eStep0:
				motionName += "A";
				break;
			case AttackStep::eStep1:
				motionName += "B";
				break;
			case AttackStep::eStep2:
				motionName += "C";
				break;
			case AttackStep::eStep3:
				motionName += "D";
				break;
			case AttackStep::eStep4:
				motionName += "E";
				break;
			}

			switch (m_emAttackType)
			{
			case AttackType::eNormal:
				motionName += ".emot";
				break;
			case AttackType::eSetA:
				motionName += "_SetA.emot";
				break;
			case AttackType::eSetB:
				motionName += "_SetB.emot";
				break;
			default:
				assert(false);
				break;
			}

			LOG_MESSAGE("Motion : %s", motionName.c_str());
			const std::string motionPath = m_path + "Animations\\" + motionName.c_str();

			graphics::MotionLoader motionLoader;
			motionLoader.InitEast(motionName.c_str(), motionPath.c_str());
			pModel->PlayMotion(graphics::MotionLayers::eLayer1, motionLoader);
		}
	})
		.SetUpdateState([&](FiniteStateMachine* pStateMachine, float elapsedTime, float stateTime)
	{
		gameobject::ComponentModel* pModel = static_cast<gameobject::ComponentModel*>(m_pActor->CreateComponent(gameobject::IComponent::eModel));
		graphics::IMotionPlayer* pPlayer = pModel->GetModelInstance()->GetMotionSystem()->GetPlayer(graphics::eLayer1);
		graphics::IMotion* pMotion = pPlayer->GetMotion();
		if (pMotion != nullptr)
		{
			const float motionEndTime = pMotion->GetEndTime();
			if (m_emAttackStep > m_emMaxAttackStep || 
				motionEndTime < stateTime)
			{
				if (m_isRequestAttack == true)
				{
					pStateMachine->Transition(FSM_Action::eAttack);
				}
				else
				{
					pStateMachine->Transition(FSM_Action::eCombat_Idle);
				}
			}
			else
			{
				if (m_isRequestAttack == true && motionEndTime * 0.8f < stateTime && stateTime <= motionEndTime)
				{
					pStateMachine->Transition(FSM_Action::eAttack);
				}
				else if ((motionEndTime * 0.5f < stateTime && stateTime <= motionEndTime) &&
					input::keyboard::IsKeyDown(input::keyboard::eS) == true)
				{
					m_isRequestAttack = true;
				}
			}
		}
		else
		{
			if (m_isRequestAttack == true)
			{
				pStateMachine->Transition(FSM_Action::eAttack);
			}
			else
			{
				pStateMachine->Transition(FSM_Action::eCombat_Idle);
			}
		}
	})
		.SetLeaveState([&](FiniteStateMachine* pStateMachine, uint32_t nextStateID, const string::StringID& nextStateName)
	{
	});

	pEventMotionPlay = std::make_unique<EventMotionPlay>(0.f, "NonCombat_JogFwd.emot", graphics::eLayer1);
	pEventMotionPlay->playbackInfo.loopCount = graphics::MotionPlaybackInfo::eMaxLoopCount;

	fsmAction.AddState(FSM_Action::eMove, "Move")
		.SetRelation(FSM_Action::eIdle)
		.SetRelation(FSM_Action::eAttack)
		.SetRelation(FSM_Action::eHit)
		.SetRelation(FSM_Action::eKnockUp)
		.SetRelation(FSM_Action::eStun)
		.SetRelation(FSM_Action::eDeath)
		.SetEventState(ProcessEvent)
		.AddEnterEvent(std::move(pEventMotionPlay))
		.SetEnterState([&](FiniteStateMachine* pStateMachine, uint32_t prevStateID, const string::StringID& prevStateName)
	{
		LOG_MESSAGE("%s -> Move", prevStateName.c_str());
	})
		.SetUpdateState([&](FiniteStateMachine* pStateMachine, float elapsedTime, float stateTime)
	{
		if (input::keyboard::IsKeyPressed(input::keyboard::eUp) == false &&
			input::keyboard::IsKeyPressed(input::keyboard::eDown) == false &&
			input::keyboard::IsKeyPressed(input::keyboard::eLeft) == false &&
			input::keyboard::IsKeyPressed(input::keyboard::eRight) == false)
		{
			pStateMachine->Transition(FSM_Action::eIdle);
		}

		if (input::keyboard::IsKeyDown(input::keyboard::eS))
		{
			pStateMachine->Transition(FSM_Action::eAttack);
		}
	})
		.SetLeaveState([&](FiniteStateMachine* pStateMachine, uint32_t nextStateID, const string::StringID& nextStateName)
	{
	});

	pEventMotionPlay = std::make_unique<EventMotionPlay>(0.f, "Combat_JogFwd_Start.emot", graphics::eLayer1);
	pEventMotionPlay->playbackInfo.loopCount = graphics::MotionPlaybackInfo::eMaxLoopCount;

	std::unique_ptr<EventMotionPlay> pEventMotionPlay2 = std::make_unique<EventMotionPlay>(0.1f, "Combat_JogFwd.emot", graphics::eLayer1);
	pEventMotionPlay2->playbackInfo.loopCount = graphics::MotionPlaybackInfo::eMaxLoopCount;

	fsmAction.AddState(FSM_Action::eCombat_Move, "CombatMove")
		.SetRelation(FSM_Action::eCombat_Idle)
		.SetRelation(FSM_Action::eAttack)
		.SetRelation(FSM_Action::eMove)
		.SetRelation(FSM_Action::eHit)
		.SetRelation(FSM_Action::eKnockUp)
		.SetRelation(FSM_Action::eStun)
		.SetRelation(FSM_Action::eDeath)
		.SetEventState(ProcessEvent)
		.AddEnterEvent(std::move(pEventMotionPlay))
		.SetEnterState([&](FiniteStateMachine* pStateMachine, uint32_t prevStateID, const string::StringID& prevStateName)
	{
		LOG_MESSAGE("%s -> CombatMove", prevStateName.c_str());
	})
		.AddUpdateEvent(std::move(pEventMotionPlay2))
		.AddUpdateEvent(std::make_unique<EventTransitionState>(3.f, FSM_Action::eCombat_Idle))
		.SetUpdateState([&](FiniteStateMachine* pStateMachine, float elapsedTime, float stateTime)
	{
		if (input::keyboard::IsKeyPressed(input::keyboard::eUp) == false &&
			input::keyboard::IsKeyPressed(input::keyboard::eDown) == false &&
			input::keyboard::IsKeyPressed(input::keyboard::eLeft) == false &&
			input::keyboard::IsKeyPressed(input::keyboard::eRight) == false)
		{
			pStateMachine->Transition(FSM_Action::eCombat_Idle);
		}

		if (input::keyboard::IsKeyDown(input::keyboard::eS))
		{
			pStateMachine->Transition(FSM_Action::eAttack);
		}
	})
		.SetLeaveState([&](FiniteStateMachine* pStateMachine, uint32_t nextStateID, const string::StringID& nextStateName)
	{
	});

	fsmAction.AddState(FSM_Action::eHit, "Hit")
		.SetRelation(FSM_Action::eCombat_Idle)
		.SetRelation(FSM_Action::eKnockUp)
		.SetRelation(FSM_Action::eStun)
		.SetRelation(FSM_Action::eDeath)
		.SetEventState(ProcessEvent)
		.AddEnterEvent(std::make_unique<EventMotionPlay>(0.f, "HitReact_Front.emot", graphics::eLayer1))
		.SetEnterState([&](FiniteStateMachine* pStateMachine, uint32_t prevStateID, const string::StringID& prevStateName)
	{
		LOG_MESSAGE("%s -> Hit", prevStateName.c_str());
	})
		.SetUpdateState([&](FiniteStateMachine* pStateMachine, float elapsedTime, float stateTime)
	{
		gameobject::ComponentModel* pModel = static_cast<gameobject::ComponentModel*>(m_pActor->CreateComponent(gameobject::IComponent::eModel));
		if (pModel != nullptr)
		{
			graphics::IMotionPlayer* pMotionPlayer = pModel->GetModelInstance()->GetMotionSystem()->GetPlayer(graphics::eLayer1);
			if (pMotionPlayer->IsPlaying() == false)
			{
				pStateMachine->Transition(FSM_Action::eCombat_Idle);
			}
		}
	})
		.SetLeaveState([&](FiniteStateMachine* pStateMachine, uint32_t nextStateID, const string::StringID& nextStateName)
	{
	});

	fsmAction.AddState(FSM_Action::eKnockUp, "KnockUp")
		.SetRelation(FSM_Action::eCombat_Idle)
		.SetRelation(FSM_Action::eDeath)
		.SetEventState(ProcessEvent)
		.AddEnterEvent(std::make_unique<EventMotionPlay>(0.f, "KnockUp.emot", graphics::eLayer1))
		.SetEnterState([&](FiniteStateMachine* pStateMachine, uint32_t prevStateID, const string::StringID& prevStateName)
	{
	})
		.SetUpdateState([&](FiniteStateMachine* pStateMachine, float elapsedTime, float stateTime)
	{
		gameobject::ComponentModel* pModel = static_cast<gameobject::ComponentModel*>(m_pActor->CreateComponent(gameobject::IComponent::eModel));
		if (pModel != nullptr)
		{
			graphics::IMotionPlayer* pMotionPlayer = pModel->GetModelInstance()->GetMotionSystem()->GetPlayer(graphics::eLayer1);
			if (pMotionPlayer->IsPlaying() == false)
			{
				pStateMachine->Transition(FSM_Action::eCombat_Idle);
			}
		}
	})
		.SetLeaveState([&](FiniteStateMachine* pStateMachine, uint32_t nextStateID, const string::StringID& nextStateName)
	{
	});

	fsmAction.AddState(FSM_Action::eStun, "Stun")
		.SetRelation(FSM_Action::eCombat_Idle)
		.SetRelation(FSM_Action::eHit)
		.SetRelation(FSM_Action::eKnockUp)
		.SetRelation(FSM_Action::eDeath)
		.SetEventState(ProcessEvent)
		.AddEnterEvent(std::make_unique<EventMotionPlay>(0.f, "Stun.emot", graphics::eLayer1))
		.SetEnterState([&](FiniteStateMachine* pStateMachine, uint32_t prevStateID, const string::StringID& prevStateName)
	{
	})
		.SetUpdateState([&](FiniteStateMachine* pStateMachine, float elapsedTime, float stateTime)
	{
		gameobject::ComponentModel* pModel = static_cast<gameobject::ComponentModel*>(m_pActor->CreateComponent(gameobject::IComponent::eModel));
		if (pModel != nullptr)
		{
			graphics::IMotionPlayer* pMotionPlayer = pModel->GetModelInstance()->GetMotionSystem()->GetPlayer(graphics::eLayer1);
			if (pMotionPlayer->IsPlaying() == false)
			{
				pStateMachine->Transition(FSM_Action::eCombat_Idle);
			}
		}
	})
		.SetLeaveState([&](FiniteStateMachine* pStateMachine, uint32_t nextStateID, const string::StringID& nextStateName)
	{
	});

	fsmAction.AddState(FSM_Action::eDeath, "Death")
		.SetRelation(FSM_Action::eCombat_Idle)
		.SetEventState(ProcessEvent)
		.AddEnterEvent(std::make_unique<EventMotionPlay>(0.f, "Death_A.emot", graphics::eLayer1))
		.SetEnterState([&](FiniteStateMachine* pStateMachine, uint32_t prevStateID, const string::StringID& prevStateName)
	{
	})
		.SetUpdateState([&](FiniteStateMachine* pStateMachine, float elapsedTime, float stateTime)
	{
		gameobject::ComponentModel* pModel = static_cast<gameobject::ComponentModel*>(m_pActor->CreateComponent(gameobject::IComponent::eModel));
		if (pModel != nullptr)
		{
			graphics::IMotionPlayer* pMotionPlayer = pModel->GetModelInstance()->GetMotionSystem()->GetPlayer(graphics::eLayer1);
			if (pMotionPlayer->IsPlaying() == false)
			{
				pStateMachine->Transition(FSM_Action::eCombat_Idle);
			}
		}
	})
		.SetLeaveState([&](FiniteStateMachine* pStateMachine, uint32_t nextStateID, const string::StringID& nextStateName)
	{
	});

	fsmAction.SetDefaultState(FSM_Action::eIdle);
	fsmAction.Transition(FSM_Action::eIdle);
}

Minion::~Minion()
{
	gameobject::IActor::Destroy(&m_pActor);
}

void Minion::Update(uint32_t eventID, float elapsedTime, float processTime)
{
}