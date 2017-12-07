#pragma once

#include "CommonLib/Singleton.h"

#include "Scene.h"

namespace EastEngine
{
	class SSceneMgr : public Singleton<SSceneMgr>
	{
		friend Singleton<SSceneMgr>;
	private:
		SSceneMgr();
		virtual ~SSceneMgr();

	public:
		bool Init();
		void Release();

		void Update(float fElapsedTime);

		void AddScene(SceneInterface* pScene);
		void DelScene(SceneInterface* pScene);
		void DelScene(String::StringID strSceneName);
		
		void ChangeScene(String::StringID strSceneName);

		SceneInterface* GetCurScene() { return m_pCurScene; }
		SceneInterface* GetScene(String::StringID strSceneName);

	private:
		std::unordered_map<String::StringID, SceneInterface*> m_umapScene;
		SceneInterface* m_pCurScene;
		SceneInterface* m_pChangeScene;

		bool m_isInit;
	};
}