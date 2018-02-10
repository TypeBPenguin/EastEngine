#pragma once

#include "GameObject/SectorMgr.h"

namespace EastEngine
{
	class SceneManager;

	class IScene
	{
	public:
		IScene(const String::StringID& strName);
		virtual ~IScene() = 0;

		virtual void Enter() = 0;
		virtual void Exit() = 0;

		virtual void Update(float fElapsedTime) = 0;

	protected:
		GameObject::SectorMgr* CreateSectorMgr(GameObject::SectorInitInfo& sectorInitInfo);

	public:
		const String::StringID& GetSceneName() const { return m_strName; }

	private:
		String::StringID m_strName;
		GameObject::SectorMgr* m_pSectorMgr;
	};
}