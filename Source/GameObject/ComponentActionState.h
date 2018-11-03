#pragma once

#include "ComponentInterface.h"

namespace eastengine
{
	namespace gameobject
	{
		class ComponentActionState;

		class ActionStateInterface
		{
		public:
			ActionStateInterface(const string::StringID& strName);
			virtual ~ActionStateInterface();

			void Init(ComponentActionState* pOwner, ActionStateInterface* pParentNode);

			virtual bool Enter() = 0;
			virtual bool Update(float fUpdateTime, float fElapsedTime) = 0;
			virtual void Leave() = 0;

		public:
			const string::StringID& GetName() { return m_strName; }

			ActionStateInterface* GetParentNode() { return m_pParentNode; }
			const string::StringID& GetParentName()
			{
				if (m_pParentNode == nullptr)
					return StrID::EmptyString;

				return m_pParentNode->GetName();
			}

			ActionStateInterface* GetNode(const string::StringID& strStateName)
			{
				for (auto& pNode : m_vecChildNode)
				{
					if (pNode->GetName() == strStateName)
						return pNode;
				}

				return nullptr;
			}

			const std::vector<ActionStateInterface*>& GetChildNodes() { return m_vecChildNode; }
			ActionStateInterface* AddChildNode(const string::StringID& strName, ActionStateInterface* pChildNode);
			void AddChildNode(ActionStateInterface* pChildNode) { m_vecChildNode.emplace_back(pChildNode); }
			IActor* GetActor() const { return m_pActor; }
			ComponentActionState* GetComponent() const { return m_pOwnerComponent; }

			void SetState(const string::StringID& strStateName);

		private:
			IActor* m_pActor;
			ComponentActionState* m_pOwnerComponent;
			string::StringID m_strName;

			ActionStateInterface* m_pParentNode;
			std::vector<ActionStateInterface*> m_vecChildNode;
		};

		class ComponentActionState : public IComponent
		{
		private:
			class ActionStateRoot : public ActionStateInterface
			{
			public:
				ActionStateRoot(ComponentActionState* pOwner);
				virtual ~ActionStateRoot();

				virtual bool Enter() override { return true; }
				virtual bool Update(float fUpdateTime, float fElapsedTime) override { return true; }
				virtual void Leave() override {}
			};

		public:
			ComponentActionState(IActor* pOwner);;
			virtual ~ComponentActionState();

		public:
			virtual void Update(float fElapsedTime) override;

			ActionStateInterface* GetRootNode() { return m_pRootNode; }

			void SetState(const string::StringID& strStateName);
			ActionStateInterface* GetNode(const string::StringID& strName)
			{
				auto iter = m_umapActionStateNode.find(strName);
				if (iter != m_umapActionStateNode.end())
					return iter->second;

				return nullptr;
			}

			void RegisterNode(ActionStateInterface* pNode)
			{
				if (pNode == nullptr)
					return;

				auto iter = m_umapActionStateNode.find(pNode->GetName());
				if (iter == m_umapActionStateNode.end())
				{
					m_umapActionStateNode.emplace(pNode->GetName(), pNode);
				}
			}

		private:
			void changeState();

		private:
			ActionStateInterface* m_pRootNode;
			ActionStateInterface* m_pCurNode;

			string::StringID m_strChangeState;

			float m_fUpdateTime;

			std::unordered_map<string::StringID, ActionStateInterface*> m_umapActionStateNode;
		};
	}
}