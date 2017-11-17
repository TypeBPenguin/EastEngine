#pragma once

#include "../Engine/SceneMgr.h"

class MaterialNodeManager;

namespace EastEngine
{
	namespace GameObject
	{
		class SectorMgr;
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

private:
	MaterialNodeManager* m_pMaterialNodeManager;
	EastEngine::GameObject::SectorMgr* m_pSectorMgr;

	std::vector<Contents::Sun*> m_vecSuns;
};