#include "stdafx.h"
#include "SkyManager.h"

#include "CommonLib/plf_colony.h"

#include "Skybox.h"

namespace eastengine
{
	namespace gameobject
	{
		class SkyManager::Impl
		{
		public:
			Impl();
			~Impl();

		public:
			void Update(float fElapsedTime);

		public:
			ISkybox * CreateSkybox(const String::StringID& strName, const SkyboxProperty& proprety);

			ISkybox* GetSkybox(size_t nIndex);
			size_t GetSkyboxCount() const;

		private:
			plf::colony<Skybox> m_colonySkybox;
		};

		SkyManager::Impl::Impl()
		{
			m_colonySkybox.reserve(16);
		}

		SkyManager::Impl::~Impl()
		{
		}

		void SkyManager::Impl::Update(float fElapsedTime)
		{
			TRACER_EVENT("SkyManager::Update");
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

		ISkybox* SkyManager::Impl::CreateSkybox(const String::StringID& strName, const SkyboxProperty& proprety)
		{
			auto iter = m_colonySkybox.emplace();
			iter->Init(proprety);
			iter->SetName(strName);

			return &(*iter);
		}

		ISkybox* SkyManager::Impl::GetSkybox(size_t nIndex)
		{
			auto iter = m_colonySkybox.begin();
			m_colonySkybox.advance(iter, nIndex);

			return &(*iter);
		}

		size_t SkyManager::Impl::GetSkyboxCount() const
		{
			return m_colonySkybox.size();
		}

		SkyManager::SkyManager()
			: m_pImpl{ std::make_unique<Impl>() }
		{
		}

		SkyManager::~SkyManager()
		{
		}

		void SkyManager::Update(float fElapsedTime)
		{
			m_pImpl->Update(fElapsedTime);
		}

		ISkybox* SkyManager::CreateSkybox(const String::StringID& strName, const SkyboxProperty& property)
		{
			return m_pImpl->CreateSkybox(strName, property);
		}

		ISkybox* SkyManager::GetSkybox(size_t nIndex)
		{
			return m_pImpl->GetSkybox(nIndex);
		}

		size_t SkyManager::GetSkyboxCount() const
		{
			return m_pImpl->GetSkyboxCount();
		}
	}
}