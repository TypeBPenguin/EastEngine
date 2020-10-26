#pragma once

#include "StringTable.h"

namespace est
{
	/*
				Root
				 |
				 |
	  Selector (only one of these children need to succeed)
		   /             \
		  /               \
		 /                 \
	Door is open?      Sequence (all of these children need to succeed)
	(if door is                /           \
	already open,             /             \
	we are done)             /               \
					   Approach door      Open the door
					  (if this fails
					  then the door
					  cannot be opened)
	*/
	class BehaviorTree
	{
	public:
		BehaviorTree();
		virtual ~BehaviorTree();

	public:
		enum State
		{
			eSuccess = 0,
			eRunning,
			eFail,
		};

		enum InterfaceType
		{
			eComposite = 0,
			eDecorator,
			eTask,
		};

		enum NodeType
		{
			eRoot = 0,
			eSelector,
			eRandomSelector,
			eSequence,
			eRandomSequence,
			eInverter,
			eSucceeder,
			eFailer,
			eLoop,
			eConditionalLoop,
			eCooldown,
			eAction,
			eRunBehavior,
			eWait,
		};

		using Value = std::variant<bool,
			int,
			int64_t,
			float,
			double,
			string::StringID>;

		using FuncAction = std::function<bool()>;

	public:
		class Selector;
		class RandomSelector;
		class Sequence;

		class Inverter;
		class Succeeder;
		class Failer;
		class Loop;
		class ConditionalLoop;
		class Cooldown;

		class Action;
		class RunBehavior;
		class Wait;

	private:
		////////////////////////////// Interface //////////////////////////////
		///////////////////////// Node /////////////////////////
		class INode
		{
		public:
			INode(const string::StringID& name);
			virtual ~INode() = 0;

		public:
			virtual InterfaceType GetInterfaceType() = 0;
			virtual NodeType GetIdentity() = 0;

		public:
			virtual State Run(float elapsedTime) = 0;

		public:
			const string::StringID& GetName() const { return m_name; }

		private:
			const string::StringID m_name;
		};

		////////////////////////////// Action //////////////////////////////
		class ITask : public INode
		{
		public:
			ITask(const string::StringID& name);
			virtual ~ITask() = 0;

		public:
			virtual InterfaceType GetInterfaceType() override { return InterfaceType::eTask; }
		};

	public:
		///////////////////////// Composite /////////////////////////
		class ICompositeNode : public INode
		{
		public:
			ICompositeNode(const string::StringID& name);
			virtual ~ICompositeNode() = 0;

		public:
			virtual InterfaceType GetInterfaceType() override { return InterfaceType::eComposite; }

		public:
			Selector* AddSelector(const string::StringID& name);
			RandomSelector* AddRandomSelector(const string::StringID& name);
			Sequence* AddSequence(const string::StringID& name);

			Inverter* AddInverter(const string::StringID& name);
			Succeeder* AddSucceeder(const string::StringID& name);
			Failer* AddFailer(const string::StringID& name);
			Loop* AddLoop(const string::StringID& name, size_t maxLoopCount);
			ConditionalLoop* AddConditionalLoop(const string::StringID& name);
			Cooldown* AddCooldown(const string::StringID& name, float timeCooldown);

			Action* AddAction(const string::StringID& name, FuncAction func);
			RunBehavior* AddRunBehavior(const string::StringID& name, const string::StringID& behaviorName);
			Wait* AddWait(const string::StringID& name, float waitTime);

		protected:
			const std::vector<std::unique_ptr<INode>>& GetChilds() const { return m_childNodes; }
			void GetShuffledNodes(std::vector<INode*>& randomNodes_out) const;

		protected:
			State m_emLastState{ State::eSuccess };

			enum : size_t
			{
				eInvalidNodeIndex = std::numeric_limits<size_t>::max(),
			};
			size_t m_lastRunningNodeIndex{ eInvalidNodeIndex };

		private:
			std::vector<std::unique_ptr<INode>> m_childNodes;
		};

		///////////////////////// Decorator /////////////////////////
		class IDecoratorNode : public INode
		{
		public:
			IDecoratorNode(const string::StringID& name);
			virtual ~IDecoratorNode() = 0;

		public:
			virtual InterfaceType GetInterfaceType() override { return InterfaceType::eDecorator; }

		public:
			Selector* SetSelector(const string::StringID& name);
			RandomSelector* SetRandomSelector(const string::StringID& name);
			Sequence* SetSequence(const string::StringID& name);

			Inverter* SetInverter(const string::StringID& name);
			Succeeder* SetSucceeder(const string::StringID& name);
			Failer* SetFailer(const string::StringID& name);
			Loop* SetLoop(const string::StringID& name, size_t maxLoopCount);
			ConditionalLoop* SetConditionalLoop(const string::StringID& name);
			Cooldown* SetCooldown(const string::StringID& name, float timeCooldown);

			Action* SetAction(const string::StringID& name, FuncAction func);
			RunBehavior* SetRunBehavior(const string::StringID& name, const string::StringID& behaviorName);
			Wait* SetWait(const string::StringID& name, float waitTime);

		protected:
			INode* GetChild() const { return m_pChildNode.get(); }

		private:
			std::unique_ptr<INode> m_pChildNode;
		};

	public:
		////////////////////////////// Composite //////////////////////////////
		class Selector : public ICompositeNode
		{
		public:
			Selector(const string::StringID& name);
			virtual ~Selector();

		public:
			virtual NodeType GetIdentity() override { return NodeType::eSelector; }
			virtual State Run(float elapsedTime) override;
		};

		class RandomSelector : public ICompositeNode
		{
		public:
			RandomSelector(const string::StringID& name);
			virtual ~RandomSelector();

		public:
			virtual NodeType GetIdentity() override { return NodeType::eRandomSelector; }
			virtual State Run(float elapsedTime) override;

		private:
			std::vector<INode*> m_lastRandomNodes;
		};

		class Sequence : public ICompositeNode
		{
		public:
			Sequence(const string::StringID& name);
			virtual ~Sequence();

		public:
			virtual NodeType GetIdentity() override { return NodeType::eSequence; }
			virtual State Run(float elapsedTime) override;
		};

		////////////////////////////// Decorator //////////////////////////////
		class Inverter : public IDecoratorNode
		{
		public:
			Inverter(const string::StringID& name);
			virtual ~Inverter();

		public:
			virtual NodeType GetIdentity() override { return NodeType::eInverter; }

		private:
			virtual State Run(float elapsedTime) override;
		};

		class Succeeder : public IDecoratorNode
		{
		public:
			Succeeder(const string::StringID& name);
			virtual ~Succeeder();

		public:
			virtual NodeType GetIdentity() override { return NodeType::eSucceeder; }

		private:
			virtual State Run(float elapsedTime) override;
		};

		class Failer : public IDecoratorNode
		{
		public:
			Failer(const string::StringID& name);
			virtual ~Failer();

		public:
			virtual NodeType GetIdentity() override { return NodeType::eFailer; }

		private:
			virtual State Run(float elapsedTime) override;
		};

		class Loop : public IDecoratorNode
		{
		public:
			Loop(const string::StringID& name, size_t maxLoopCount);
			virtual ~Loop();

		public:
			virtual NodeType GetIdentity() override { return NodeType::eLoop; }

		private:
			virtual State Run(float elapsedTime) override;

		private:
			const size_t m_maxLoopCount{0};
		};

		class ConditionalLoop : public IDecoratorNode
		{
		public:
			ConditionalLoop(const string::StringID& name);
			virtual ~ConditionalLoop();

		public:
			virtual NodeType GetIdentity() override { return NodeType::eConditionalLoop; }

		private:
			virtual State Run(float elapsedTime) override;
		};

		class Cooldown : public IDecoratorNode
		{
		public:
			Cooldown(const string::StringID& name, float timeCooldown);
			virtual ~Cooldown();

		public:
			virtual NodeType GetIdentity() override { return NodeType::eCooldown; }

		private:
			virtual State Run(float elapsedTime) override;

		private:
			float m_time{ 0.f };
			float m_timeCooldown{ 0.f };
		};

		////////////////////////////// Task //////////////////////////////
		class Action : public ITask
		{
		public:
			Action(const string::StringID& name, FuncAction func);
			virtual ~Action();

		public:
			virtual NodeType GetIdentity() override { return NodeType::eAction; }

		private:
			virtual State Run(float elapsedTime) override;

		private:
			FuncAction m_func;
		};

		class RunBehavior : public ITask
		{
		public:
			RunBehavior(const string::StringID& name, const string::StringID& behaviorName);
			virtual ~RunBehavior();

		public:
			virtual NodeType GetIdentity() override { return NodeType::eRunBehavior; }

		private:
			virtual State Run(float elapsedTime) override;

		private:
			const string::StringID& m_behaviorName;
		};

		class Wait : public ITask
		{
		public:
			Wait(const string::StringID& name, float waitTime);
			virtual ~Wait();

		public:
			virtual NodeType GetIdentity() override { return NodeType::eWait; }

		private:
			virtual State Run(float elapsedTime) override;

		private:
			float m_time{ 0.f };
			float m_waitTime{ 0.f };
		};

	private:
		class Root : public IDecoratorNode
		{
		public:
			Root(const string::StringID& name);
			virtual ~Root();

		public:
			virtual NodeType GetIdentity() override { return NodeType::eRoot; }
			virtual State Run(float elapsedTime) override;
		};

	public:
		IDecoratorNode * GetRoot() const { return m_pRoot.get(); }
		State Run(float elapsedTime) { return m_pRoot->Run(elapsedTime); }

	private:
		std::unique_ptr<Root> m_pRoot;
	};
}