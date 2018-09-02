#include "stdafx.h"
#include "GameObject.h"

#include "CommonLib/FileStream.h"

#include "Actor.h"
#include "Terrain.h"
#include "Skybox.h"

#include "GameObjectManager.h"

namespace eastengine
{
	namespace gameobject
	{
		IGameObject::IGameObject(const Handle& handle)
			: m_handle(handle)
		{
		}

		IActor::IActor(const Handle& handle)
			: IGameObject(handle)
		{
		}

		IActor* IActor::CreateByFile(const char* strFilePath)
		{
			file::Stream file;
			if (file.Open(strFilePath, file::eRead | file::eBinary) == false)
			{
				LOG_WARNING("Can't open to file : %s", strFilePath);
				return nullptr;
			}

			std::string strBuf;
			file >> strBuf;

			IActor* pActor = GameObjectManager::GetInstance()->CreateActor(strBuf.c_str());

			uint32_t nComponentSize = 0;
			file >> nComponentSize;

			for (uint32_t i = 0; i < nComponentSize; ++i)
			{
				strBuf.clear();
				file >> strBuf;

				EmComponent::Type emType = EmComponent::GetType(strBuf.c_str());
				if (emType != EmComponent::TypeCount)
				{
					IComponent* pComponent = pActor->CreateComponent(emType);
					pComponent->LoadToFile(file);
				}
			}

			file.Close();

			return pActor;
		}

		IActor* IActor::Create(const String::StringID& strName)
		{
			return GameObjectManager::GetInstance()->CreateActor(strName);
		}

		void IActor::Destroy(IActor** ppActor)
		{
			if (ppActor == nullptr || *ppActor == nullptr)
				return;

			Actor* pActor = static_cast<Actor*>(*ppActor);
			pActor->SetDestroy(true);

			*ppActor = nullptr;
		}

		bool IActor::SaveToFile(IActor* pActor, const char* strFilePath)
		{
			file::Stream file;
			if (file.Open(strFilePath, file::eWrite | file::eBinary) == false)
			{
				LOG_WARNING("Can't open to file : %s", strFilePath);
				return false;
			}

			file << pActor->GetName().c_str();

			uint32_t nCount = 0;
			for (int i = 0; i < EmComponent::TypeCount; ++i)
			{
				IComponent* pComponent = pActor->GetComponent(static_cast<EmComponent::Type>(i));
				if (pComponent != nullptr)
				{
					++nCount;
				}
			}

			file << nCount;

			for (int i = 0; i < EmComponent::TypeCount; ++i)
			{
				IComponent* pComponent = pActor->GetComponent(static_cast<EmComponent::Type>(i));
				if (pComponent != nullptr)
				{
					file << EmComponent::ToString(pComponent->GetComponentType());

					pComponent->SaveToFile(file);
				}
			}

			file.Close();

			return true;
		}

		ITerrain::ITerrain(const Handle& handle)
			: IGameObject(handle)
		{
		}

		ITerrain* ITerrain::Create(const String::StringID& strName, const TerrainProperty& terrainProperty)
		{
			return GameObjectManager::GetInstance()->CreateTerrain(strName, terrainProperty);
		}

		ITerrain* ITerrain::CreateAsync(const String::StringID& strName, const TerrainProperty& terrainProperty)
		{
			return GameObjectManager::GetInstance()->CreateTerrainAsync(strName, terrainProperty);
		}

		void ITerrain::Destroy(ITerrain** ppTerrain)
		{
			if (ppTerrain == nullptr || *ppTerrain == nullptr)
				return;

			Terrain* pTerrain = static_cast<Terrain*>(*ppTerrain);
			pTerrain->SetDestroy(true);

			*ppTerrain = nullptr;
		}

		ISkybox::ISkybox(const Handle& handle)
			: IGameObject(handle)
		{
		}

		ISkybox* ISkybox::Create(const String::StringID& strName, const SkyboxProperty& property)
		{
			return GameObjectManager::GetInstance()->CreateSkybox(strName, property);
		}

		void ISkybox::Destroy(ISkybox** ppSkybox)
		{
			if (ppSkybox == nullptr || *ppSkybox == nullptr)
				return;

			Skybox* pSkybox = static_cast<Skybox*>(*ppSkybox);
			pSkybox->SetDestroy(true);

			*ppSkybox = nullptr;
		}
	}
}