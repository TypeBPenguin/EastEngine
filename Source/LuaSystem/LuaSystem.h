#pragma once

#include "CommonLib/Singleton.h"

#include "LuaThread.h"

struct lua_State;

namespace EastEngine
{
	namespace Lua
	{
		class LuaSystem : public Singleton<LuaSystem>
		{
			friend Singleton<LuaSystem>;
		private:
			LuaSystem();
			virtual ~LuaSystem();

		public:
			bool Init(bool isEnableJIT);
			void Release();

			void SetEnableJIT(bool isEnableJIT);

			bool CompileLua(const char* strFile);

			std::shared_ptr<LuaThread> GetThread();

		private:
			lua_State* m_pLuaState;

			std::vector<std::shared_ptr<LuaThread>> m_vecLuaThread;

			std::unordered_map<String::StringID, std::string> m_umapCompiledLua;

			size_t m_nIndex;

			bool m_isInit;
		};
	}
}