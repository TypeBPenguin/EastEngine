#include "stdafx.h"
#include "GameObject.h"

#include "CommonLib/FileStream.h"

#include "ActorManager.h"
#include "TerrainManager.h"

namespace EastEngine
{
	namespace GameObject
	{
		IActor* IActor::CreateByFile(const char* strFilePath)
		{
			File::FileStream file;
			if (file.Open(strFilePath, File::EmState::eRead | File::EmState::eBinary) == false)
			{
				PRINT_LOG("Can't open to file : %s", strFilePath);
				return nullptr;
			}

			std::string strBuf;
			file >> strBuf;

			IActor* pActor = ActorManager::GetInstance()->CreateActor(strBuf.c_str());

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
			return ActorManager::GetInstance()->CreateActor(strName);
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
			File::FileStream file;
			if (file.Open(strFilePath, File::EmState::eWrite | File::EmState::eBinary) == false)
			{
				PRINT_LOG("Can't open to file : %s", strFilePath);
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

		ITerrain* ITerrain::Create(const String::StringID& strName, const TerrainProperty* pTerrainProperty)
		{
			return TerrainManager::GetInstance()->CreateTerrain(strName, pTerrainProperty);
		}

		ITerrain* ITerrain::CreateAsync(const String::StringID& strName, const TerrainProperty* pTerrainProperty)
		{
			return TerrainManager::GetInstance()->CreateTerrainAsync(strName, pTerrainProperty);
		}

		void ITerrain::Destroy(ITerrain** ppTerrain)
		{
			if (ppTerrain == nullptr || *ppTerrain == nullptr)
				return;

			Terrain* pTerrain = static_cast<Terrain*>(*ppTerrain);
			pTerrain->SetDestroy(true);

			*ppTerrain = nullptr;
		}
	}
}