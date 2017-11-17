#include "stdafx.h"
#include "LuaThread.h"

namespace EastEngine
{
	namespace Lua
	{
		LuaThread::LuaThread(lua_State* pLuaState)
			: m_pLuaState(pLuaState)
			, m_isIdle(false)
		{
		}

		LuaThread::~LuaThread()
		{
			m_pLuaState = nullptr;
		}

		void LuaThread::PushInt(int* nValue)
		{
			if (nValue == nullptr)
				return;

			LuaIOValue value;
			value.emValueType = VT_INT;
			value.pValue = nValue;

			m_queueInputValue.push(value);
		}

		void LuaThread::PushFloat(float* fValue)
		{
			if (fValue == nullptr)
				return;

			LuaIOValue value;
			value.emValueType = VT_FLOAT;
			value.pValue = fValue;

			m_queueInputValue.push(value);
		}

		void LuaThread::PushString(char* sValue)
		{
			if (sValue == nullptr)
				return;

			LuaIOValue value;
			value.emValueType = VT_STRING;
			value.pValue = sValue;

			m_queueInputValue.push(value);
		}

		void LuaThread::PushBool(bool* bValue)
		{
			if (bValue == nullptr)
				return;

			LuaIOValue value;
			value.emValueType = VT_BOOL;
			value.pValue = bValue;

			m_queueInputValue.push(value);
		}

		void LuaThread::PopInt(int* nValue)
		{
			if (nValue == nullptr)
				return;

			LuaIOValue value;
			value.emValueType = VT_INT;
			value.pValue = nValue;

			m_stackOutputValue.push(value);
		}

		void LuaThread::PopFloat(float* fValue)
		{
			if (fValue == nullptr)
				return;

			LuaIOValue value;
			value.emValueType = VT_FLOAT;
			value.pValue = fValue;

			m_stackOutputValue.push(value);
		}

		void LuaThread::PopString(String::StringID* sValue)
		{
			if (sValue == nullptr)
				return;

			LuaIOValue value;
			value.emValueType = VT_STRING;
			value.pValue = sValue;

			m_stackOutputValue.push(value);
		}

		void LuaThread::PopBool(bool* bValue)
		{
			if (bValue == nullptr)
				return;

			LuaIOValue value;
			value.emValueType = VT_BOOL;
			value.pValue = bValue;

			m_stackOutputValue.push(value);
		}

		void LuaThread::Run(const String::StringID& strFuncName)
		{
			lua_getglobal(m_pLuaState, strFuncName.c_str());

			int nArgs = (int)(m_queueInputValue.size());
			int nReturn = (int)(m_stackOutputValue.size());

			while (m_queueInputValue.empty() == false)
			{
				LuaIOValue& value = m_queueInputValue.front();

				switch (value.emValueType)
				{
				case VT_INT:
				{
					int nValue = *reinterpret_cast<int*>(value.pValue);
					lua_pushinteger(m_pLuaState, nValue);
				}
				break;
				case VT_FLOAT:
				{
					float fValue = *reinterpret_cast<float*>(value.pValue);
					lua_pushnumber(m_pLuaState, fValue);
				}
				break;
				case VT_STRING:
				{
					char* sValue = reinterpret_cast<char*>(value.pValue);
					lua_pushstring(m_pLuaState, sValue);
				}
				break;
				case VT_BOOL:
				{
					bool bValue = *reinterpret_cast<bool*>(value.pValue);
					lua_pushboolean(m_pLuaState, bValue);
				}
				break;
				}

				m_queueInputValue.pop();
			}

			if (lua_pcall(m_pLuaState, nArgs, nReturn, 0) != 0)
			{
				PRINT_LOG("Can't Running Function : %s", lua_tostring(m_pLuaState, -1));

				while (m_queueInputValue.empty() == false)
				{
					m_queueInputValue.pop();
				}

				while (m_stackOutputValue.empty() == false)
				{
					m_stackOutputValue.pop();
				}

				return;
			}

			while (m_stackOutputValue.empty() == false)
			{
				LuaIOValue& value = m_stackOutputValue.top();

				switch (value.emValueType)
				{
				case VT_INT:
				{
					int* nValue = reinterpret_cast<int*>(value.pValue);
					*nValue = lua_tointeger(m_pLuaState, -1);
				}
				break;
				case VT_FLOAT:
				{
					float* fValue = reinterpret_cast<float*>(value.pValue);
					*fValue = (float)(lua_tonumber(m_pLuaState, -1));
				}
				break;
				case VT_STRING:
				{
					String::StringID* sValue = reinterpret_cast<String::StringID*>(value.pValue);
					*sValue = lua_tostring(m_pLuaState, -1);
				}
				break;
				case VT_BOOL:
				{
					bool* bValue = reinterpret_cast<bool*>(value.pValue);
					*bValue = lua_toboolean(m_pLuaState, -1) != 0;
				}
				break;
				}

				lua_pop(m_pLuaState, 1);

				m_stackOutputValue.pop();
			}

			m_isIdle = true;
		}
	}
}