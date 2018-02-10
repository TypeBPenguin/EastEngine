#include "stdafx.h"
#include "SceneManager.h"

namespace EastEngine
{
	class SceneManager::Impl
	{
	public:
		Impl();
		~Impl();

	public:
		void Update(float fElapsedTime);
		void Flush();

	public:
		void AddScene(IScene* pScene);
		void RemoveScene(IScene* pScene);
		void RemoveScene(const String::StringID& strSceneName);

		void ChangeScene(const String::StringID& strSceneName);

		IScene* GetCurScene();
		IScene* GetScene(const String::StringID& strSceneName);

	private:
		std::unordered_map<String::StringID, IScene*> m_umapScene;
		IScene* m_pCurScene{ nullptr };
		IScene* m_pChangeScene{ nullptr };
	};

	SceneManager::Impl::Impl()
	{
	}

	SceneManager::Impl::~Impl()
	{
		if (m_pCurScene != nullptr)
		{
			m_pCurScene->Exit();
		}

		m_pCurScene = nullptr;
		m_pChangeScene = nullptr;

		std::for_each(m_umapScene.begin(), m_umapScene.end(), DeleteSTLMapObject());
		m_umapScene.clear();
	}

	void SceneManager::Impl::Update(float fElapsedTime)
	{
		PERF_TRACER_EVENT("SceneManager::Update", "");
		if (m_pCurScene != nullptr)
		{
			PERF_TRACER_PUSHARGS("Scene Name", m_pCurScene->GetSceneName().c_str());
			m_pCurScene->Update(fElapsedTime);
		}
	}

	void SceneManager::Impl::Flush()
	{
		PERF_TRACER_EVENT("SceneManager::Flush", "");
		if (m_pChangeScene != nullptr)
		{
			if (m_pCurScene != nullptr)
			{
				PERF_TRACER_EVENT("SceneManager::Flush", "Exit");
				PERF_TRACER_PUSHARGS("Scene Name", m_pCurScene->GetSceneName().c_str());
				m_pCurScene->Exit();
			}

			m_pCurScene = m_pChangeScene;

			if (m_pCurScene != nullptr)
			{
				PERF_TRACER_EVENT("SceneManager::Flush", "Enter");
				PERF_TRACER_PUSHARGS("Scene Name", m_pCurScene->GetSceneName().c_str());
				m_pCurScene->Enter();
			}

			m_pChangeScene = nullptr;
		}
	}

	void SceneManager::Impl::AddScene(IScene* pScene)
	{
		if (pScene == nullptr)
			return;

		RemoveScene(pScene);

		m_umapScene.emplace(pScene->GetSceneName(), pScene);
	}

	void SceneManager::Impl::RemoveScene(IScene* pScene)
	{
		RemoveScene(pScene->GetSceneName().c_str());
	}

	void SceneManager::Impl::RemoveScene(const String::StringID& strSceneName)
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

	void SceneManager::Impl::ChangeScene(const String::StringID& strSceneName)
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

	IScene* SceneManager::Impl::GetScene(const String::StringID& strSceneName)
	{
		auto iter = m_umapScene.find(strSceneName);
		if (iter == m_umapScene.end())
			return nullptr;

		return iter->second;
	}

	SceneManager::SceneManager()
		: m_pImpl{ std::make_unique<Impl>() }
	{
	}

	SceneManager::~SceneManager()
	{
	}

	void SceneManager::Update(float fElapsedTime)
	{
		m_pImpl->Update(fElapsedTime);
	}

	void SceneManager::Flush()
	{
		m_pImpl->Flush();
	}

	void SceneManager::AddScene(IScene* pScene)
	{
		m_pImpl->AddScene(pScene);
	}

	void SceneManager::RemoveScene(IScene* pScene)
	{
		m_pImpl->RemoveScene(pScene->GetSceneName());
	}

	void SceneManager::RemoveScene(const String::StringID& strSceneName)
	{
		m_pImpl->RemoveScene(strSceneName);
	}

	void SceneManager::ChangeScene(const String::StringID& strSceneName)
	{
		m_pImpl->ChangeScene(strSceneName);
	}
	
	IScene* SceneManager::GetScene(const String::StringID& strSceneName)
	{
		return m_pImpl->GetScene(strSceneName);
	}
}