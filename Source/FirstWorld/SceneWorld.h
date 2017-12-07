#pragma once

#include "Engine/SceneMgr.h"

class SceneWorld : public EastEngine::SceneInterface
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