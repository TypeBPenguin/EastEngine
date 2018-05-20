#pragma once

#include "CommonLib/Singleton.h"

#include "GameObject.h"

namespace eastengine
{
	namespace gameobject
	{
		class SkyManager : public Singleton<SkyManager>
		{
			friend Singleton<SkyManager>;
		private:
			SkyManager();
			virtual ~SkyManager();

		public:
			void Update(float fElapsedTime);

		public:
			ISkybox* CreateSkybox(const String::StringID& strName, const SkyboxProperty& property);

			ISkybox* GetSkybox(size_t nIndex);
			size_t GetSkyboxCount() const;

		private:
			class Impl;
			std::unique_ptr<Impl> m_pImpl;
		};

	}
}