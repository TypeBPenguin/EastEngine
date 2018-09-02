#pragma once

#include <future>

#include "Singleton.h"

namespace eastengine
{
	namespace thread
	{
		class ThreadPool;

		class Task
		{
			friend ThreadPool;
		public:
			Task(std::function<void(Task*)> func);

		private:
			~Task();

		public:
			void Wait();

		public:
			enum State
			{
				eIdle = 0,
				eProcessing,
			};

			State GetState();

		private:
			void SetState(State emState);

		private:
			class Impl;
			std::unique_ptr<Impl> m_pImpl;
		};

		class ThreadPool : public Singleton<ThreadPool>
		{
			friend Singleton<ThreadPool>;
		private:
			ThreadPool();
			virtual ~ThreadPool();

		public:
			std::future<Task*> Push(std::function<void()> funcTask);

		private:
			class Impl;
			std::unique_ptr<Impl> m_pImpl;
		};

		std::future<Task*> CreateTask(std::function<void()> funcTask);
	}
}