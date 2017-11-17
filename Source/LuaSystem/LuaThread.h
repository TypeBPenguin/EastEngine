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
			virtual ~LuaThread();

			bool IsIdle() { return m_isIdle; }
			void SetIdle(bool bIdle) { m_isIdle = bIdle; }

			void PushInt(int* nValue);
			void PushFloat(float* fValue);
			void PushString(char* sValue);
			void PushBool(bool* bValue);

			void PopInt(int* nValue);
			void PopFloat(float* fValue);
			void PopString(String::StringID* sValue);
			void PopBool(bool* bValue);

			void Run(const String::StringID& strFuncName);

		private:
			lua_State* m_pLuaState;

			bool m_isIdle;

			enum EM_VALUE_TYPE
			{
				VT_INT = 0,
				VT_FLOAT,
				VT_STRING,
				VT_BOOL,
			};

			struct LuaIOValue
			{
				EM_VALUE_TYPE emValueType = VT_INT;
				void* pValue = nullptr;
			};

			std::queue<LuaIOValue> m_queueInputValue;
			std::stack<LuaIOValue> m_stackOutputValue;
		};
	}
}