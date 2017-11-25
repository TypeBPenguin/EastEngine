#pragma once

// Âü°í : http://www.cplusplus.com/forum/general/141582/

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
namespace EastEngine
{
	namespace GameObject
	{
		class BehaviorTree
		{
		public:
			BehaviorTree();
			virtual ~BehaviorTree();

		public:
			class INode
			{
			public:
				INode(String::StringID strName) : m_strName(strName) {}
				virtual ~INode() = 0 {}

				virtual bool Run(float fElapsedTime) = 0;

			public:
				String::StringID GetName() { return m_strName; }

			private:
				String::StringID m_strName;
			};

			class ICompositeNode : public INode
			{
			public:
				ICompositeNode(String::StringID strName) : INode(strName) {}
				virtual ~ICompositeNode() {}

			public:
				const std::vector<INode*>& GetChildNode() const { return m_vecChildNode; }
				void AddChildNode(INode* pChildNode) { m_vecChildNode.emplace_back(pChildNode); }
				void AddChildNodes(std::initializer_list<INode*>&& newChildren)
				{
					for (INode* pChildNode : newChildren)
					{
						AddChildNode(pChildNode);
					}
				}

				template <typename CONTAINER>
				void AddChildNodes(const CONTAINER& newChildren)
				{
					for (INode* pChildNode : newChildren)
					{
						AddChildNode(pChildNode);
					}
				}

			protected:
				void getShuffledNodes(std::vector<INode*>& out_vecRandomNode)
				{
					out_vecRandomNode = m_vecChildNode;

					std::shuffle(out_vecRandomNode.begin(), out_vecRandomNode.end(), Math::mt19937());
				}

			private:
				std::vector<INode*> m_vecChildNode;
			};

			class Selector : public ICompositeNode
			{
			public:
				Selector(String::StringID strName) : ICompositeNode(strName) {};
				virtual ~Selector() {}

				virtual bool Run(float fElapsedTime) override
				{
					const std::vector<INode*>& vecChildNode = GetChildNode();
					for (INode* pChildNode : vecChildNode)
					{
						if (pChildNode->Run(fElapsedTime))
							return true;
					}

					return false;
				}
			};

			class RandomSelector : public ICompositeNode
			{
			public:
				RandomSelector(String::StringID strName) : ICompositeNode(strName) {};
				virtual ~RandomSelector() {}

				virtual bool Run(float fElapsedTime) override
				{
					std::vector<INode*> vecRandomNode;
					getShuffledNodes(vecRandomNode);

					for (INode* pChildNode : vecRandomNode)
					{
						if (pChildNode->Run(fElapsedTime))
							return true;
					}

					return false;
				}
			};

			class Sequence : public ICompositeNode
			{
			public:
				Sequence(String::StringID strName) : ICompositeNode(strName) {};
				virtual ~Sequence() {}

				virtual bool Run(float fElapsedTime) override
				{
					const std::vector<INode*>& vecChildNode = GetChildNode();
					for (INode* pChildNode : vecChildNode)
					{
						if (pChildNode->Run(fElapsedTime) == false)
							return false;
					}

					return true;
				}
			};

			//class Action : public INode
			//{
			//public:
			//	Action(String::StringID strName, std::function<bool()> func) : INode(strName), m_func(func) {}
			//	virtual ~Action() {};
			//
			//	virtual bool Run(float fElapsedTime) override { m_func(); }
			//
			//private:
			//	std::function<bool()> m_func;
			//};

			class DecoratorNode : public INode
			{
			public:
				DecoratorNode(String::StringID strName) : INode(strName), m_pChildNode(nullptr) {}
				virtual ~DecoratorNode() {}

				void SetChild(INode* pNewChild) { m_pChildNode = pNewChild; }

			protected:
				INode* getChild() const { return m_pChildNode; }

			private:
				INode* m_pChildNode;
			};

			class Root : public DecoratorNode
			{
			private:
				friend class BehaviorTree;

				Root(String::StringID strName) : DecoratorNode(strName) {}
				virtual ~Root() {}

				virtual bool Run(float fElapsedTime) override
				{
					return getChild()->Run(fElapsedTime);
				}
			};

			class Inverter : public DecoratorNode
			{
			public:
				Inverter(String::StringID strName) : DecoratorNode(strName) {}
				virtual ~Inverter() {}

			private:
				virtual bool Run(float fElapsedTime) override
				{
					return getChild()->Run(fElapsedTime) == false;
				}
			};

			class Succeeder : public DecoratorNode
			{
			public:
				Succeeder(String::StringID strName) : DecoratorNode(strName) {}
				virtual ~Succeeder() {}

			private:
				virtual bool Run(float fElapsedTime) override
				{
					getChild()->Run(fElapsedTime);
					return true;
				}
			};

			class Failer : public DecoratorNode
			{
			public:
				Failer(String::StringID strName) : DecoratorNode(strName) {}
				virtual ~Failer() {}

			private:
				virtual bool Run(float fElapsedTime) override
				{
					getChild()->Run(fElapsedTime);
					return false;
				}
			};

			class Repeater : public DecoratorNode
			{
			private:
				Repeater(String::StringID strName, int num = -1) : DecoratorNode(strName), m_nRepeats(num) {}
				virtual ~Repeater() {}

				virtual bool Run(float fElapsedTime) override
				{
					if (m_nRepeats == -1)
					{
						while (true)
						{
							getChild()->Run(fElapsedTime);
						}
					}
					else
					{
						for (int i = 0; i < m_nRepeats; ++i)
						{
							getChild()->Run(fElapsedTime);
						}

						return getChild()->Run(fElapsedTime);
					}
				}

			private:
				int m_nRepeats;
			};

			class RepeatUntilFail : public DecoratorNode
			{
			public:
				RepeatUntilFail(String::StringID strName) : DecoratorNode(strName) {}
				virtual ~RepeatUntilFail() {}

			private:
				virtual bool Run(float fElapsedTime) override
				{
					while (getChild()->Run(fElapsedTime))
					{
					}

					return true;
				}
			};

			/*template <typename T>
			class StackNode : public INode
			{
			protected:
				CStackNode(String::StringID strName, std::stack<T*>& stack) : INode(strName), m_stack(stack) {}

			protected:
				std::stack<T*>& m_stack;
			};

			template <typename T>
			class PushToStack : public CStackNode<T>
			{
			public:
				CPushToStack(String::StringID strName, T*& t, std::stack<T*>& stack) : CStackNode<T>(strName, s), m_item(t) {}

			private:
				virtual bool Run(float fElapsedTime) override
				{
					this->m_stack.push(item);
					return true;
				}

			private:
				T*& m_item;
			};

			template <typename T>
			class GetStack : public CStackNode<T>
			{
			public:
				CGetStack(String::StringID strName, std::stack<T*>& stack, const std::stack<T*>& obtainedStack, T* t = nullptr)
					: CStackNode<T>(strName, s), m_obtainedStack(obtainedStack), m_pObject(pObject) {}

			private:
				virtual bool Run(float fElapsedTime) override
				{
					this->m_stack = m_obtainedStack;
					if (m_pObject != nullptr)
					{
						this->m_stack.push(pObject);
					}

					return true;
				}

			private:
				const std::stack<T*>& m_obtainedStack;
				T* m_pObject;
			};

			template <typename T>
			class PopFromStack : public CStackNode<T>
			{
			public:
				CPopFromStack(String::StringID strName, T*& t, std::stack<T*>& stack) : CStackNode<T>(strName, s), m_item(t) {}

			private:
				virtual bool Run(float fElapsedTime) override
				{
					if (this->m_stack.empty())
						return false;

					m_item = this->m_stack.top();

					this->m_stack.pop();

					return true;
				}

			private:
				T*& m_item;
			};

			template <typename T>
			class SetVariable : public INode
			{
			public:
				CSetVariable(String::StringID strName, T*& t, T*& pObject) : INode(strName), m_pVariable(t), m_pObject(pObject) {}

				virtual bool Run(float fElapsedTime) override
				{
					m_pVariable = m_pObject;
					return true;
				}

			private:
				T*& m_pVariable;
				T*& m_pObject;
			};

			template <typename T>
			class IsNull : public INode
			{
			public:
				IsNull(String::StringID strName, T*& t) : INode(strName), m_pObject(t) {}

				virtual bool Run(float fElapsedTime) override
				{
					return m_pObject == nullptr;
				}

			private:
				T*& m_pObject;
			};*/

		public:
			void SetRootChild(INode* pRootChild) const { m_pRoot->SetChild(pRootChild); }
			bool Run(float fElapsedTime) const { return m_pRoot->Run(fElapsedTime); }

		private:
			Root* m_pRoot;
		};
	}
}