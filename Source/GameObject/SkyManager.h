#pragma once

#include "CommonLib/Singleton.h"
#include "CommonLib/plf_colony.h"

#include "Skybox.h"

namespace EastEngine
{
	namespace GameObject
	{
		class SkyManager : public Singleton<SkyManager>
		{
			friend Singleton<SkyManager>;
		private:
			SkyManager();
			virtual ~SkyManager();

		public:
			void Release();

			void Update(float fElapsedTime);

		public:
			ISkybox* CreateSkybox(const String::StringID& strName, const SkyboxProperty& proprety);

			ISkybox* GetSkybox(uint32_t nIndex);
			uint32_t GetSkyboxCount() const { return m_colonySkybox.size(); }

		private:
			plf::colony<Skybox> m_colonySkybox;
		};

	}
}