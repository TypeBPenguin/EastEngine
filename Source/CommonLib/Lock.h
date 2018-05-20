#pragma once

namespace eastengine
{
	namespace thread
	{
		class Lock
		{
		public:
			Lock() = default;
			~Lock()
			{
				ReleaseSRWLockExclusive(&m_lock);
			}

			void Enter()
			{
				AcquireSRWLockExclusive(&m_lock);
			}

			void Leave()
			{
				ReleaseSRWLockExclusive(&m_lock);
			}

		private:
			SRWLOCK m_lock{ SRWLOCK_INIT };
		};

		class AutoLock
		{
		public:
			AutoLock(Lock* pLock)
				: m_pLock(pLock)
			{
				m_pLock->Enter();
			}

			~AutoLock()
			{
				m_pLock->Leave();
			}

		private:
			Lock* m_pLock{ nullptr };
		};
	}
}