#pragma once

#include "CommonLib/Singleton.h"

#include "EffectInterface.h"

namespace EastEngine
{
	namespace Graphics
	{
		class ShaderManager : public Singleton<ShaderManager>
		{
			friend Singleton<ShaderManager>;
		private:
			ShaderManager();
			virtual ~ShaderManager();

		public:
			bool Init();
			void Release();

			void Flush();

			static void CALLBACK DirectoryMonitorCallback(const char* strPath, DWORD dwAction, LPARAM lParam);

		public:
			IEffect* GetEffect(const String::StringID& strName);
			bool AddEffect(IEffect* pEffect);
			void RemoveEffect(IEffect* pEffect);

			void RequestEffectAsyncLoad(IEffect* pEffect, std::function<bool()> funcLoader);
			void CompleteEffectAsyncLoad(IEffect* pEffect, bool isSuccess, std::function<void(IEffect*, bool)> funcCallback);

		private:
			bool m_isInit;
			std::mutex m_mutex;

			struct RequestEffectAsyncLoader
			{
				IEffect* pEffect = nullptr;
				std::function<bool()> funcLoader;
			};
			Concurrency::concurrent_queue<RequestEffectAsyncLoader> m_conQueueRequestEffectAsyncLoader;

			struct CompleteEffectAsyncLoader
			{
				IEffect* pEffect = nullptr;
				bool isSuccess = false;
				std::function<void(IEffect*, bool)> funcCallback;
			};
			Concurrency::concurrent_queue<CompleteEffectAsyncLoader> m_conQueueCompleteEffectAsyncLoader;

			std::unordered_map<String::StringID, IEffect*> m_umapEffects;
		};
	}
}