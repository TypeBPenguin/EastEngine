#include "stdafx.h"
#include "GameObject.h"

#include "GameObjectManager.h"

namespace est
{
	namespace gameobject
	{
		void GameObjectDeleter::operator()(IGameObject* pGameObject)
		{
			switch (pGameObject->GetType())
			{
			case eActor:
				GameObjectManager::GetInstance()->RemoveActor(static_cast<IActor*>(pGameObject));
				break;
			case eTerrain:
				GameObjectManager::GetInstance()->RemoveTerrain(static_cast<ITerrain*>(pGameObject));
				break;
			case eSky:
				GameObjectManager::GetInstance()->RemoveSkybox(static_cast<ISkybox*>(pGameObject));
				break;
			default:
				LOG_ERROR(L"failed to remove gameobject, unknown object type : %d", pGameObject->GetType());
				break;
			}
		}

		IGameObject::IGameObject(const Handle& handle)
			: m_handle(handle)
		{
		}

		IActor::IActor(const Handle& handle)
			: IGameObject(handle)
		{
		}

		ITerrain::ITerrain(const Handle& handle)
			: IGameObject(handle)
		{
		}

		ISkybox::ISkybox(const Handle& handle)
			: IGameObject(handle)
		{
		}

		ActorPtr CreateActor(const string::StringID& name)
		{
			return GameObjectManager::GetInstance()->CreateActor(name);
		}

		SkyboxPtr CreateSkybox(const string::StringID& name, const SkyboxProperty& skyProperty)
		{
			return GameObjectManager::GetInstance()->CreateSkybox(name, skyProperty);
		}

		TerrainPtr CreateTerrain(const string::StringID& name, const TerrainProperty& terrainProperty)
		{
			return GameObjectManager::GetInstance()->CreateTerrain(name, terrainProperty);
		}

		TerrainPtr CreateTerrainAsync(const string::StringID& name, const TerrainProperty& terrainProperty)
		{
			return GameObjectManager::GetInstance()->CreateTerrainAsync(name, terrainProperty);
		}
	}
}