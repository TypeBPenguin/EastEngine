#include "stdafx.h"
#include "LuaThread.h"

namespace StrID
{
	RegisterStringID(Undefined);
	RegisterStringID(Int);
	RegisterStringID(Double);
	RegisterStringID(String);
	RegisterStringID(Bool);
}

namespace EastEngine
{
	namespace Lua
	{
		LuaThread::LuaIOValue::LuaIOValue(double value)
			: emValueType(EmValueType::eDouble)
		{
			element.emplace<double>(value);
		}

		LuaThread::LuaIOValue::LuaIOValue(const String::StringID& value)
			: emValueType(EmValueType::eString)
		{
			element.emplace<String::StringID>(value);
		}

		LuaThread::LuaIOValue::LuaIOValue(bool value)
			: emValueType(EmValueType::eBool)
		{
			element.emplace<bool>(value);
		}

		double LuaThread::LuaIOValue::ToDouble() const
		{
			if (IsDouble() == false)
				return 0.0;

			return std::get<double>(element);
		}

		const String::StringID& LuaThread::LuaIOValue::ToString() const
		{
			if (IsString() == false)
				return StrID::Unregistered;

			return std::get<String::StringID>(element);
		}

		bool LuaThread::LuaIOValue::ToBool() const
		{
			if (IsBool() == false)
				return false;

			return std::get<bool>(element);
		}

		bool LuaThread::LuaIOValue::IsDouble() const
		{
			return emValueType == EmValueType::eDouble;
		}

		bool LuaThread::LuaIOValue::IsString() const
		{
			return emValueType == EmValueType::eString;
		}

		bool LuaThread::LuaIOValue::IsBool() const
		{
			return emValueType == EmValueType::eBool;
		}

		const String::StringID& LuaThread::LuaIOValue::TypeToString() const
		{
			switch (emValueType)
			{
			case EmValueType::eDouble:
				return StrID::Double;
			case EmValueType::eString:
				return StrID::String;
			case EmValueType::eBool:
				return StrID::Bool;
			default:
				return StrID::Undefined;
			}
		}

		LuaThread::LuaThread(lua_State* pLuaState)
			: m_pLuaState(pLuaState)
			, m_isIdle(false)
		{
		}

		LuaThread::~LuaThread()
		{
			m_pLuaState = nullptr;
		}

		void LuaThread::PushInt(intptr_t nValue)
		{
			m_queueInputValue.emplace(static_cast<double>(nValue));
		}

		void LuaThread::PushFloat(float fValue)
		{
			m_queueInputValue.emplace(static_cast<double>(fValue));
		}

		void LuaThread::PushDouble(double dValue)
		{
			m_queueInputValue.emplace(dValue);
		}

		void LuaThread::PushString(const String::StringID& sValue)
		{
			m_queueInputValue.emplace(sValue);
		}

		void LuaThread::PushBool(bool bValue)
		{
			m_queueInputValue.emplace(bValue);
		}

		intptr_t LuaThread::PopInt(size_t nIndex)
		{
			return static_cast<intptr_t>(PopDouble(nIndex));
		}

		float LuaThread::PopFloat(size_t nIndex)
		{
			return static_cast<float>(PopDouble(nIndex));
		}

		double LuaThread::PopDouble(size_t nIndex)
		{
			if (nIndex >= m_vecOutputValue.size())
			{
				LOG_WARNING("인덱스 범위 초과 : BufferSize[%Iu] <= Index[%Iu]", m_vecOutputValue.size(), nIndex);
				return 0.0;
			}

			if (m_vecOutputValue[nIndex].IsDouble() == false)
			{
				LOG_WARNING("잘못 된 변수 요청 : ValueType is %s", m_vecOutputValue[nIndex].TypeToString().c_str());
				return 0.0;
			}

			return static_cast<float>(m_vecOutputValue[nIndex].ToDouble());
		}

		const String::StringID& LuaThread::PopString(size_t nIndex)
		{
			if (nIndex >= m_vecOutputValue.size())
			{
				LOG_WARNING("인덱스 범위 초과 : BufferSize[%Iu] <= Index[%Iu]", m_vecOutputValue.size(), nIndex);
				return StrID::Unregistered;
			}

			if (m_vecOutputValue[nIndex].IsString() == false)
			{
				LOG_WARNING("잘못 된 변수 요청 : ValueType is %s", m_vecOutputValue[nIndex].TypeToString().c_str());
				return StrID::Unregistered;
			}

			return m_vecOutputValue[nIndex].ToString();
		}

		bool LuaThread::PopBool(size_t nIndex)
		{
			if (nIndex >= m_vecOutputValue.size())
			{
				LOG_WARNING("인덱스 범위 초과 : BufferSize[%Iu] <= Index[%Iu]", m_vecOutputValue.size(), nIndex);
				return false;
			}

			if (m_vecOutputValue[nIndex].IsBool() == false)
			{
				LOG_WARNING("잘못 된 변수 요청 : ValueType is %s", m_vecOutputValue[nIndex].TypeToString().c_str());
				return false;
			}

			return m_vecOutputValue[nIndex].ToBool();
		}

		bool LuaThread::Run(const String::StringID& strFuncName, const int nReturnValueCount)
		{
			lua_getglobal(m_pLuaState, strFuncName.c_str());

			m_vecOutputValue.reserve(nReturnValueCount);

			int nArgs = static_cast<int>(m_queueInputValue.size());

			while (m_queueInputValue.empty() == false)
			{
				LuaIOValue& value = m_queueInputValue.front();

				switch (value.emValueType)
				{
				case LuaIOValue::EmValueType::eDouble:
				{
					lua_pushnumber(m_pLuaState, value.ToDouble());
				}
				break;
				case LuaIOValue::EmValueType::eString:
				{
					lua_pushstring(m_pLuaState, value.ToString().c_str());
				}
				break;
				case LuaIOValue::EmValueType::eBool:
				{
					lua_pushboolean(m_pLuaState, value.ToBool());
				}
				break;
				}

				m_queueInputValue.pop();
			}

			if (lua_pcall(m_pLuaState, nArgs, nReturnValueCount, 0) != 0)
			{
				LOG_WARNING("Can't Running Function : %s", lua_tostring(m_pLuaState, -1));

				while (m_queueInputValue.empty() == false)
				{
					m_queueInputValue.pop();
				}

				m_vecOutputValue.clear();

				return false;
			}

			for (int i = 0; i < nReturnValueCount; ++i)
			{
				int t = lua_type(m_pLuaState, i);
				switch (t)
				{
				case LUA_TSTRING:
				{
					String::StringID value = lua_tostring(m_pLuaState, -1);
					m_vecOutputValue.emplace_back(value);
				}
				break;
				case LUA_TBOOLEAN:
				{
					bool value = lua_toboolean(m_pLuaState, -1);
					m_vecOutputValue.emplace_back(value);
				}
				break;
				case LUA_TNUMBER:
				{
					double value = lua_tonumber(m_pLuaState, -1);
					m_vecOutputValue.emplace_back(value);
				}
				break;
				default:
					LOG_WARNING("Unknown Type Return : %s", lua_typename(m_pLuaState, t));
					break;
				}

				lua_pop(m_pLuaState, 1);
			}

			m_isIdle = true;

			return true;
		}
	}
}