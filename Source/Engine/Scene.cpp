#include "stdafx.h"
#include "Scene.h"

#include "SceneMgr.h"

#include "GameObject/SectorMgr.h"

namespace EastEngine
{
	SceneInterface::SceneInterface(String::StringID strName)
		: m_strName(strName)
		, m_pSceneMgr(nullptr)
		, m_pSectorMgr(nullptr)
	{
	}

	SceneInterface::~SceneInterface()
	{
		SafeReleaseDelete(m_pSectorMgr);
	}

	GameObject::SectorMgr* SceneInterface::CreateSectorMgr(GameObject::SectorInitInfo& sectorInitInfo)
	{
		if (m_pSectorMgr != nullptr)
			return m_pSectorMgr;

		GameObject::SectorMgr* pNewSectorMgr = new GameObject::SectorMgr;
		if (pNewSectorMgr->Init(sectorInitInfo) == false)
		{
			SafeReleaseDelete(pNewSectorMgr);
			return nullptr;
		}

		m_pSectorMgr = pNewSectorMgr;

		return pNewSectorMgr;
	}

	void SceneInterface::ChangeScene(const char* strSceneName)
	{
		m_pSceneMgr->ChangeScene(strSceneName);
	}
}