#include "stdafx.h"
#include "MotionPlayer.h"

namespace eastengine
{
	namespace graphics
	{
		MotionPlayer::MotionPlayer()
		{
		}

		MotionPlayer::MotionPlayer(const MotionPlayer& source)
		{
			*this = source;
		}

		MotionPlayer::MotionPlayer(MotionPlayer&& source) noexcept
		{
			*this = std::move(source);
		}

		MotionPlayer::~MotionPlayer()
		{
			if (m_pMotion != nullptr)
			{
				m_pMotion->DecreaseReference();
				m_pMotion = nullptr;
			}
		}

		MotionPlayer& MotionPlayer::operator = (const MotionPlayer& source)
		{
			m_isStop = source.m_isStop;
			m_isPause = source.m_isPause;
			m_playTime = source.m_playTime;
			m_stopTime = source.m_stopTime;
			m_stopTimeCheck = source.m_stopTimeCheck;
			m_blendTime = source.m_blendTime;
			m_blendWeight = source.m_blendWeight;
			m_loonCount = source.m_loonCount;
			m_playBackInfo = source.m_playBackInfo;
			m_motionRecorder = source.m_motionRecorder;
			m_pMotion = source.m_pMotion;

			return *this;
		}

		MotionPlayer& MotionPlayer::operator = (MotionPlayer&& source) noexcept
		{
			m_isStop = std::move(source.m_isStop);
			m_isPause = std::move(source.m_isPause);
			m_playTime = std::move(source.m_playTime);
			m_stopTime = std::move(source.m_stopTime);
			m_stopTimeCheck = std::move(source.m_stopTimeCheck);
			m_blendTime = std::move(source.m_blendTime);
			m_blendWeight = std::move(source.m_blendWeight);
			m_loonCount = std::move(source.m_loonCount);
			m_playBackInfo = std::move(source.m_playBackInfo);
			m_motionRecorder = std::move(source.m_motionRecorder);
			m_pMotion = std::move(source.m_pMotion);

			return *this;
		}

		float MotionPlayer::GetBlendWeight() const
		{
			if (math::IsZero(m_playBackInfo.blendTime) == false && m_blendTime <= m_playBackInfo.blendTime)
			{
				return m_blendWeight;
			}
			else
			{
				return m_playBackInfo.weight;
			}
		}

		bool MotionPlayer::Update(float elapsedTime, bool isEnableTransformUpdate)
		{
			if (m_pMotion == nullptr)
				return false;

			if (m_isPause == true)
				return true;

			bool isPlay = true;
			if (m_isStop == true)
			{
				m_stopTime += elapsedTime;
				if (m_stopTime >= m_stopTimeCheck)
				{
					m_isStop = false;

					isPlay = false;
				}
			}

			if (isPlay == true)
			{
				const float endTime = m_pMotion->GetEndTime();
				if (m_playTime >= endTime)
				{
					if (IsLoop() == true)
					{
						m_playTime -= endTime;

						m_motionRecorder.Clear(m_playTime);
					}
					else
					{
						Reset(0.f);

						return false;
					}
				}

				if (math::IsZero(GetBlendWeight()) == false)
				{
					float fPlayTime = m_playTime;
					if (m_playBackInfo.isInverse == true)
					{
						fPlayTime = m_pMotion->GetEndTime() - m_playTime;
					}

					m_pMotion->Update(&m_motionRecorder, fPlayTime, m_playBackInfo.isInverse, isEnableTransformUpdate);
				}

				if (math::IsZero(m_playBackInfo.blendTime) == false && m_blendTime <= m_playBackInfo.blendTime)
				{
					m_blendWeight = (m_blendTime / m_playBackInfo.blendTime) * m_playBackInfo.weight;

					m_blendTime += elapsedTime;
				}

				m_playTime += elapsedTime * m_playBackInfo.speed;

				return true;
			}
			else
			{
				Reset(0.f);

				return false;
			}
		}

		void MotionPlayer::Play(IMotion* pMotion, const MotionPlaybackInfo* pPlayback, float blendTime)
		{
			if (pMotion == nullptr)
				return;

			const float startTime = pMotion->GetStartTime();
			Reset(startTime);

			m_pMotion = pMotion;
			m_pMotion->IncreaseReference();

			if (pPlayback != nullptr)
			{
				m_playBackInfo = *pPlayback;
				m_playBackInfo.weight = std::clamp(m_playBackInfo.weight, 0.f, 1.f);
			}
			else
			{
				m_playBackInfo.Reset();
			}

			m_playBackInfo.blendTime = blendTime;
		}

		void MotionPlayer::Stop(float stopTime)
		{
			m_stopTimeCheck = stopTime;
			m_stopTime = 0.f;
			m_isStop = true;
		}

		void MotionPlayer::Reset(float startTime)
		{
			m_isStop = false;

			m_playTime = startTime;

			m_stopTime = 0.f;
			m_stopTimeCheck = 0.f;

			m_blendTime = 0.f;
			m_blendWeight = 0.f;

			m_loonCount = 0;

			if (m_pMotion != nullptr)
			{
				m_pMotion->DecreaseReference();
				m_pMotion = nullptr;
			}

			m_playBackInfo.Reset();
			m_motionRecorder.Clear(startTime);
		}
	}
}