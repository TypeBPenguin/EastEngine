#include "stdafx.h"
#include "SkyManager.h"

namespace EastEngine
{
	namespace GameObject
	{
		SkyManager::SkyManager()
		{
		}

		SkyManager::~SkyManager()
		{
		}

		void SkyManager::Release()
		{
		}

		void SkyManager::Update(float fElapsedTime)
		{
			auto iter = m_colonySkybox.begin();
			auto iter_end = m_colonySkybox.end();
			while (iter != iter_end)
			{
				Skybox& skybox = *iter;

				if (skybox.IsDestroy() == true)
				{
					iter = m_colonySkybox.erase(iter);
					continue;
				}

				skybox.Update(fElapsedTime);

				++iter;
			}
		}

		ISkybox* SkyManager::CreateSkybox(const String::StringID& strName, const SkyboxProperty& proprety)
		{
			auto iter = m_colonySkybox.emplace();
			iter->Init(proprety);
			iter->SetName(strName);

			return &(*iter);
		}

		ISkybox* SkyManager::GetSkybox(size_t nIndex)
		{
			auto iter = m_colonySkybox.begin();
			m_colonySkybox.advance(iter, nIndex);

			return &(*iter);
		}
	}
}