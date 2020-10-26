#include "stdafx.h"
#include "LuaSystem.h"

namespace est
{
	namespace Lua
	{
		class System::Impl
		{
		public:
			Impl();
			~Impl();

		public:
			bool Initialize(bool isEnableJIT);

			void SetEnableJIT(bool isEnableJIT);

			bool CompileLua(const wchar_t* filePath);

		private:
			bool m_isInitialized{ false };
			lua_State* m_pLuaState{ nullptr };

			std::unordered_map<string::StringID, std::string> m_umapCompiledLua;

			size_t m_nIndex{ 0 };
		};

		System::Impl::Impl()
		{
		}

		System::Impl::~Impl()
		{
			lua_close(m_pLuaState);
		}

		bool System::Impl::Initialize(bool isEnableJIT)
		{
			if (m_isInitialized == true)
				return true;

			// Lua state 생성
			m_pLuaState = luaL_newstate();

			// 표준 라이브러리 로드
			luaL_openlibs(m_pLuaState);

			SetEnableJIT(isEnableJIT);

			m_isInitialized = true;

			return true;
		}

		void System::Impl::SetEnableJIT(bool isEnableJIT)
		{
			if (isEnableJIT == true)
			{
				luaJIT_setmode(m_pLuaState, 0, LUAJIT_MODE_ENGINE | LUAJIT_MODE_ON);
			}
			else
			{
				luaJIT_setmode(m_pLuaState, 0, LUAJIT_MODE_ENGINE | LUAJIT_MODE_OFF);
			}
		}

		bool System::Impl::CompileLua(const wchar_t* filePath)
		{
			const std::string multiPath = string::WideToMulti(filePath);
			int nError = luaL_loadfile(m_pLuaState, multiPath.c_str());
			if (nError != 0)
			{
				LOG_ERROR(L"Can't Load Lua Script : %s", filePath);
				return false;
			}

			//luaL_dofile(m_pLuaState, filePath);

			/*StringID filePathName = Alofile::GetFileName(filePath);
			auto iter = m_umapCompiledLua.find(filePathName);
			if (iter != m_umapCompiledLua.end())
			{
			LOG_ERROR(L"Already compiled file : %s", filePathName.c_str());
			return false;
			}

			std::string strCompile;
			int nError = luaL_loadfile(m_pLuaState, filePath);
			if (nError != 0)
			{
			LOG_ERROR(L"Can't Load Lua Script : %s", filePath);
			return false;
			}

			auto DumpWriter = [](lua_State* pLuaState, const void* buf, size_t size, void* lua_buf)
			{
			luaL_addlstring(static_cast<luaL_Buffer*>(lua_buf), static_cast<const char*>(buf), size);
			return 0;
			};

			int nTop = lua_gettop(m_pLuaState);

			luaL_Buffer buf;
			luaL_buffinit(m_pLuaState, &buf);
			lua_dump(m_pLuaState, DumpWriter, &buf);
			luaL_pushresult(&buf);

			strCompile = std::string(lua_tostring(m_pLuaState, -1), lua_strlen(m_pLuaState, -1));
			lua_settop(m_pLuaState, nTop);

			m_umapCompiledLua.insert(std::make_pair(StringID(filePathName.c_str()), strCompile));*/

			return true;
		}

		System::System()
			: m_pImpl{ std::make_unique<Impl>() }
		{
		}

		System::~System()
		{
		}

		bool System::Initialize(bool isEnableJIT)
		{
			return m_pImpl->Initialize(isEnableJIT);
		}

		void System::SetEnableJIT(bool isEnableJIT)
		{
			m_pImpl->SetEnableJIT(isEnableJIT);
		}

		bool System::CompileLua(const wchar_t* filePath)
		{
			return m_pImpl->CompileLua(filePath);
		}
	}
}