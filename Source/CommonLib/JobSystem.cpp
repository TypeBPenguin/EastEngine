#include "stdafx.h"
#include "JobSystem.h"

#include "Lock.h"

namespace est
{
	namespace jobsystem
	{
		template <typename T, size_t capacity>
		class ThreadSafeRingBuffer
		{
		public:
			// Push an item to the end if there is free space
			//  Returns true if succesful
			//  Returns false if there is not enough space
			inline bool push_back(const T& item)
			{
				bool result = false;
				m_lock.AcquireWriteLock();
				size_t next = (m_head + 1) % capacity;
				if (next != m_tail)
				{
					m_data[m_head] = item;
					m_head = next;
					result = true;
				}
				m_lock.ReleaseWriteLock();
				return result;
			}

			// Get an item if there are any
			//  Returns true if succesful
			//  Returns false if there are no items
			inline bool pop_front(T& item)
			{
				bool result = false;
				m_lock.AcquireWriteLock();
				if (m_tail != m_head)
				{
					item = m_data[m_tail];
					m_tail = (m_tail + 1) % capacity;
					result = true;
				}
				m_lock.ReleaseWriteLock();
				return result;
			}

		private:
			T m_data[capacity];
			size_t m_head{ 0 };
			size_t m_tail{ 0 };
			thread::SRWLock m_lock;
		};

		uint32_t s_numThreads = 0;    // number of worker threads, it will be initialized in the Initialize() function
		uint64_t s_currentLabel = 0;    // tracks the state of execution of the main thread

		ThreadSafeRingBuffer<std::function<void()>, 256> s_jobPool;    // a thread safe queue to put pending jobs onto the end (with a capacity of 256 jobs). A worker thread can grab a job from the beginning

		std::condition_variable s_wakeCondition;    // used in conjunction with the s_wakeMutex below. Worker threads just sleep when there is no job, and the main thread can wake them up
		std::mutex s_wakeMutex;    // used in conjunction with the s_wakeCondition above
		std::atomic<uint64_t> s_finishedLabel;    // track the state of execution across background worker threads

		void Initialize()
		{
			// Initialize the worker execution state to 0:
			s_finishedLabel.store(0);

			// Retrieve the number of hardware threads in this system:
			const uint32_t numCores = std::thread::hardware_concurrency();

			// Calculate the actual number of worker threads we want:
			s_numThreads = std::max(1u, numCores);

			// Create all our worker threads while immediately starting them:
			for (uint32_t threadID = 0; threadID < s_numThreads; ++threadID)
			{
				std::thread worker([]()
					{
						std::function<void()> job; // the current job for the thread, it's empty at start.

						// This is the infinite loop that a worker thread will do 
						while (true)
						{
							if (s_jobPool.pop_front(job)) // try to grab a job from the s_jobPool queue
							{
								// It found a job, execute it:
								job(); // execute job
								s_finishedLabel.fetch_add(1); // update worker label state
							}
							else
							{
								// no job, put thread to sleep
								std::unique_lock<std::mutex> lock(s_wakeMutex);
								s_wakeCondition.wait(lock);
							}
						}
					});
				// *****Here we could do platform specific thread setup...

				worker.detach(); // forget about this thread, let it do it's job in the infinite loop that we created above
			}
		}

		// This little helper function will not let the system to be deadlocked while the main thread is waiting for something
		inline void poll()
		{
			s_wakeCondition.notify_one(); // wake one worker thread
			std::this_thread::yield(); // allow this thread to be rescheduled
		}

		void Execute(const std::function<void()>& job)
		{
			if (s_numThreads == 0)
			{
				Initialize();
			}

			// The main thread label state is updated:
			s_currentLabel += 1;

			// Try to push a new job until it is pushed successfully:
			while (s_jobPool.push_back(job) == false)
			{
				poll();
			}

			s_wakeCondition.notify_one(); // wake one thread
		}

		bool IsBusy()
		{
			// Whenever the main thread label is not reached by the workers, it indicates that some worker is still alive
			return s_finishedLabel.load() < s_currentLabel;
		}

		void Wait()
		{
			while (IsBusy()) { poll(); }
		}

		void Dispatch(uint32_t jobCount, uint32_t groupSize, const std::function<void(const JobDispatchArgs&)>& job)
		{
			if (jobCount == 0 || groupSize == 0)
				return;

			if (s_numThreads == 0)
			{
				Initialize();
			}

			// Calculate the amount of job groups to dispatch (overestimate, or "ceil"):
			const uint32_t groupCount = (jobCount + groupSize - 1) / groupSize;

			// The main thread label state is updated:
			s_currentLabel += groupCount;

			for (uint32_t groupIndex = 0; groupIndex < groupCount; ++groupIndex)
			{
				// For each group, generate one real job:
				auto jobGroup = [jobCount, groupSize, job, groupIndex]()
				{
					// Calculate the current group's offset into the jobs:
					const uint32_t groupJobOffset = groupIndex * groupSize;
					const uint32_t groupJobEnd = std::min(groupJobOffset + groupSize, jobCount);

					JobDispatchArgs args;
					args.groupIndex = groupIndex;

					// Inside the group, loop through all job indices and execute job for each index:
					for (uint32_t i = groupJobOffset; i < groupJobEnd; ++i)
					{
						args.jobIndex = i;
						job(args);
					}
				};

				// Try to push a new job until it is pushed successfully:
				while (s_jobPool.push_back(jobGroup) == false)
				{
					poll();
				}

				s_wakeCondition.notify_one(); // wake one thread
			}
		}

		void ParallelFor(size_t jobCount, const std::function<void(size_t index)>& job)
		{
			Concurrency::parallel_for((size_t)0, jobCount, job);
		}
	}
}