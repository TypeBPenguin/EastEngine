#include "stdafx.h"
#include "ThreadPool.h"

namespace eastengine
{
	namespace thread
	{
		class Task::Impl
		{
		public:
			Impl(std::function<void(Task*)> func, Task* pInterface);
			~Impl();

		public:
			void Wait();

		public:
			State GetState();
			void SetState(State emState);

		private:
			std::atomic<State> m_emState{ State::eIdle };
			std::condition_variable m_condition;
			std::mutex m_mutex;

			std::thread m_thread;
		};

		Task::Impl::Impl(std::function<void(Task*)> func, Task* pInterface)
			: m_thread{ func, pInterface }
		{
		}

		Task::Impl::~Impl()
		{
			if (m_thread.joinable() == true)
			{
				m_thread.join();
			}
		}

		void Task::Impl::Wait()
		{
			std::unique_lock<std::mutex> lock(m_mutex);

			m_condition.wait(lock, [&]()
			{
				return m_emState.load() == State::eIdle;
			});
		}

		Task::State Task::Impl::GetState()
		{
			return m_emState.load();
		}

		void Task::Impl::SetState(State emState)
		{
			m_emState.store(emState);

			if (m_emState == State::eIdle)
			{
				m_condition.notify_all();
			}
		}

		Task::Task(std::function<void(Task*)> func)
			: m_pImpl{ std::make_unique<Impl>(func, this) }
		{
		}

		Task::~Task()
		{
		}

		void Task::Wait()
		{
			m_pImpl->Wait();
		}

		Task::State Task::GetState()
		{
			return m_pImpl->GetState();
		}

		void Task::SetState(State emState)
		{
			m_pImpl->SetState(emState);
		}

		struct RequestTask
		{
			std::promise<Task*> promiseThread;
			std::function<void()> funcTask;

			RequestTask(std::function<void()> funcTask);
		};

		RequestTask::RequestTask(std::function<void()> funcTask)
			: funcTask(funcTask)
		{
		}

		class ThreadPool::Impl
		{
		public:
			Impl();
			~Impl();

		public:
			std::future<Task*> Push(std::function<void()> funcTask);

		public:
			bool IsStop() { return m_isStop.load(); }
			bool IsEmptyTask() { return m_queueTasks.empty(); }

		private:
			void SetTaskState(Task* pTask, Task::State emState) { pTask->SetState(emState); }

		private:
			std::atomic<bool> m_isStop{ false };

			std::vector<Task*> m_vecTaskWorkers;
			std::queue<RequestTask> m_queueTasks;

			std::mutex m_mutex;
			std::condition_variable m_condition;
		};

		ThreadPool::Impl::Impl()
		{
			const size_t nThreadCount = std::thread::hardware_concurrency() - 1;
			m_vecTaskWorkers.resize(nThreadCount);

			for (uint32_t i = 0; i < nThreadCount; ++i)
			{
				m_vecTaskWorkers[i] = new Task([&](Task* pWorker)
				{
					while (true)
					{
						RequestTask task(nullptr);

						{
							std::unique_lock<std::mutex> lock(m_mutex);

							m_condition.wait(lock, [&]()
							{
								return IsStop() == true || IsEmptyTask() == false;
							});

							if (IsStop() == true && IsEmptyTask() == true)
								return;

							task = std::move(m_queueTasks.front());
							m_queueTasks.pop();
						}

						SetTaskState(pWorker, Task::State::eProcessing);

						task.promiseThread.set_value(pWorker);
						task.funcTask();

						SetTaskState(pWorker, Task::State::eIdle);
					}
				});
			}
		}

		ThreadPool::Impl::~Impl()
		{
			m_isStop.store(true);

			m_condition.notify_all();

			for (Task* pTask : m_vecTaskWorkers)
			{
				pTask->Wait();

				delete pTask;
			}
			m_vecTaskWorkers.clear();
		}

		std::future<Task*> ThreadPool::Impl::Push(std::function<void()> funcTask)
		{
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

		ThreadPool::ThreadPool()
			: m_pImpl{ std::make_unique<Impl>() }
		{
		}

		ThreadPool::~ThreadPool()
		{
		}

		std::future<Task*> ThreadPool::Push(std::function<void()> funcTask)
		{
			return m_pImpl->Push(funcTask);
		}

		std::future<Task*> CreateTask(std::function<void()> funcTask)
		{
			return ThreadPool::GetInstance()->Push(funcTask);
		}
	}
}