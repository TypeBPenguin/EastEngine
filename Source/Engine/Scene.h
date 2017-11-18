#pragma once

#include "GameObject/SectorMgr.h"

namespace EastEngine
{
	class SSceneMgr;

	class SceneInterface
	{
	public:
		SceneInterface(String::StringID strName);
		virtual ~SceneInterface() = 0;

		virtual void Enter() = 0;
		virtual void Exit() = 0;

		virtual void Update(float fElapsedTime) = 0;

		void ChangeScene(const char* strSceneName);
		void SetSceneMgr(SSceneMgr* pSceneMgr) { if (pSceneMgr == nullptr) return; m_pSceneMgr = pSceneMgr; }

	protected:
		GameObject::SectorMgr* CreateSectorMgr(GameObject::SectorInitInfo& sectorInitInfo);

	public:
		String::StringID GetSceneName() { return m_strName; }

	protected:
		SSceneMgr* m_pSceneMgr;

	private:
		String::StringID m_strName;
		GameObject::SectorMgr* m_pSectorMgr;
	};
}