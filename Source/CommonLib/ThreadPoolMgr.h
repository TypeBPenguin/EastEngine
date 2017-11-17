#pragma once
//
//#include "Singleton.h"
//
//namespace EastEngine
//{
//	namespace Thread
//	{
//		typedef void(*Task)();
//		typedef void(*Callback)();
//
//		class Thread
//		{
//		public:
//			Thread(Task funcTask);
//			virtual ~Thread();
//		
//		private:
//			Concurrency::task<void> m_task;
//		};
//
//		class ThreadMgr : public Singleton<ThreadMgr>
//		{
//			friend Singleton<ThreadMgr>;
//		private:
//			ThreadMgr();
//			virtual ~ThreadMgr();
//
//			struct TaskInstance
//			{
//				Task funcTask;
//				Callback funcCallback;
//			};
//
//		public:
//			bool Init();
//			void Release();
//
//			void Enqueue(Task funcTask, Callback funcCallback = nullptr);
//
//		private:
//			bool m_isInit;
//
//			std::vector<Thread*> m_veThreadWorkers;
//			std::queue<std::function<void()>> m_queueTasks;
//
//			std::mutex m_mutex;
//			std::condition_variable m_condition;
//		};
//	}
//}