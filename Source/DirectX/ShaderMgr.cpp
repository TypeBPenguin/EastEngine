#include "stdafx.h"
#include "ShaderMgr.h"

#include "CommonLib/FileUtil.h"
#include "CommonLib/DirectoryMonitor.h"
#include "CommonLib/ThreadPool.h"

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

		void ShaderManager::Flush()
		{
			while (m_conQueueCompleteEffectAsyncLoader.empty() == false)
			{
				CompleteEffectAsyncLoader loader;
				if (m_conQueueCompleteEffectAsyncLoader.try_pop(loader) == true)
				{
					Effect* pEffect = static_cast<Effect*>(loader.pEffect);
					pEffect->SetValid(loader.isSuccess);
					loader.funcCallback(loader.pEffect, loader.isSuccess);
				}
			}
		}

		void CALLBACK ShaderManager::DirectoryMonitorCallback(const char* strPath, DWORD dwAction, LPARAM lParam)
		{
			switch (dwAction)
			{
			case FILE_ACTION_ADDED:
				LOG_MESSAGE("FILE_ACTION_ADDED : %s", strPath);
				break;
			case FILE_ACTION_REMOVED:
				LOG_MESSAGE("FILE_ACTION_REMOVED : %s", strPath);
				break;
			case FILE_ACTION_MODIFIED:
				LOG_MESSAGE("FILE_ACTION_MODIFIED : %s", strPath);
				break;
			case FILE_ACTION_RENAMED_OLD_NAME:
				LOG_MESSAGE("FILE_ACTION_RENAMED_OLD_NAME : %s", strPath);
				break;
			case FILE_ACTION_RENAMED_NEW_NAME:
				LOG_MESSAGE("FILE_ACTION_RENAMED_NEW_NAME : %s", strPath);
				break;
			default:
				break;
			}
		}

		IEffect* ShaderManager::GetEffect(const String::StringID& strName)
		{
			std::lock_guard<std::mutex> lock(m_mutex);

			auto iter = m_umapEffects.find(strName);
			if (iter != m_umapEffects.end())
				return iter->second;

			return nullptr;
		}

		bool ShaderManager::AddEffect(IEffect* pEffect)
		{
			if (pEffect == nullptr || GetEffect(pEffect->GetName()) != nullptr)
				return false;

			std::lock_guard<std::mutex> lock(m_mutex);

			m_umapEffects.emplace(pEffect->GetName(), pEffect);

			return true;
		}

		void ShaderManager::RemoveEffect(IEffect* pEffect)
		{
			if (pEffect == nullptr)
				return;

			std::lock_guard<std::mutex> lock(m_mutex);

			auto iter = m_umapEffects.find(pEffect->GetName());
			if (iter == m_umapEffects.end())
				return;

			Effect* pRealEffect = static_cast<Effect*>(iter->second);
			SafeDelete(pRealEffect);
			m_umapEffects.erase(iter);
		}

		void ShaderManager::RequestEffectAsyncLoad(IEffect* pEffect, std::function<bool()> funcLoader)
		{
		}

		void ShaderManager::CompleteEffectAsyncLoad(IEffect* pEffect, bool isSuccess, std::function<void(IEffect*, bool)> funcCallback)
		{
			CompleteEffectAsyncLoader loader;
			loader.pEffect = pEffect;
			loader.isSuccess = isSuccess;
			loader.funcCallback = funcCallback;
			m_conQueueCompleteEffectAsyncLoader.push(loader);
		}
	}
}