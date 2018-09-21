#pragma once

#include "Engine/SceneManager.h"

class SceneNewStudio : public eastengine::IScene
{
public:
	SceneNewStudio();
	virtual ~SceneNewStudio();

public:
	virtual void Enter() override;
	virtual void Exit() override;

	virtual void Update(float fElapsedTime) override;

private:
	void ProcessInput(float fElapsedTime);
	void RenderImGui(float fElapsedTime);

private:

};

