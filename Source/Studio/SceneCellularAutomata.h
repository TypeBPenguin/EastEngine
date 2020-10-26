#pragma once

#include "Engine/SceneManager.h"

class SceneCellularAutomata : public est::IScene
{
public:
	SceneCellularAutomata();
	virtual ~SceneCellularAutomata();

public:
	virtual void Enter(const std::queue<est::gameobject::ActorPtr>& savedPrevSceneActors) override;
	virtual void Exit(std::queue<est::gameobject::ActorPtr>& saveSceneActors_out) override;

	virtual void Update(float elapsedTime) override;
};