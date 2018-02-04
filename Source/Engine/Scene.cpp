#include "stdafx.h"
#include "Scene.h"

#include "SceneManager.h"

#include "GameObject/SectorMgr.h"

namespace EastEngine
{
	IScene::IScene(const String::StringID& strName)
		: m_strName(strName)
		, m_pSectorMgr(nullptr)
	{
	}

	IScene::~IScene()
	{
		SafeReleaseDelete(m_pSectorMgr);
	}

	GameObject::SectorMgr* IScene::CreateSectorMgr(GameObject::SectorInitInfo& sectorInitInfo)
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
}