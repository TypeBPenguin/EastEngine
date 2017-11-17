#pragma once

#include "../CommonLib/Singleton.h"

#include "EffectInterface.h"

namespace EastEngine
{
	namespace Graphics
	{
		class ShaderManager : public Singleton<ShaderManager>
		{
			friend Singleton<ShaderManager>;
		private:
			ShaderManager();
			virtual ~ShaderManager();

		public:
			bool Init();
			void Release();

			static void CALLBACK DirectoryMonitorCallback(const char* strPath, DWORD dwAction, LPARAM lParam);

		public:
			IEffect* GetEffect(const String::StringID& strName);
			bool AddEffect(IEffect* pEffect);
			void RemoveEffect(IEffect* pEffect);

		private:
			bool m_isInit;
			boost::unordered_map<String::StringID, IEffect*> m_umapEffects;
		};
	}
}