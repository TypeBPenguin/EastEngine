#include "stdafx.h"
#include "SceneManager.h"

namespace est
{
	class SceneManager::Impl
	{
	public:
		Impl();
		~Impl();

	public:
		void Update(float elapsedTime);

	public:
		void AddScene(std::unique_ptr<IScene>&& pScene);
		void RemoveScene(const string::StringID& sceneName);

		void ChangeScene(const string::StringID& sceneName);

		IScene* GetScene(const string::StringID& sceneName) const;

	private:
		std::unordered_map<string::StringID, std::unique_ptr<IScene>> m_umapScene;
		string::StringID m_curSceneID;
		string::StringID m_changeSceneID;
	};

	SceneManager::Impl::Impl()
	{
	}

	SceneManager::Impl::~Impl()
	{
		IScene* pScene = GetScene(m_curSceneID);
		if (pScene != nullptr)
		{
			std::queue<gameobject::ActorPtr> savedPrevSceneActors;
			pScene->Exit(savedPrevSceneActors);
		}

		m_curSceneID = {};
		m_changeSceneID = {};

		m_umapScene.clear();
	}

	void SceneManager::Impl::Update(float elapsedTime)
	{
		TRACER_EVENT(__FUNCTIONW__);
		if (m_changeSceneID.empty() == false)
		{
			std::queue<gameobject::ActorPtr> savedPrevSceneActors;

			IScene* pCurScene = GetScene(m_curSceneID);
			if (pCurScene != nullptr)
			{
				TRACER_BEGINEVENT(L"Exit");
				TRACER_PUSHARGS(L"Scene Name", m_curSceneID.c_str());
				pCurScene->Exit(savedPrevSceneActors);
				TRACER_ENDEVENT();
			}

			m_curSceneID = m_changeSceneID;

			pCurScene = GetScene(m_curSceneID);
			if (pCurScene != nullptr)
			{
				TRACER_BEGINEVENT(L"Enter");
				TRACER_PUSHARGS(L"Scene Name", m_curSceneID.c_str());
				pCurScene->Enter(savedPrevSceneActors);
				TRACER_ENDEVENT();
			}

			m_changeSceneID = {};
		}

		IScene* pCurScene = GetScene(m_curSceneID);
		if (pCurScene != nullptr)
		{
			TRACER_PUSHARGS(L"Scene Name", m_curSceneID.c_str());
			pCurScene->Update(elapsedTime);
		}
	}

	void SceneManager::Impl::AddScene(std::unique_ptr<IScene>&& pScene)
	{
		if (pScene == nullptr)
			return;

		RemoveScene(pScene->GetName());

		const string::StringID sceneName = pScene->GetName();
		m_umapScene.emplace(sceneName, std::move(pScene));
	}

	void SceneManager::Impl::RemoveScene(const string::StringID& sceneName)
	{
		auto iter = m_umapScene.find(sceneName);
		if (iter == m_umapScene.end())
			return;

		const bool isSame = m_curSceneID == sceneName;
		m_umapScene.erase(iter);

		if (isSame == true)
		{
			if (m_umapScene.empty() == true)
			{
				m_curSceneID = {};
			}
			else
			{
				m_curSceneID = m_umapScene.begin()->second->GetName();
			}
		}
	}

	void SceneManager::Impl::ChangeScene(const string::StringID& sceneName)
	{
		if (m_curSceneID.empty() == false && m_curSceneID == sceneName)
			return;

		auto iter = m_umapScene.find(sceneName);
		if (iter == m_umapScene.end())
			return;

		m_changeSceneID = sceneName;
	}

	IScene* SceneManager::Impl::GetScene(const string::StringID& sceneName) const
	{
		auto iter = m_umapScene.find(sceneName);
		if (iter == m_umapScene.end())
			return nullptr;

		return iter->second.get();
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

	void SceneManager::AddScene(std::unique_ptr<IScene>&& pScene)
	{
		m_pImpl->AddScene(std::move(pScene));
	}

	void SceneManager::RemoveScene(const string::StringID& sceneName)
	{
		m_pImpl->RemoveScene(sceneName);
	}

	void SceneManager::ChangeScene(const string::StringID& sceneName)
	{
		m_pImpl->ChangeScene(sceneName);
	}
	
	IScene* SceneManager::GetScene(const string::StringID& sceneName) const
	{
		return m_pImpl->GetScene(sceneName);
	}
}