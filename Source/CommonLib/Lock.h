#pragma once

namespace est
{
	namespace thread
	{
		class SRWLock
		{
		public:
			SRWLock()
			{
				InitializeSRWLock(&m_lock);
			}

			~SRWLock() = default;

			void AcquireReadLock()
			{
				AcquireSRWLockShared(&m_lock);
			}

			bool TryAcquireReadLock()
			{
				return TryAcquireSRWLockShared(&m_lock) == TRUE;
			}

			void ReleaseReadLock()
			{
				ReleaseSRWLockShared(&m_lock);
			}

			void AcquireWriteLock()
			{
				AcquireSRWLockExclusive(&m_lock);
			}

			bool TryAcquireWriteLock()
			{
				return TryAcquireSRWLockExclusive(&m_lock) == TRUE;
			}

			void ReleaseWriteLock()
			{
				ReleaseSRWLockExclusive(&m_lock);
			}

		private:
			SRWLOCK m_lock{ SRWLOCK_INIT };
		};

		class SRWReadLock
		{
		public:
			SRWReadLock(SRWLock* pLock)
				: m_pLock(pLock)
			{
				m_pLock->AcquireReadLock();
			}

			~SRWReadLock()
			{
				m_pLock->ReleaseReadLock();
			}

		private:
			SRWLock* m_pLock{ nullptr };
		};

		class SRWWriteLock
		{
		public:
			SRWWriteLock(SRWLock* pLock)
				: m_pLock(pLock)
			{
				m_pLock->AcquireWriteLock();
			}

			~SRWWriteLock()
			{
				m_pLock->ReleaseWriteLock();
			}

		private:
			SRWLock* m_pLock{ nullptr };
		};
	}
}