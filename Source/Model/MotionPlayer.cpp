#include "stdafx.h"
#include "MotionPlayer.h"

namespace eastengine
{
	namespace graphics
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
			if (math::IsZero(m_playBackInfo.fBlendTime) == false && m_fBlendTime <= m_playBackInfo.fBlendTime)
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
				const float fEndTime = m_pMotion->GetEndTime();
				if (m_fPlayTime >= fEndTime)
				{
					if (IsLoop() == true)
					{
						m_fPlayTime -= fEndTime;

						m_motionRecorder.Clear(m_fPlayTime);
					}
					else
					{
						Reset(0.f);

						return false;
					}
				}

				if (math::IsZero(GetBlendWeight()) == false)
				{
					float fPlayTime = m_fPlayTime;
					if (m_playBackInfo.isInverse == true)
					{
						fPlayTime = m_pMotion->GetEndTime() - m_fPlayTime;
					}

					m_pMotion->Update(&m_motionRecorder, fPlayTime, m_playBackInfo.isInverse, isEnableTransformUpdate);
				}

				if (math::IsZero(m_playBackInfo.fBlendTime) == false && m_fBlendTime <= m_playBackInfo.fBlendTime)
				{
					m_fBlendWeight = (m_fBlendTime / m_playBackInfo.fBlendTime) * m_playBackInfo.fWeight;

					m_fBlendTime += fElapsedTime;
				}

				m_fPlayTime += fElapsedTime * m_playBackInfo.fSpeed;

				return true;
			}
			else
			{
				Reset(0.f);

				return false;
			}
		}

		void MotionPlayer::Play(IMotion* pMotion, const MotionPlaybackInfo* pPlayback)
		{
			if (pMotion == nullptr)
				return;

			const float fStartTime = pMotion->GetStartTime();

			Reset(fStartTime);

			m_pMotion = pMotion;
			m_pMotion->IncreaseReference();

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

		void MotionPlayer::Reset(float fStartTime)
		{
			m_isStop = false;

			m_fPlayTime = fStartTime;

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
			m_motionRecorder.Clear(fStartTime);
		}

		const math::Transform* MotionPlayer::GetTransform(const string::StringID& strBoneName) const
		{
			return m_motionRecorder.GetTransform(strBoneName);
		}
	}
}