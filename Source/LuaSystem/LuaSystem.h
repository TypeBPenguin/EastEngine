#pragma once

#include "CommonLib/Singleton.h"

#include "LuaThread.h"

namespace eastengine
{
	namespace Lua
	{
		class System : public Singleton<System>
		{
			friend Singleton<System>;
		private:
			System();
			virtual ~System();

		public:
			bool Initialize(bool isEnableJIT);

			void SetEnableJIT(bool isEnableJIT);

			bool CompileLua(const char* strFile);

			std::shared_ptr<LuaThread> GetThread();

		private:
			class Impl;
			std::unique_ptr<Impl> m_pImpl;
		};
	}
}