#pragma once

#include <future>

#include "Singleton.h"

namespace EastEngine
{
	namespace Thread
	{
		class ThreadPool;

		class Task
		{
			friend ThreadPool;
		public:
			Task(std::thread& thread);

		private:
			~Task();

		public:
			void Wait();

		public:
			enum EmState
			{
				eIdle = 0,
				eProcessing,
			};

			EmState GetState();

		private:
			void SetState(EmState emState);

		private:
			std::atomic<EmState> m_emState;
			std::condition_variable m_condition;
			std::mutex m_mutex;

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
				std::function<void()> funcTask;

				RequestTask(std::function<void()> funcTask);
			};

		public:
			bool Init(uint32_t nThreadCount);
			void Release();

		public:
			std::future<Task*> Push(std::function<void()> funcTask);

			bool IsStop() { return m_isStop.load(); }
			bool IsEmptyTask() { return m_queueTasks.empty(); }

		private:
			void SetTaskState(Task* pTask, Task::EmState emState) { pTask->SetState(emState); }

		private:
			bool m_isInit;

			std::atomic<bool> m_isStop;

			std::vector<Task*> m_vecTaskWorkers;
			std::queue<RequestTask> m_queueTasks;

			std::mutex m_mutex;
			std::condition_variable m_condition;
		};

		std::future<Task*> CreateTask(std::function<void()> funcTask);
	}
}