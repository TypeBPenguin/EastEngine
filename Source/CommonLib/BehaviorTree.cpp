#include "stdafx.h"
#include "BehaviorTree.h"

#include "Math.h"
#include "Log.h"

namespace eastengine
{
	namespace StrID
	{
		RegisterStringID(Root);
	}

	BehaviorTree::BehaviorTree()
		: m_pRoot(std::make_unique<Root>(StrID::Root))
	{
	}

	BehaviorTree::~BehaviorTree()
	{
	}

	BehaviorTree::INode::INode(const string::StringID& name)
		: m_name(name)
	{
	}

	BehaviorTree::INode::~INode()
	{
	}

	BehaviorTree::ICompositeNode::ICompositeNode(const string::StringID& name)
		: INode(name)
	{
	}

	BehaviorTree::ICompositeNode::~ICompositeNode()
	{
	}

	BehaviorTree::Selector* BehaviorTree::ICompositeNode::AddSelector(const string::StringID& name)
	{
		return static_cast<Selector*>(m_childNodes.emplace_back(std::make_unique<Selector>(name)).get());
	}

	BehaviorTree::RandomSelector* BehaviorTree::ICompositeNode::AddRandomSelector(const string::StringID& name)
	{
		return static_cast<RandomSelector*>(m_childNodes.emplace_back(std::make_unique<RandomSelector>(name)).get());
	}

	BehaviorTree::Sequence* BehaviorTree::ICompositeNode::AddSequence(const string::StringID& name)
	{
		return static_cast<Sequence*>(m_childNodes.emplace_back(std::make_unique<Sequence>(name)).get());
	}

	BehaviorTree::Inverter* BehaviorTree::ICompositeNode::AddInverter(const string::StringID& name)
	{
		return static_cast<Inverter*>(m_childNodes.emplace_back(std::make_unique<Inverter>(name)).get());
	}

	BehaviorTree::Succeeder* BehaviorTree::ICompositeNode::AddSucceeder(const string::StringID& name)
	{
		return static_cast<Succeeder*>(m_childNodes.emplace_back(std::make_unique<Succeeder>(name)).get());
	}

	BehaviorTree::Failer* BehaviorTree::ICompositeNode::AddFailer(const string::StringID& name)
	{
		return static_cast<Failer*>(m_childNodes.emplace_back(std::make_unique<Failer>(name)).get());
	}

	BehaviorTree::Loop* BehaviorTree::ICompositeNode::AddLoop(const string::StringID& name, size_t maxLoopCount)
	{
		return static_cast<Loop*>(m_childNodes.emplace_back(std::make_unique<Loop>(name, maxLoopCount)).get());
	}

	BehaviorTree::ConditionalLoop* BehaviorTree::ICompositeNode::AddConditionalLoop(const string::StringID& name)
	{
		return static_cast<ConditionalLoop*>(m_childNodes.emplace_back(std::make_unique<ConditionalLoop>(name)).get());
	}

	BehaviorTree::Cooldown* BehaviorTree::ICompositeNode::AddCooldown(const string::StringID& name, float timeCooldown)
	{
		return static_cast<Cooldown*>(m_childNodes.emplace_back(std::make_unique<Cooldown>(name, timeCooldown)).get());
	}

	BehaviorTree::Action* BehaviorTree::ICompositeNode::AddAction(const string::StringID& name, FuncAction func)
	{
		return static_cast<Action*>(m_childNodes.emplace_back(std::make_unique<Action>(name, func)).get());
	}

	BehaviorTree::RunBehavior* BehaviorTree::ICompositeNode::AddRunBehavior(const string::StringID& name, const string::StringID& behaviorName)
	{
		return static_cast<RunBehavior*>(m_childNodes.emplace_back(std::make_unique<RunBehavior>(name, behaviorName)).get());
	}

	BehaviorTree::Wait* BehaviorTree::ICompositeNode::AddWait(const string::StringID& name, float waitTime)
	{
		return static_cast<Wait*>(m_childNodes.emplace_back(std::make_unique<Wait>(name, waitTime)).get());
	}

	void BehaviorTree::ICompositeNode::GetShuffledNodes(std::vector<INode*>& randomNodes_out) const
	{
		const size_t size = m_childNodes.size();
		randomNodes_out.reserve(size);

		for (size_t i = 0; i < size; ++i)
		{
			randomNodes_out.emplace_back(m_childNodes[i].get());
		}
		std::shuffle(randomNodes_out.begin(), randomNodes_out.end(), math::mt19937_64);
	}

	BehaviorTree::IDecoratorNode::IDecoratorNode(const string::StringID& name)
		: INode(name)
	{
	}

	BehaviorTree::IDecoratorNode::~IDecoratorNode()
	{
	}

	BehaviorTree::Selector* BehaviorTree::IDecoratorNode::SetSelector(const string::StringID& name)
	{
		m_pChildNode = std::make_unique<Selector>(name);
		return static_cast<Selector*>(m_pChildNode.get());
	}

	BehaviorTree::RandomSelector* BehaviorTree::IDecoratorNode::SetRandomSelector(const string::StringID& name)
	{
		m_pChildNode = std::make_unique<RandomSelector>(name);
		return static_cast<RandomSelector*>(m_pChildNode.get());
	}

	BehaviorTree::Sequence* BehaviorTree::IDecoratorNode::SetSequence(const string::StringID& name)
	{
		m_pChildNode = std::make_unique<Sequence>(name);
		return static_cast<Sequence*>(m_pChildNode.get());
	}

	BehaviorTree::Inverter* BehaviorTree::IDecoratorNode::SetInverter(const string::StringID& name)
	{
		m_pChildNode = std::make_unique<Inverter>(name);
		return static_cast<Inverter*>(m_pChildNode.get());
	}

	BehaviorTree::Succeeder* BehaviorTree::IDecoratorNode::SetSucceeder(const string::StringID& name)
	{
		m_pChildNode = std::make_unique<Succeeder>(name);
		return static_cast<Succeeder*>(m_pChildNode.get());
	}

	BehaviorTree::Failer* BehaviorTree::IDecoratorNode::SetFailer(const string::StringID& name)
	{
		m_pChildNode = std::make_unique<Failer>(name);
		return static_cast<Failer*>(m_pChildNode.get());
	}

	BehaviorTree::Loop* BehaviorTree::IDecoratorNode::SetLoop(const string::StringID& name, size_t maxLoopCount)
	{
		m_pChildNode = std::make_unique<Loop>(name, maxLoopCount);
		return static_cast<Loop*>(m_pChildNode.get());
	}

	BehaviorTree::ConditionalLoop* BehaviorTree::IDecoratorNode::SetConditionalLoop(const string::StringID& name)
	{
		m_pChildNode = std::make_unique<ConditionalLoop>(name);
		return static_cast<ConditionalLoop*>(m_pChildNode.get());
	}

	BehaviorTree::Cooldown* BehaviorTree::IDecoratorNode::SetCooldown(const string::StringID& name, float timeCooldown)
	{
		m_pChildNode = std::make_unique<Cooldown>(name, timeCooldown);
		return static_cast<Cooldown*>(m_pChildNode.get());
	}

	BehaviorTree::Action* BehaviorTree::IDecoratorNode::SetAction(const string::StringID& name, FuncAction func)
	{
		m_pChildNode = std::make_unique<Action>(name, func);
		return static_cast<Action*>(m_pChildNode.get());
	}

	BehaviorTree::RunBehavior* BehaviorTree::IDecoratorNode::SetRunBehavior(const string::StringID& name, const string::StringID& behaviorName)
	{
		m_pChildNode = std::make_unique<RunBehavior>(name, behaviorName);
		return static_cast<RunBehavior*>(m_pChildNode.get());
	}

	BehaviorTree::Wait* BehaviorTree::IDecoratorNode::SetWait(const string::StringID& name, float waitTime)
	{
		m_pChildNode = std::make_unique<Wait>(name, waitTime);
		return static_cast<Wait*>(m_pChildNode.get());
	}

	BehaviorTree::ITask::ITask(const string::StringID& name)
		: INode(name)
	{
	}

	BehaviorTree::ITask::~ITask()
	{
	}

	BehaviorTree::Selector::Selector(const string::StringID& name)
		: ICompositeNode(name)
	{
	}

	BehaviorTree::Selector::~Selector()
	{
	}

	BehaviorTree::State BehaviorTree::Selector::Run(float elapsedTime)
	{
		const std::vector<std::unique_ptr<INode>>& childNodes = GetChilds();

		size_t startNodeIndex = 0;
		if (m_emLastState == State::eRunning && m_lastRunningNodeIndex != eInvalidNodeIndex)
		{
			startNodeIndex = m_lastRunningNodeIndex;
		}

		const size_t size = childNodes.size();
		for (size_t i = startNodeIndex; i < size; ++i)
		{
			INode* pChildNode = childNodes[i].get();

			m_emLastState = pChildNode->Run(elapsedTime);
			if (m_emLastState == State::eFail)
				continue;

			m_lastRunningNodeIndex = m_emLastState == State::eRunning ? i : eInvalidNodeIndex;
			return m_emLastState;
		}

		m_emLastState = State::eFail;
		m_lastRunningNodeIndex = eInvalidNodeIndex;

		return m_emLastState;
	}

	BehaviorTree::RandomSelector::RandomSelector(const string::StringID& name)
		: ICompositeNode(name)
	{
	}

	BehaviorTree::RandomSelector::~RandomSelector()
	{
	}

	BehaviorTree::State BehaviorTree::RandomSelector::Run(float elapsedTime)
	{
		size_t startNodeIndex = 0;
		if (m_emLastState == State::eRunning && m_lastRunningNodeIndex != eInvalidNodeIndex)
		{
			startNodeIndex = m_lastRunningNodeIndex;
		}
		else
		{
			m_lastRandomNodes.clear();
			GetShuffledNodes(m_lastRandomNodes);
		}

		const size_t size = m_lastRandomNodes.size();
		for (size_t i = startNodeIndex; i < size; ++i)
		{
			INode* pChildNode = m_lastRandomNodes[i];

			m_emLastState = pChildNode->Run(elapsedTime);
			if (m_emLastState == State::eFail)
				continue;

			m_lastRunningNodeIndex = m_emLastState == State::eRunning ? i : eInvalidNodeIndex;
			return m_emLastState;
		}

		m_emLastState = State::eFail;
		m_lastRunningNodeIndex = eInvalidNodeIndex;

		return m_emLastState;
	}

	BehaviorTree::Sequence::Sequence(const string::StringID& name)
		: ICompositeNode(name)
	{
	}

	BehaviorTree::Sequence::~Sequence()
	{
	}

	BehaviorTree::State BehaviorTree::Sequence::Run(float elapsedTime)
	{
		const std::vector<std::unique_ptr<INode>>& childNodes = GetChilds();

		size_t startNodeIndex = 0;
		if (m_emLastState == State::eRunning && m_lastRunningNodeIndex != eInvalidNodeIndex)
		{
			startNodeIndex = m_lastRunningNodeIndex;
		}

		const size_t size = childNodes.size();
		for (size_t i = startNodeIndex; i < size; ++i)
		{
			INode* pChildNode = childNodes[i].get();

			m_emLastState = pChildNode->Run(elapsedTime);
			if (m_emLastState == State::eSuccess)
				continue;

			m_lastRunningNodeIndex = m_emLastState == State::eRunning ? i : eInvalidNodeIndex;
			return m_emLastState;
		}

		m_emLastState = State::eSuccess;
		m_lastRunningNodeIndex = eInvalidNodeIndex;

		return m_emLastState;
	}

	BehaviorTree::Inverter::Inverter(const string::StringID& name)
		: IDecoratorNode(name)
	{
	}

	BehaviorTree::Inverter::~Inverter()
	{
	}

	BehaviorTree::State BehaviorTree::Inverter::Run(float elapsedTime)
	{
		const State State = GetChild()->Run(elapsedTime);
		switch (State)
		{
		case State::eSuccess:
			return State::eFail;
		case State::eFail:
			return State::eSuccess;
		default:
			return State::eRunning;
		}
	}

	BehaviorTree::Succeeder::Succeeder(const string::StringID& name)
		: IDecoratorNode(name)
	{
	}

	BehaviorTree::Succeeder::~Succeeder()
	{
	}

	BehaviorTree::State BehaviorTree::Succeeder::Run(float elapsedTime)
	{
		GetChild()->Run(elapsedTime);
		return State::eSuccess;
	}

	BehaviorTree::Failer::Failer(const string::StringID& name)
		: IDecoratorNode(name)
	{
	}

	BehaviorTree::Failer::~Failer()
	{
	}

	BehaviorTree::State BehaviorTree::Failer::Run(float elapsedTime)
	{
		GetChild()->Run(elapsedTime);
		return State::eFail;
	}

	BehaviorTree::Loop::Loop(const string::StringID& name, size_t maxLoopCount)
		: IDecoratorNode(name)
		, m_maxLoopCount(maxLoopCount)
	{
	}

	BehaviorTree::Loop::~Loop()
	{
	}

	BehaviorTree::State BehaviorTree::Loop::Run(float elapsedTime)
	{
		for (size_t i = 0; i < m_maxLoopCount; ++i)
		{
			GetChild()->Run(elapsedTime);
		}
		return State::eSuccess;
	}

	BehaviorTree::ConditionalLoop::ConditionalLoop(const string::StringID& name)
		: IDecoratorNode(name)
	{
	}

	BehaviorTree::ConditionalLoop::~ConditionalLoop()
	{
	}

	BehaviorTree::State BehaviorTree::ConditionalLoop::Run(float elapsedTime)
	{
		const State emState = GetChild()->Run(elapsedTime);
		switch (emState)
		{
		case State::eSuccess:
		case State::eRunning:
			return State::eRunning;
		case State::eFail:
			return State::eFail;
		default:
			return State::eSuccess;
		}
	}

	BehaviorTree::Cooldown::Cooldown(const string::StringID& name, float timeCooldown)
		: IDecoratorNode(name)
		, m_timeCooldown(timeCooldown)
	{
	}

	BehaviorTree::Cooldown::~Cooldown()
	{
	}

	BehaviorTree::State BehaviorTree::Cooldown::Run(float elapsedTime)
	{
		m_time += elapsedTime;
		if (m_time >= m_timeCooldown)
		{
			m_time -= m_timeCooldown;
			return GetChild()->Run(elapsedTime);
		}
		return State::eFail;
	}

	BehaviorTree::Action::Action(const string::StringID& name, FuncAction func)
		: ITask(name)
		, m_func(func)
	{
		if (m_func == nullptr)
		{
			LOG_ERROR("Action Node Function is nullptr : %s", GetName().c_str());
		}
	}

	BehaviorTree::Action::~Action()
	{
	}

	BehaviorTree::State BehaviorTree::Action::Run(float elapsedTime)
	{
		if (m_func == nullptr)
		{
			LOG_ERROR("Action Node Function is nullptr : %s", GetName().c_str());
			return State::eFail;
		}

		return m_func() == true ? State::eSuccess : State::eFail;
	}

	BehaviorTree::RunBehavior::RunBehavior(const string::StringID& name, const string::StringID& behaviorName)
		: ITask(name)
		, m_behaviorName(behaviorName)
	{
	}

	BehaviorTree::RunBehavior::~RunBehavior()
	{
	}

	BehaviorTree::State BehaviorTree::RunBehavior::Run(float elapsedTime)
	{
		return State::eSuccess;
	}

	BehaviorTree::Wait::Wait(const string::StringID& name, float waitTime)
		: ITask(name)
		, m_waitTime(waitTime)
	{
	}

	BehaviorTree::Wait::~Wait()
	{
	}

	BehaviorTree::State BehaviorTree::Wait::Run(float elapsedTime)
	{
		m_time += elapsedTime;
		if (m_time >= m_waitTime)
		{
			m_time -= m_waitTime;
			return State::eSuccess;
		}
		return State::eRunning;
	}

	BehaviorTree::Root::Root(const string::StringID& name)
		: IDecoratorNode(name)
	{
	}

	BehaviorTree::Root::~Root()
	{
	}

	BehaviorTree::State BehaviorTree::Root::Run(float elapsedTime)
	{
		return GetChild()->Run(elapsedTime);
	}
}