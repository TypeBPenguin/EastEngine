#pragma once

#include "Engine/SceneManager.h"

class SkeletonController;
class MaterialNodeManager;

namespace eastengine
{
	namespace gameobject
	{
		class SectorMgr;
		class ISkybox;
	}
}

namespace Contents
{
	class Sun;
}

class SceneStudio : public eastengine::IScene
{
public:
	SceneStudio();
	virtual ~SceneStudio();

	virtual void Enter() override;
	virtual void Exit() override;

	virtual void Update(float fElapsedTime) override;

private:
	void ProcessInput(float fElapsedTime);

private:
	void RenderUI();
	void ShowConfig();

private:
	SkeletonController* m_pSkeletonController;
	MaterialNodeManager* m_pMaterialNodeManager;
	eastengine::gameobject::SectorMgr* m_pSectorMgr;

	std::vector<Contents::Sun*> m_vecSuns;
	eastengine::gameobject::ISkybox* m_pSkybox{ nullptr };
};