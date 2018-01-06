#include "stdafx.h"
#include "MotionPlayer.h"

namespace EastEngine
{
	namespace Graphics
	{
		MotionPlayer::MotionPlayer()
			: m_isStop(false)
			, m_isPause(false)
			, m_fPlayTime(0.f)
			, m_fStopTime(0.f)
			, m_fStopTimeCheck(0.f)
			, m_fBlendTime(0.f)
			, m_fBlendWeight(0.f)
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

		float MotionPlayer::GetBlendWeight() const
		{
			if (Math::IsZero(m_playBackInfo.fBlendTime) == false && m_fBlendTime <= m_playBackInfo.fBlendTime)
			{
				return m_fBlendWeight;
			}
			else
			{
				return m_playBackInfo.fWeight;
			}
		}

		bool MotionPlayer::Update(float fElapsedTime, bool isEnableTransformUpdate)
		{
			if (m_pMotion == nullptr)
				return false;

			if (m_isPause == true)
				return true;

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

						m_motionRecoder.Clear();
					}
					else
					{
						Reset();

						return false;
					}
				}

				if (Math::IsZero(GetBlendWeight()) == false && isEnableTransformUpdate == true)
				{
					float fPlayTime = m_fPlayTime;
					if (m_playBackInfo.isInverse == true)
					{
						fPlayTime = m_pMotion->GetEndTime() - m_fPlayTime;
					}

					m_pMotion->Update(&m_motionRecoder, fPlayTime, m_playBackInfo.isInverse);
				}

				if (Math::IsZero(m_playBackInfo.fBlendTime) == false && m_fBlendTime <= m_playBackInfo.fBlendTime)
				{
					m_fBlendWeight = (m_fBlendTime / m_playBackInfo.fBlendTime) * m_playBackInfo.fWeight;

					LOG_MESSAGE("%f", m_fBlendWeight);

					m_fBlendTime += fElapsedTime;
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

			m_fBlendTime = 0.f;
			m_fBlendWeight = 0.f;

			m_nLoonCount = 0;

			if (m_pMotion != nullptr)
			{
				m_pMotion->DecreaseReference();
				m_pMotion = nullptr;
			}

			m_playBackInfo.Reset();
			m_motionRecoder.Clear();
		}

		const IMotion::Keyframe* MotionPlayer::GetKeyframe(const String::StringID& strBoneName) const
		{
			return m_motionRecoder.GetKeyframe(strBoneName);
		}
	}
}