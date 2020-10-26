#pragma once

#include "CommonLib/Singleton.h"

#include "Scene.h"

namespace est
{
	class SceneManager : public Singleton<SceneManager>
	{
		friend Singleton<SceneManager>;
	private:
		SceneManager();
		virtual ~SceneManager();

	public:
		void Update(float elapsedTime);

	public:
		void AddScene(std::unique_ptr<IScene>&& pScene);
		void RemoveScene(const string::StringID& sceneName);
		
		void ChangeScene(const string::StringID& sceneName);

		IScene* GetScene(const string::StringID& sceneName) const;

	private:
		class Impl;
		std::unique_ptr<Impl> m_pImpl;
	};
}