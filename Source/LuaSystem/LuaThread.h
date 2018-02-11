#pragma once

struct lua_State;

namespace EastEngine
{
	namespace Lua
	{
		class LuaThread
		{
		public:
			LuaThread(lua_State* pLuaState);
			~LuaThread();

		public:
			bool IsIdle() const;
			void SetIdle(bool isIdle);

		public:
			void PushInt(intptr_t nValue);
			void PushDouble(double dValue);
			void PushString(const String::StringID& sValue);
			void PushBool(bool bValue);

		public:
			intptr_t PopInt(size_t nIndex);
			double PopDouble(size_t nIndex);
			const String::StringID& PopString(size_t nIndex);
			bool PopBool(size_t nIndex);

		public:
			bool Run(const String::StringID& strFuncName, const int nReturnValueCount);

		private:
			class Impl;
			std::unique_ptr<Impl> m_pImpl;
		};
	}
}