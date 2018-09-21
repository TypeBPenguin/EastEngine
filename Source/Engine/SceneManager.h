#pragma once

#include "CommonLib/Singleton.h"

#include "Scene.h"

namespace eastengine
{
	class SceneManager : public Singleton<SceneManager>
	{
		friend Singleton<SceneManager>;
	private:
		SceneManager();
		virtual ~SceneManager();

	public:
		void Flush();
		void Update(float fElapsedTime);

	public:
		void AddScene(IScene* pScene);
		void RemoveScene(IScene* pScene);
		void RemoveScene(const String::StringID& strSceneName);
		
		void ChangeScene(const String::StringID& strSceneName);

		IScene* GetScene(const String::StringID& strSceneName);

	private:
		class Impl;
		std::unique_ptr<Impl> m_pImpl;
	};
}