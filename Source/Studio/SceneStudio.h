#pragma once

#include "Engine/SceneManager.h"

class SkeletonController;
class MaterialNodeManager;

namespace est
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

class SceneStudio : public est::IScene
{
public:
	SceneStudio();
	virtual ~SceneStudio();

	virtual void Enter() override;
	virtual void Exit() override;

	virtual void Update(float elapsedTime) override;

private:
	void ProcessInput(float elapsedTime);

private:
	void RenderUI();
	void ShowConfig();

private:
	SkeletonController* m_pSkeletonController;
	MaterialNodeManager* m_pMaterialNodeManager;
	est::gameobject::SectorMgr* m_pSectorMgr;

	std::vector<Contents::Sun*> m_vecSuns;
	est::gameobject::ISkybox* m_pSkybox{ nullptr };
};