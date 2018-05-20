#include "stdafx.h"
#include "ShaderMgr.h"

#include "CommonLib/FileUtil.h"
#include "CommonLib/DirectoryMonitor.h"
#include "CommonLib/ThreadPool.h"

#include "Effect.h"

namespace eastengine
{
	namespace graphics
	{
		class ShaderManager::Impl
		{
		public:
			Impl();
			~Impl();

		public:
			void Flush();

		public:
			IEffect * GetEffect(const String::StringID& strName);
			bool AddEffect(IEffect* pEffect);
			void RemoveEffect(IEffect* pEffect);

			void CompleteEffectAsyncLoad(IEffect* pEffect, bool isSuccess, std::function<void(IEffect*, bool)> funcCallback);

		private:
			std::mutex m_mutex;

			struct CompleteEffectAsyncLoader
			{
				IEffect* pEffect = nullptr;
				bool isSuccess = false;
				std::function<void(IEffect*, bool)> funcCallback;
			};
			Concurrency::concurrent_queue<CompleteEffectAsyncLoader> m_conQueueCompleteEffectAsyncLoader;

			std::unordered_map<String::StringID, IEffect*> m_umapEffects;
		};

		void CALLBACK DirectoryMonitorCallback(const char* strPath, DWORD dwAction, LPARAM lParam)
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

		ShaderManager::Impl::Impl()
		{
			file::DirectoryMonitor::GetInstance()->AddDirectoryMonitor(file::GetPath(file::EmPath::eFx), DirectoryMonitorCallback);
		}

		ShaderManager::Impl::~Impl()
		{
			for (auto& iter : m_umapEffects)
			{
				Effect* pEffect = static_cast<Effect*>(iter.second);
				SafeDelete(pEffect);
			}
			m_umapEffects.clear();
		}

		void ShaderManager::Impl::Flush()
		{
			TRACER_EVENT("ShaderManager::Flush");
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

		IEffect* ShaderManager::Impl::GetEffect(const String::StringID& strName)
		{
			std::lock_guard<std::mutex> lock(m_mutex);

			auto iter = m_umapEffects.find(strName);
			if (iter != m_umapEffects.end())
				return iter->second;

			return nullptr;
		}

		bool ShaderManager::Impl::AddEffect(IEffect* pEffect)
		{
			if (pEffect == nullptr || GetEffect(pEffect->GetName()) != nullptr)
				return false;

			std::lock_guard<std::mutex> lock(m_mutex);

			m_umapEffects.emplace(pEffect->GetName(), pEffect);

			return true;
		}

		void ShaderManager::Impl::RemoveEffect(IEffect* pEffect)
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

		void ShaderManager::Impl::CompleteEffectAsyncLoad(IEffect* pEffect, bool isSuccess, std::function<void(IEffect*, bool)> funcCallback)
		{
			CompleteEffectAsyncLoader loader;
			loader.pEffect = pEffect;
			loader.isSuccess = isSuccess;
			loader.funcCallback = funcCallback;
			m_conQueueCompleteEffectAsyncLoader.push(loader);
		}

		ShaderManager::ShaderManager()
			: m_pImpl{ std::make_unique<Impl>() }
		{
		}

		ShaderManager::~ShaderManager()
		{
		}

		void ShaderManager::Flush()
		{
			m_pImpl->Flush();
		}

		IEffect* ShaderManager::GetEffect(const String::StringID& strName)
		{
			return m_pImpl->GetEffect(strName);
		}

		bool ShaderManager::AddEffect(IEffect* pEffect)
		{
			return m_pImpl->AddEffect(pEffect);
		}

		void ShaderManager::RemoveEffect(IEffect* pEffect)
		{
			m_pImpl->RemoveEffect(pEffect);
		}
		
		void ShaderManager::CompleteEffectAsyncLoad(IEffect* pEffect, bool isSuccess, std::function<void(IEffect*, bool)> funcCallback)
		{
			m_pImpl->CompleteEffectAsyncLoad(pEffect, isSuccess, funcCallback);
		}
	}
}