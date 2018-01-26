#pragma once

#include "Engine/SceneMgr.h"

class SkeletonController;
class MaterialNodeManager;

namespace EastEngine
{
	namespace GameObject
	{
		class SectorMgr;
		class ISkybox;
	}
}

namespace Contents
{
	class Sun;
}

class SceneStudio : public EastEngine::SceneInterface
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
	EastEngine::GameObject::SectorMgr* m_pSectorMgr;

	std::vector<Contents::Sun*> m_vecSuns;
	EastEngine::GameObject::ISkybox* m_pSkybox{ nullptr };
};