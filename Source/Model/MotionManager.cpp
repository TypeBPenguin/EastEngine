#include "stdafx.h"
#include "MotionManager.h"

#include "CommonLib/plf_colony.h"

#include "Motion.h"

namespace EastEngine
{
	namespace Graphics
	{
		class MotionManager::Impl
		{
		public:
			Impl();
			~Impl();

		public:
			void Flush(bool isEnableGarbageCollector);

		public:
			// FilePath or ModelName
			IMotion* AllocateMotion(const Motion::Key& key);

			// FilePath or ModelName
			IMotion* GetMotion(const Motion::Key& key);
			IMotion* GetMotion(const size_t nIndex);

			size_t GetMotionCount() const;

		private:
			plf::colony<Motion> m_clnMotions;

			std::unordered_map<Motion::Key, Motion*> m_umapMotions;
		};

		MotionManager::Impl::Impl()
		{
			m_clnMotions.reserve(128);
		}

		MotionManager::Impl::~Impl()
		{
			m_clnMotions.clear();
		}

		void MotionManager::Impl::Flush(bool isEnableGarbageCollector)
		{
			if (isEnableGarbageCollector == true)
			{
				auto iter = m_clnMotions.begin();
				while (iter != m_clnMotions.end())
				{
					Motion& motion = *iter;

					if (motion.GetLoadState() == EmLoadState::eReady ||
						motion.GetLoadState() == EmLoadState::eLoading)
					{
						++iter;
						continue;
					}

					if (motion.GetReferenceCount() > 0)
					{
						motion.SetAlive(true);
						++iter;
						continue;
					}

					if (motion.IsAlive() == false)
					{
						auto iter_find = m_umapMotions.find(motion.GetKey());
						if (iter_find != m_umapMotions.end())
						{
							m_umapMotions.erase(iter_find);
						}

						iter = m_clnMotions.erase(iter);
						continue;
					}

					motion.SubtractLife();
					++iter;
				}
			}
		}

		IMotion* MotionManager::Impl::AllocateMotion(const Motion::Key& key)
		{
			auto iter_find = m_umapMotions.find(key);
			if (iter_find != m_umapMotions.end())
			{
				assert(false);
				return iter_find->second;
			}

			auto iter = m_clnMotions.emplace(key);
			if (iter != m_clnMotions.end())
			{
				Motion* pMotion = &(*iter);
				m_umapMotions.emplace(key, pMotion);

				return pMotion;
			}

			return nullptr;
		}

		IMotion* MotionManager::Impl::GetMotion(const Motion::Key& key)
		{
			auto iter = m_umapMotions.find(key);
			if (iter != m_umapMotions.end())
				return iter->second;

			return nullptr;
		}

		IMotion* MotionManager::Impl::GetMotion(const size_t nIndex)
		{
			auto iter = m_clnMotions.begin();
			m_clnMotions.advance(iter, nIndex);

			if (iter != m_clnMotions.end())
				return &(*iter);

			return nullptr;
		}

		size_t MotionManager::Impl::GetMotionCount() const
		{
			return m_clnMotions.size();
		}

		MotionManager::MotionManager()
			: m_pImpl{ std::make_unique<Impl>() }
		{
		}

		MotionManager::~MotionManager()
		{
		}

		void MotionManager::Flush(bool isEnableGarbageCollector)
		{
			m_pImpl->Flush(isEnableGarbageCollector);
		}

		IMotion* MotionManager::AllocateMotion(const String::StringID& strKey)
		{
			IMotion::Key key(String::GetKey(strKey.c_str()));
			return m_pImpl->AllocateMotion(key);
		}

		IMotion* MotionManager::GetMotion(const String::StringID& strKey)
		{
			IMotion::Key key(String::GetKey(strKey.c_str()));
			return m_pImpl->GetMotion(key);
		}

		IMotion* MotionManager::GetMotion(const size_t nIndex)
		{
			return m_pImpl->GetMotion(nIndex);
		}

		size_t MotionManager::GetMotionCount() const
		{
			return m_pImpl->GetMotionCount();
		}
	}
}