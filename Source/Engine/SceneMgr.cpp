#include "stdafx.h"
#include "SceneMgr.h"

namespace EastEngine
{
	SSceneMgr::SSceneMgr()
		: m_pCurScene(nullptr)
		, m_pChangeScene(nullptr)
		, m_isInit(false)
	{
	}

	SSceneMgr::~SSceneMgr()
	{
		Release();
	}

	bool SSceneMgr::Init()
	{
		if (m_isInit == true)
			return true;

		m_isInit = true;

		return true;
	}

	void SSceneMgr::Release()
	{
		if (m_isInit == false)
			return;

		if (m_pCurScene != nullptr)
		{
			m_pCurScene->Exit();
		}

		m_pCurScene = nullptr;
		m_pChangeScene = nullptr;
		
		std::for_each(m_umapScene.begin(), m_umapScene.end(), DeleteSTLMapObject());
		m_umapScene.clear();

		m_isInit = false;
	}

	void SSceneMgr::Update(float fElapsedTime)
	{
		if (m_pChangeScene != nullptr)
		{
			if (m_pCurScene != nullptr)
			{
				m_pCurScene->Exit();
			}

			m_pCurScene = m_pChangeScene;

			if (m_pCurScene != nullptr)
			{
				m_pCurScene->Enter();
			}

			m_pChangeScene = nullptr;
		}

		if (m_pCurScene != nullptr)
		{
			m_pCurScene->Update(fElapsedTime);
		}
	}

	void SSceneMgr::AddScene(SceneInterface* pScene)
	{
		if (pScene == nullptr)
			return;

		DelScene(pScene);

		pScene->SetSceneMgr(this);

		m_umapScene.emplace(pScene->GetSceneName(), pScene);
	}

	void SSceneMgr::DelScene(SceneInterface* pScene)
	{
		DelScene(pScene->GetSceneName().c_str());
	}

	void SSceneMgr::DelScene(String::StringID strSceneName)
	{
		auto iter = m_umapScene.find(strSceneName);
		if (iter == m_umapScene.end())
			return;

		bool isSame = m_pCurScene == iter->second;
		SafeDelete(iter->second);
		m_umapScene.erase(iter);

		if (isSame == true)
		{
			if (m_umapScene.empty() == true)
			{
				m_pCurScene = nullptr;
			}
			else
			{
				m_pCurScene = m_umapScene.begin()->second;
			}
		}
	}

	void SSceneMgr::ChangeScene(String::StringID strSceneName)
	{
		if (m_pCurScene != nullptr)
		{
			if (m_pCurScene->GetSceneName() == strSceneName)
				return;
		}

		auto iter = m_umapScene.find(strSceneName);
		if (iter == m_umapScene.end())
			return;

		m_pChangeScene = iter->second;
	}
	
	SceneInterface* SSceneMgr::GetScene(String::StringID strSceneName)
	{
		auto iter = m_umapScene.find(strSceneName);
		if (iter == m_umapScene.end())
			return nullptr;

		return iter->second;
	}
}