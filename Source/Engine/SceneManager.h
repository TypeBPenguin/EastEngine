#pragma once

#include "CommonLib/Singleton.h"

#include "Scene.h"

namespace EastEngine
{
	class SceneManager : public Singleton<SceneManager>
	{
		friend Singleton<SceneManager>;
	private:
		SceneManager();
		virtual ~SceneManager();

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
		class Impl;
		std::unique_ptr<Impl> m_pImpl;
	};
}