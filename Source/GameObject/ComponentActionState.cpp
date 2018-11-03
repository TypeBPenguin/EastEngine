#include "stdafx.h"
#include "ComponentActionState.h"

namespace StrID
{
	RegisterStringID(Root);
}

namespace eastengine
{
	namespace gameobject
	{
		ActionStateInterface::ActionStateInterface(const string::StringID& strName)
			: m_pOwnerComponent(nullptr)
			, m_strName(strName)
			, m_pParentNode(nullptr)
		{
		}

		ActionStateInterface::~ActionStateInterface()
		{
			m_pParentNode = nullptr;
			std::for_each(m_vecChildNode.begin(), m_vecChildNode.end(), [](ActionStateInterface* pNode)
			{
				SafeDelete(pNode);
			});
			m_vecChildNode.clear();
		}

		void ActionStateInterface::Init(ComponentActionState* pOwner, ActionStateInterface* pParentNode)
		{
			m_pOwnerComponent = pOwner;
			m_pActor = m_pOwnerComponent->GetOwner();
			m_pParentNode = pParentNode;
		}

		ActionStateInterface* ActionStateInterface::AddChildNode(const string::StringID& strName, ActionStateInterface* pChildNode)
		{
			pChildNode->Init(m_pOwnerComponent, this);

			m_vecChildNode.emplace_back(pChildNode);

			return pChildNode;
		}

		void ActionStateInterface::SetState(const string::StringID& strStateName)
		{
			m_pOwnerComponent->SetState(strStateName);
		}

		ComponentActionState::ActionStateRoot::ActionStateRoot(ComponentActionState* pOwner)
			: ActionStateInterface(StrID::Root)
		{
		}

		ComponentActionState::ActionStateRoot::~ActionStateRoot()
		{
		}

		ComponentActionState::ComponentActionState(IActor* pOwner)
			: IComponent(pOwner, EmComponent::eActionState)
			, m_pRootNode(nullptr)
			, m_pCurNode(nullptr)
		{
			m_pRootNode = new ActionStateRoot(this);
			m_pRootNode->Init(this, nullptr);
		}

		ComponentActionState::~ComponentActionState()
		{
			m_pCurNode = nullptr;
			SafeDelete(m_pRootNode);
		}

		void ComponentActionState::Update(float fElapsedTime)
		{
			changeState();

			if (m_pCurNode != nullptr)
			{
				m_fUpdateTime += fElapsedTime;

				if (m_pCurNode->Update(m_fUpdateTime, fElapsedTime) == false)
				{
					SetState(m_pCurNode->GetParentName());
				}
			}
		}

		void ComponentActionState::SetState(const string::StringID& strStateName)
		{
			m_strChangeState = strStateName;
		}
		
		void ComponentActionState::changeState()
		{
			if (m_pCurNode == nullptr)
			{
				m_pCurNode = m_pRootNode;
			}

			if (m_pCurNode != nullptr && m_strChangeState.empty() == false)
			{
				m_fUpdateTime = 0.f;

				ActionStateInterface* pNode = m_pCurNode->GetNode(m_strChangeState);
				m_strChangeState.clear();

				if (pNode != nullptr)
				{
					m_pCurNode->Leave();
					if (pNode->Enter() == true)
					{
						m_pCurNode = pNode;
						return;
					}
				}

				ActionStateInterface* pParent = m_pCurNode->GetParentNode();
				if (pParent == nullptr || pParent == m_pCurNode)
				{
					m_pCurNode->Enter();
					return;
				}

				pParent->Enter();
				m_pCurNode = pParent;
			}
		}
	}
}