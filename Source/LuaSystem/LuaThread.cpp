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

namespace eastengine
{
	namespace Lua
	{
		class LuaThread::Impl
		{
		public:
			Impl();
			~Impl();

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
			lua_State* m_pLuaState;

			bool m_isIdle;

			enum
			{
				eInt = 0,
				eDouble,
				eString,
				eBool,

				TypeCount,
			};

			const std::array<const String::StringID, TypeCount> TypeToString =
			{
				"Int",
				"Double",
				"String",
				"Bool",
			};

			std::queue<std::variant<intptr_t, double, String::StringID, bool>> m_queueInputValue;
			std::vector<std::variant<intptr_t, double, String::StringID, bool>> m_vecOutputValue;
		};

		LuaThread::Impl::Impl()
		{
		}

		LuaThread::Impl::~Impl()
		{
		}

		bool LuaThread::Impl::IsIdle() const
		{
			return m_isIdle;
		}

		void LuaThread::Impl::SetIdle(bool isIdle)
		{
			m_isIdle = isIdle;
		}

		void LuaThread::Impl::PushInt(intptr_t nValue)
		{
			m_queueInputValue.emplace(static_cast<intptr_t>(nValue));
		}

		void LuaThread::Impl::PushDouble(double dValue)
		{
			m_queueInputValue.emplace(dValue);
		}

		void LuaThread::Impl::PushString(const String::StringID& sValue)
		{
			m_queueInputValue.emplace(sValue);
		}

		void LuaThread::Impl::PushBool(bool bValue)
		{
			m_queueInputValue.emplace(bValue);
		}

		intptr_t LuaThread::Impl::PopInt(size_t nIndex)
		{
			return static_cast<intptr_t>(PopDouble(nIndex));
		}

		double LuaThread::Impl::PopDouble(size_t nIndex)
		{
			if (nIndex >= m_vecOutputValue.size())
			{
				LOG_WARNING("인덱스 범위 초과 : BufferSize[%Iu] <= Index[%Iu]", m_vecOutputValue.size(), nIndex);
				return 0;
			}

			if (m_vecOutputValue[nIndex].index() != eDouble)
			{
				LOG_WARNING("잘못 된 변수 요청 : ValueType is %s", TypeToString[m_vecOutputValue[nIndex].index()].c_str());
				return 0;
			}

			return std::get<double>(m_vecOutputValue[nIndex]);
		}

		const String::StringID& LuaThread::Impl::PopString(size_t nIndex)
		{
			if (nIndex >= m_vecOutputValue.size())
			{
				LOG_WARNING("인덱스 범위 초과 : BufferSize[%Iu] <= Index[%Iu]", m_vecOutputValue.size(), nIndex);
				return 0;
			}

			if (m_vecOutputValue[nIndex].index() != eString)
			{
				LOG_WARNING("잘못 된 변수 요청 : ValueType is %s", TypeToString[m_vecOutputValue[nIndex].index()].c_str());
				return 0;
			}

			return std::get<String::StringID>(m_vecOutputValue[nIndex]);
		}

		bool LuaThread::Impl::PopBool(size_t nIndex)
		{
			if (nIndex >= m_vecOutputValue.size())
			{
				LOG_WARNING("인덱스 범위 초과 : BufferSize[%Iu] <= Index[%Iu]", m_vecOutputValue.size(), nIndex);
				return 0;
			}

			if (m_vecOutputValue[nIndex].index() != eBool)
			{
				LOG_WARNING("잘못 된 변수 요청 : ValueType is %s", TypeToString[m_vecOutputValue[nIndex].index()].c_str());
				return 0;
			}

			return std::get<bool>(m_vecOutputValue[nIndex]);
		}

		bool LuaThread::Impl::Run(const String::StringID& strFuncName, const int nReturnValueCount)
		{
			lua_getglobal(m_pLuaState, strFuncName.c_str());

			m_vecOutputValue.reserve(nReturnValueCount);

			int nArgs = static_cast<int>(m_queueInputValue.size());

			while (m_queueInputValue.empty() == false)
			{
				std::variant<intptr_t, double, String::StringID, bool>& variantValue = m_queueInputValue.front();

				switch (variantValue.index())
				{
				case eInt:
					lua_pushinteger(m_pLuaState, std::get<intptr_t>(variantValue));
					break;
				case eDouble:
					lua_pushnumber(m_pLuaState, std::get<double>(variantValue));
					break;
				case eString:
					lua_pushstring(m_pLuaState, std::get<String::StringID>(variantValue).c_str());
					break;
				case eBool:
					lua_pushboolean(m_pLuaState, std::get<bool>(variantValue));
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

		LuaThread::LuaThread(lua_State* pLuaState)
			: m_pImpl{ std::make_unique<Impl>() }
		{
		}

		LuaThread::~LuaThread()
		{
		}

		bool LuaThread::IsIdle() const
		{
			return m_pImpl->IsIdle();
		}

		void LuaThread::SetIdle(bool isIdle)
		{
			return m_pImpl->SetIdle(isIdle);
		}

		void LuaThread::PushInt(intptr_t nValue)
		{
			m_pImpl->PushInt(nValue);
		}

		void LuaThread::PushDouble(double dValue)
		{
			m_pImpl->PushDouble(dValue);
		}

		void LuaThread::PushString(const String::StringID& sValue)
		{
			m_pImpl->PushString(sValue);
		}

		void LuaThread::PushBool(bool bValue)
		{
			m_pImpl->PushBool(bValue);
		}

		intptr_t LuaThread::PopInt(size_t nIndex)
		{
			return m_pImpl->PopInt(nIndex);
		}

		double LuaThread::PopDouble(size_t nIndex)
		{
			return m_pImpl->PopDouble(nIndex);
		}

		const String::StringID& LuaThread::PopString(size_t nIndex)
		{
			return m_pImpl->PopString(nIndex);
		}

		bool LuaThread::PopBool(size_t nIndex)
		{
			return m_pImpl->PopBool(nIndex);
		}

		bool LuaThread::Run(const String::StringID& strFuncName, const int nReturnValueCount)
		{
			return m_pImpl->Run(strFuncName, nReturnValueCount);
		}
	}
}