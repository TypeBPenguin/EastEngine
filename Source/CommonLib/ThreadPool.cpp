#include "stdafx.h"
#include "ThreadPool.h"

#include "Log.h"

namespace EastEngine
{
	namespace Thread
	{
		Task::Task(std::thread& thread)
		{
			m_thread.swap(thread);
		}

		Task::~Task()
		{
			if (m_thread.joinable() == true)
			{
				m_thread.join();
			}
		}

		ThreadPool::RequestTask::RequestTask(FuncTask funcTask)
			: funcTask(funcTask)
		{
		}

		ThreadPool::ThreadPool()
			: m_isInit(false)
			, m_isStop(true)
		{
		}

		ThreadPool::~ThreadPool()
		{
			Release();
		}

		bool ThreadPool::Init(uint32_t nThreadCount)
		{
			if (m_isInit == true)
				return true;

			m_isInit = true;
			m_isStop = false;

			m_vecTaskWorkers.resize(nThreadCount);

			for (uint32_t i = 0; i < nThreadCount; ++i)
			{
				std::promise<Task*> promiseThread;
				
				std::thread thread([this](std::future<Task*> futureThread)
				{
					Task* pWorker = futureThread.get();

					while (1)
					{
						RequestTask task(nullptr);

						{
							std::unique_lock<std::mutex> lock(m_mutex);

							m_condition.wait(lock, [&]()
							{
								return this->IsStop() || this->IsEmptyTask() == false;
							});

							if (this->IsStop() == true && m_queueTasks.empty() == true)
								return false;

							task = std::move(m_queueTasks.front());
							m_queueTasks.pop();
						}

						task.promiseThread.set_value(pWorker);
						task.funcTask();
					}
				}, promiseThread.get_future());

				// Aㅏ.. 뭔가 지저분하다. 순환참조잖아?
				Task* pTask = new Task(thread);
				promiseThread.set_value(pTask);

				m_vecTaskWorkers.emplace_back(pTask);
			}

			return true;
		}

		void ThreadPool::Release()
		{
			if (m_isInit == false)
				return;

			{
				std::lock_guard<std::mutex> lock(m_mutex);
				m_isStop = true;
			}

			m_condition.notify_all();

			for (Task* pTask : m_vecTaskWorkers)
			{
				delete pTask;
			}
			m_vecTaskWorkers.clear();

			m_isInit = false;
		}

		std::future<Task*> ThreadPool::Push(FuncTask funcTask)
		{
			assert(m_isInit == true);

			std::future<Task*> futureThread;

			{
				std::lock_guard<std::mutex> lock(m_mutex);

				auto& a = m_queueTasks.emplace(funcTask);
				futureThread = a.promiseThread.get_future();
			}

			// wake up one thread
			m_condition.notify_one();

			return futureThread;
		}

		std::future<Task*> CreateTask(FuncTask funcTask)
		{
			return ThreadPool::GetInstance()->Push(funcTask);
		}
	}
}