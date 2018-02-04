#pragma once

#include "Engine/SceneManager.h"

class SceneWorld : public EastEngine::IScene
{
public:
	SceneWorld(const EastEngine::String::StringID& strSceneName);
	virtual ~SceneWorld();

public:
	virtual void Enter() override;
	virtual void Exit() override;

	virtual void Update(float fElapsedTime) override;

private:
};