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

			void PushInt(intptr_t nValue);
			void PushFloat(float fValue);
			void PushDouble(double dValue);
			void PushString(const String::StringID& sValue);
			void PushBool(bool bValue);

			intptr_t PopInt(size_t nIndex);
			float PopFloat(size_t nIndex);
			double PopDouble(size_t nIndex);
			const String::StringID& PopString(size_t nIndex);
			bool PopBool(size_t nIndex);

			bool Run(const String::StringID& strFuncName, const int nReturnValueCount);

		private:
			lua_State* m_pLuaState;

			bool m_isIdle;

			struct LuaIOValue
			{
				enum EmValueType
				{
					eUndefined = 0,
					eDouble,
					eString,
					eBool,
				};

				EmValueType emValueType = EmValueType::eUndefined;
				std::variant<intptr_t, double, String::StringID, bool> element;

				explicit LuaIOValue(double value);
				explicit LuaIOValue(const String::StringID& value);
				explicit LuaIOValue(bool value);

				double ToDouble() const;
				const String::StringID& ToString() const;
				bool ToBool() const;

				bool IsDouble() const;
				bool IsString() const;
				bool IsBool() const;

				const String::StringID& TypeToString() const;
			};

			std::queue<LuaIOValue> m_queueInputValue;
			std::vector<LuaIOValue> m_vecOutputValue;
		};
	}
}