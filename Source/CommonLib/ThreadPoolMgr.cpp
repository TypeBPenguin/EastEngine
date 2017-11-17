#include "stdafx.h"
//#include "ThreadPoolMgr.h"
//
//#include "Log.h"
//
//namespace AloThread
//{
//	SThreadPoolMgr::SThreadPoolMgr()
//		: m_bStop(false)
//	{
//	}
//
//	SThreadPoolMgr::~SThreadPoolMgr()
//	{
//		Release();
//	}
//
//	bool SThreadPoolMgr::Init()
//	{
//		uint32_t nThreadCount = std::thread::hardware_concurrency() - 1;
//		
//		std::function<void(CThread*)> func([this](CThread* pWorker)
//		{
//			while (1)
//			{
//				std::pair<std::promise<CThread*>*, std::function<void()>> funcTask;
//
//				{
//					std::unique_lock<std::mutex> lock(m_mutex);
//
//					m_condition.wait(lock, [&]()
//					{
//						return this->IsStop() || !this->IsTasksEmpty();
//					});
//
//					if (m_bStop && m_queueTasks.empty())
//						return false;
//
//					pWorker->SetState(false);
//
//					funcTask = std::move(m_queueTasks.front());
//					m_queueTasks.pop();
//				}
//
//				if (funcTask.first != nullptr)
//				{
//					funcTask.first->set_value(pWorker);
//				}
//
//				funcTask.second();
//
//				pWorker->SetState(true);
//				pWorker->NotifyAll();
//			}
//		});
//
//		m_vecThreadWorkers.reserve(nThreadCount);
//		for (uint32_t i = 0; i < nThreadCount; ++i)
//		{
//			m_vecThreadWorkers.push_back(new CThread(func));
//		}
//
//		return true;
//	}
//
//	void SThreadPoolMgr::Release()
//	{
//		{
//			std::unique_lock<std::mutex> lock(m_mutex);
//			m_bStop = true;
//		}
//		m_condition.notify_all();
//		for (CThread* pThread : m_vecThreadWorkers)
//		{
//			pThread->Join();
//			delete pThread;
//		}
//		m_vecThreadWorkers.clear();
//	}
//	
//	void SThreadPoolMgr::Enqueue(std::function<void()> func, ThreadState* pThState)
//	{
//		{ // acquire lock
//			std::unique_lock<std::mutex> lock(m_mutex);
//		
//			// add the task
//			if (pThState != nullptr)
//			{
//				m_queueTasks.emplace(std::make_pair(pThState->GetPromise(), func));
//				pThState->Init();
//			}
//			else
//			{
//				m_queueTasks.emplace(std::make_pair(nullptr, func));
//			}
//		} // release lock
//		
//		  // wake up one thread
//		m_condition.notify_one();
//	}
//
//	void Join(const std::string strName)
//	{
//	}
//}