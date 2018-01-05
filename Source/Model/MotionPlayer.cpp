#include "stdafx.h"
#include "MotionPlayer.h"

namespace EastEngine
{
	namespace Graphics
	{
		MotionPlayer::MotionPlayer()
			: m_isStop(false)
			, m_fPlayTime(0.f)
			, m_fStopTime(0.f)
			, m_fStopTimeCheck(0.f)
			, m_nLoonCount(0)
			, m_pMotion(nullptr)
		{
		}

		MotionPlayer::~MotionPlayer()
		{
			if (m_pMotion != nullptr)
			{
				m_pMotion->DecreaseReference();
				m_pMotion = nullptr;
			}
		}

		void MotionPlayer::SetCaching(const String::StringID& strBoneName, size_t nIndex)
		{
			auto iter = m_umapKeyframeIndexCaching.find(strBoneName);
			if (iter != m_umapKeyframeIndexCaching.end())
			{
				iter->second.value = nIndex;
			}
			else
			{
				m_umapKeyframeIndexCaching.emplace(strBoneName, nIndex);
			}
		}

		size_t MotionPlayer::GetCaching(const String::StringID& strBoneName) const
		{
			auto iter = m_umapKeyframeIndexCaching.find(strBoneName);
			if (iter != m_umapKeyframeIndexCaching.end())
				return iter->second.value;

			return eInvalidCachingIndex;
		}

		void MotionPlayer::SetKeyframe(const String::StringID& strBoneName, const IMotion::Keyframe& keyframe)
		{
			auto iter = m_umapKeyframe.find(strBoneName);
			if (iter != m_umapKeyframe.end())
			{
				iter->second = keyframe;
			}
			else
			{
				m_umapKeyframe.emplace(strBoneName, keyframe);
			}
		}

		const IMotion::Keyframe* MotionPlayer::GetKeyframe(const String::StringID& strBoneName) const
		{
			auto iter = m_umapKeyframe.find(strBoneName);
			if (iter != m_umapKeyframe.end())
				return &iter->second;

			return nullptr;
		}

		bool MotionPlayer::Update(float fElapsedTime)
		{
			if (m_pMotion == nullptr)
				return false;

			bool isPlay = true;
			if (m_isStop == true)
			{
				m_fStopTime += fElapsedTime;
				if (m_fStopTime >= m_fStopTimeCheck)
				{
					m_isStop = false;

					isPlay = false;
				}
			}

			if (isPlay == true)
			{
				float fEndTime = m_pMotion->GetEndTime();
				if (m_fPlayTime >= fEndTime)
				{
					if (IsLoop() == true)
					{
						m_fPlayTime -= fEndTime;

						m_umapKeyframeIndexCaching.clear();
					}
					else
					{
						Reset();

						return false;
					}
				}

				if (Math::IsZero(m_playBackInfo.fWeight) == false)
				{
					float fPlayTime = m_fPlayTime;
					if (m_playBackInfo.isInverse == true)
					{
						fPlayTime = m_pMotion->GetEndTime() - m_fPlayTime;
					}

					m_pMotion->Update(fPlayTime, this);
				}

				m_fPlayTime += fElapsedTime * m_playBackInfo.fSpeed;

				return true;
			}
			else
			{
				Reset();

				return false;
			}
		}

		void MotionPlayer::Play(IMotion* pMotion, const MotionPlaybackInfo* pPlayback)
		{
			if (pMotion == nullptr)
				return;

			Reset();

			m_pMotion = pMotion;
			m_pMotion->IncreaseReference();

			m_fPlayTime = m_pMotion->GetStartTime();

			if (pPlayback != nullptr)
			{
				m_playBackInfo = *pPlayback;
				m_playBackInfo.fWeight = std::clamp(m_playBackInfo.fWeight, 0.f, 1.f);
			}
			else
			{
				m_playBackInfo.Reset();
			}
		}

		void MotionPlayer::Stop(float fStopTime)
		{
			m_fStopTimeCheck = fStopTime;
			m_fStopTime = 0.f;
			m_isStop = true;
		}

		void MotionPlayer::Reset()
		{
			m_isStop = false;

			m_fPlayTime = 0.f;
			m_fStopTime = 0.f;
			m_fStopTimeCheck = 0.f;

			m_nLoonCount = 0;

			if (m_pMotion != nullptr)
			{
				m_pMotion->DecreaseReference();
				m_pMotion = nullptr;
			}

			m_playBackInfo.Reset();

			for (auto& iter : m_umapKeyframeIndexCaching)
			{
				iter.second.value = eInvalidCachingIndex;
			}
		}
	}
}