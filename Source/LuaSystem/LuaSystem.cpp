#include "stdafx.h"
#include "LuaSystem.h"

#include "LuaThread.h"

namespace EastEngine
{
	namespace Lua
	{
		LuaSystem::LuaSystem()
			: m_pLuaState(nullptr)
			, m_isInit(false)
			, m_nIndex(0)
		{
		}

		LuaSystem::~LuaSystem()
		{
			Release();
		}

		bool LuaSystem::Init(bool isEnableJIT)
		{
			if (m_isInit == true)
				return true;

			m_isInit = true;

			// Lua state 생성
			m_pLuaState = luaL_newstate();

			// 표준 라이브러리 로드
			luaL_openlibs(m_pLuaState);

			// Lua state에 LuaBind 연결
			luabind::open(m_pLuaState);

			SetEnableJIT(isEnableJIT);

			m_vecLuaThread.reserve(100);

			return true;
		}

		void LuaSystem::Release()
		{
			if (m_isInit == false)
				return;

			m_vecLuaThread.clear();

			lua_close(m_pLuaState);

			m_isInit = false;
		}

		void LuaSystem::SetEnableJIT(bool isEnableJIT)
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

		bool LuaSystem::CompileLua(const char* strFile)
		{
			int nError = luaL_loadfile(m_pLuaState, strFile);
			if (nError != 0)
			{
				LOG_ERROR("Can't Load Lua Script : %s", strFile);
				return false;
			}

			//luaL_dofile(m_pLuaState, strFile);

			/*StringID strFileName = AloFile::GetFileName(strFile);
			auto iter = m_umapCompiledLua.find(strFileName);
			if (iter != m_umapCompiledLua.end())
			{
				LOG_ERROR("Already compiled file : %s", strFileName.c_str());
				return false;
			}

			std::string strCompile;
			int nError = luaL_loadfile(m_pLuaState, strFile);
			if (nError != 0)
			{
				LOG_ERROR("Can't Load Lua Script : %s", strFile);
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

			m_umapCompiledLua.insert(std::make_pair(StringID(strFileName.c_str()), strCompile));*/

			return true;
		}

		std::shared_ptr<LuaThread> LuaSystem::GetThread()
		{
			size_t nSize = m_vecLuaThread.size();
			size_t nUseCount = 0;
			while (m_vecLuaThread.empty() == false)
			{
				std::shared_ptr<LuaThread> pThread = m_vecLuaThread[m_nIndex];

				if (pThread->IsIdle() == true)
				{
					pThread->SetIdle(false);
					return m_vecLuaThread[m_nIndex++];
				}

				++nUseCount;
				m_nIndex = ++m_nIndex % nSize;

				if (nUseCount >= m_vecLuaThread.size())
					break;
			}

			std::shared_ptr<LuaThread> pNewThread = std::make_shared<LuaThread>(m_pLuaState);
			pNewThread->SetIdle(false);

			m_vecLuaThread.push_back(pNewThread);

			return pNewThread;
		}
	}
}