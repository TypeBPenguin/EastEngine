#pragma once

#include <future>

#include "Singleton.h"

namespace EastEngine
{
	namespace Thread
	{
		using FuncTask = std::function<void()>;

		class Task
		{
		public:
			Task(std::thread& thread);
			~Task();

		private:
			std::thread m_thread;
		};

		class ThreadPool : public Singleton<ThreadPool>
		{
			friend Singleton<ThreadPool>;
		private:
			ThreadPool();
			virtual ~ThreadPool();

			struct RequestTask
			{
				std::promise<Task*> promiseThread;
				FuncTask funcTask;

				RequestTask(FuncTask funcTask);
			};

		public:
			bool Init(uint32_t nThreadCount);
			void Release();

		public:
			std::future<Task*> Push(FuncTask funcTask);

			bool IsStop() { return m_isStop; }
			bool IsEmptyTask() { return m_queueTasks.empty(); }

		private:
			bool m_isInit;
			bool m_isStop;

			std::vector<Task*> m_vecTaskWorkers;
			std::queue<RequestTask> m_queueTasks;

			std::mutex m_mutex;
			std::condition_variable m_condition;
		};

		std::future<Task*> CreateTask(FuncTask funcTask);
	}
}