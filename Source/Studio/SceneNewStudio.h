#pragma once

#include "Engine/SceneManager.h"

class Minion;

class SceneNewStudio : public eastengine::IScene
{
public:
	SceneNewStudio();
	virtual ~SceneNewStudio();

public:
	virtual void Enter() override;
	virtual void Exit() override;

	virtual void Update(float elapsedTime) override;

private:
	void ProcessInput(float elapsedTime);
	void RenderImGui(float elapsedTime);

private:
	std::unique_ptr<Minion> m_pMinion;
};

