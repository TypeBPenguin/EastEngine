#include "stdafx.h"
#include "ShaderMgr.h"

#include "../CommonLib/FileUtil.h"
#include "../CommonLib/DirectoryMonitor.h"

#include "Effect.h"

namespace EastEngine
{
	namespace Graphics
	{
		ShaderManager::ShaderManager()
			: m_isInit(false)
		{
		}

		ShaderManager::~ShaderManager()
		{
			Release();
		}

		bool ShaderManager::Init()
		{
			if (m_isInit == true)
				return true;

			m_isInit = true;

			File::DirectoryMonitor::GetInstance()->AddDirMonitor(File::GetPath(File::EmPath::eFx), ShaderManager::DirectoryMonitorCallback);

			return true;
		}

		void ShaderManager::Release()
		{
			if (m_isInit == false)
				return;

			for (auto& iter : m_umapEffects)
			{
				Effect* pEffect = static_cast<Effect*>(iter.second);
				SafeDelete(pEffect);
			}
			m_umapEffects.clear();

			m_isInit = false;
		}

		void CALLBACK ShaderManager::DirectoryMonitorCallback(const char* strPath, DWORD dwAction, LPARAM lParam)
		{
			switch (dwAction)
			{
			case FILE_ACTION_ADDED:
				PRINT_LOG("FILE_ACTION_ADDED : %s", strPath);
				break;
			case FILE_ACTION_REMOVED:
				PRINT_LOG("FILE_ACTION_REMOVED : %s", strPath);
				break;
			case FILE_ACTION_MODIFIED:
				PRINT_LOG("FILE_ACTION_MODIFIED : %s", strPath);
				break;
			case FILE_ACTION_RENAMED_OLD_NAME:
				PRINT_LOG("FILE_ACTION_RENAMED_OLD_NAME : %s", strPath);
				break;
			case FILE_ACTION_RENAMED_NEW_NAME:
				PRINT_LOG("FILE_ACTION_RENAMED_NEW_NAME : %s", strPath);
				break;
			default:
				break;
			}
		}

		IEffect* ShaderManager::GetEffect(const String::StringID& strName)
		{
			auto iter = m_umapEffects.find(strName);
			if (iter != m_umapEffects.end())
				return iter->second;

			return nullptr;
		}

		bool ShaderManager::AddEffect(IEffect* pEffect)
		{
			if (pEffect == nullptr || GetEffect(pEffect->GetName()) != nullptr)
				return false;

			m_umapEffects.emplace(pEffect->GetName(), pEffect);

			return true;
		}

		void ShaderManager::RemoveEffect(IEffect* pEffect)
		{
			if (pEffect == nullptr)
				return;

			auto iter = m_umapEffects.find(pEffect->GetName());
			if (iter == m_umapEffects.end())
				return;

			m_umapEffects.erase(iter);
		}
	}
}