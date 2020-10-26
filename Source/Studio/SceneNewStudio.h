#pragma once

#include "Engine/SceneManager.h"

namespace est
{
	namespace gameobject
	{
		class ComponentModel;
	}

	namespace graphics
	{
		class IMaterial;
	}
}

class Minion;

class SceneNewStudio : public est::IScene
{
public:
	SceneNewStudio();
	virtual ~SceneNewStudio();

	static const est::string::StringID Name;

public:
	virtual void Enter(const std::queue<est::gameobject::ActorPtr>& savedPrevSceneActors) override;
	virtual void Exit(std::queue<est::gameobject::ActorPtr>& saveSceneActors_out) override;

	virtual void Update(float elapsedTime) override;

private:
	void ProcessInput(float elapsedTime);
	void RenderImGui(float elapsedTime);

	void ShowConfig();
	void ShowMotion(bool& isShowMotionMenu, est::gameobject::ComponentModel* pCompModel);
	void ShowMaterial(bool& isShowMaterial, est::graphics::IMaterial* pMaterial, int index);
	void ShowSoundWindow(bool& isShowSoundMenu);
	void ShowActorWindow(bool& isShowActorMenu);
	void ShowGizmo();
	void ShowNodeEditer();

private:
	std::unique_ptr<Minion> m_pMinion;

	std::vector<est::graphics::LightPtr> m_pLights;

	std::vector<est::gameobject::ActorPtr> m_actors;
	std::vector<est::gameobject::TerrainPtr> m_terrains;
	std::vector<est::gameobject::SkyboxPtr> m_skyboxs;

	est::gameobject::IGameObject::Handle m_selectedActor;
};