#pragma once

struct JobDispatchArgs
{
	uint32_t jobIndex{ 0 };
	uint32_t groupIndex{ 0 };
};

namespace est
{
	namespace jobsystem
	{
		// Create the internal resources such as worker threads, etc. Call it once when initializing the application.
		void Initialize();

		// Add a job to execute asynchronously. Any idle thread will execute this job.
		void Execute(const std::function<void()>& job);

		// Divide a job onto multiple jobs and execute in parallel.
		//  jobCount    : how many jobs to generate for this task.
		//  groupSize   : how many jobs to execute per thread. Jobs inside a group execute serially. It might be worth to increase for small jobs
		//  func        : receives a JobDispatchArgs as parameter
		void Dispatch(uint32_t jobCount, uint32_t groupSize, const std::function<void(const JobDispatchArgs&)>& job);

		// No need Wait();
		void ParallelFor(size_t jobCount, const std::function<void(size_t index)>& job);

		// Check if any threads are working currently or not
		bool IsBusy();

		// Wait until all threads become idle
		void Wait();
	}
}