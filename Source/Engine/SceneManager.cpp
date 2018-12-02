#include "stdafx.h"
#include "SceneManager.h"

namespace eastengine
{
	class SceneManager::Impl
	{
	public:
		Impl();
		~Impl();

	public:
		void Update(float elapsedTime);

	public:
		void AddScene(IScene* pScene);
		void RemoveScene(IScene* pScene);
		void RemoveScene(const string::StringID& strSceneName);

		void ChangeScene(const string::StringID& strSceneName);

		IScene* GetScene(const string::StringID& strSceneName);

	private:
		std::unordered_map<string::StringID, IScene*> m_umapScene;
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

	void SceneManager::Impl::Update(float elapsedTime)
	{
		TRACER_EVENT(__FUNCTION__);
		if (m_pChangeScene != nullptr)
		{
			if (m_pCurScene != nullptr)
			{
				TRACER_BEGINEVENT("Exit");
				TRACER_PUSHARGS("Scene Name", m_pCurScene->GetName().c_str());
				m_pCurScene->Exit();
				TRACER_ENDEVENT();
			}

			m_pCurScene = m_pChangeScene;

			if (m_pCurScene != nullptr)
			{
				TRACER_BEGINEVENT("Enter");
				TRACER_PUSHARGS("Scene Name", m_pCurScene->GetName().c_str());
				m_pCurScene->Enter();
				TRACER_ENDEVENT();
			}

			m_pChangeScene = nullptr;
		}

		if (m_pCurScene != nullptr)
		{
			TRACER_PUSHARGS("Scene Name", m_pCurScene->GetName().c_str());
			m_pCurScene->Update(elapsedTime);
		}
	}

	void SceneManager::Impl::AddScene(IScene* pScene)
	{
		if (pScene == nullptr)
			return;

		RemoveScene(pScene);

		m_umapScene.emplace(pScene->GetName(), pScene);
	}

	void SceneManager::Impl::RemoveScene(IScene* pScene)
	{
		RemoveScene(pScene->GetName().c_str());
	}

	void SceneManager::Impl::RemoveScene(const string::StringID& strSceneName)
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

	void SceneManager::Impl::ChangeScene(const string::StringID& strSceneName)
	{
		if (m_pCurScene != nullptr)
		{
			if (m_pCurScene->GetName() == strSceneName)
				return;
		}

		auto iter = m_umapScene.find(strSceneName);
		if (iter == m_umapScene.end())
			return;

		m_pChangeScene = iter->second;
	}

	IScene* SceneManager::Impl::GetScene(const string::StringID& strSceneName)
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

	void SceneManager::Update(float elapsedTime)
	{
		m_pImpl->Update(elapsedTime);
	}

	void SceneManager::AddScene(IScene* pScene)
	{
		m_pImpl->AddScene(pScene);
	}

	void SceneManager::RemoveScene(IScene* pScene)
	{
		m_pImpl->RemoveScene(pScene->GetName());
	}

	void SceneManager::RemoveScene(const string::StringID& strSceneName)
	{
		m_pImpl->RemoveScene(strSceneName);
	}

	void SceneManager::ChangeScene(const string::StringID& strSceneName)
	{
		m_pImpl->ChangeScene(strSceneName);
	}
	
	IScene* SceneManager::GetScene(const string::StringID& strSceneName)
	{
		return m_pImpl->GetScene(strSceneName);
	}
}