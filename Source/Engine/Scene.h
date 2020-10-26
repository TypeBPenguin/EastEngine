#pragma once

#include "GameObject/GameObject.h"

namespace est
{
	class IScene
	{
	public:
		IScene(const string::StringID& name);
		virtual ~IScene() = 0;

	public:
		virtual void Enter(const std::queue<gameobject::ActorPtr>& savedPrevSceneActors) = 0;
		virtual void Exit(std::queue<gameobject::ActorPtr>& saveSceneActors_out) = 0;

		virtual void Update(float elapsedTime) = 0;

	public:
		const string::StringID& GetName() const { return m_name; }

	private:
		string::StringID m_name;
	};
}