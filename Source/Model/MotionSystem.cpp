#include "stdafx.h"
#include "MotionSystem.h"

namespace EastEngine
{
	namespace Graphics
	{
		MotionPlayer::MotionPlayer()
			: m_fPlayTime(0.f)
		{
		}

		MotionPlayer::~MotionPlayer()
		{
			m_umapKeyframeIndexCaching.clear();
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

		size_t MotionPlayer::GetCaching(const String::StringID& strBoneName)
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

		const IMotion::Keyframe* MotionPlayer::GetKeyframe(const String::StringID& strBoneName)
		{
			auto iter = m_umapKeyframe.find(strBoneName);
			if (iter != m_umapKeyframe.end())
				return &iter->second;

			return nullptr;
		}

		void MotionPlayer::Reset()
		{
			m_fPlayTime = 0.f;

			m_motionState.Reset();

			for (auto& iter : m_umapKeyframeIndexCaching)
			{
				iter.second.value = eInvalidCachingIndex;
			}
		}

		MotionSystem::MotionSystem(IModel* pModel)
			: m_fPlayTime(0.f)
			, m_fPlayTimeSub(0.f)
			, m_fBlemdTime(0.f)
			, m_fStopTime(0.f)
			, m_fStopTimeCheck(0.f)
			, m_isStop(false)
			, m_pMotion(nullptr)
			, m_pMotionSub(nullptr)
			, m_pModel(pModel)
		{
		}

		MotionSystem::~MotionSystem()
		{
			m_pMotion = nullptr;
			m_pMotionSub = nullptr;
		}

		void MotionSystem::Update(float fElapsedTime, ISkeletonInstance* pSkeletonInst)
		{
			if (pSkeletonInst == nullptr)
				return;

			if (m_pMotion == nullptr)
				return;

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
				if (pSkeletonInst != nullptr)
				{
					float fPlayTime = 0.f;
					if (m_motionPlayer.IsInverse() == true)
					{
						fPlayTime = m_pMotion->GetEndTime() - m_fPlayTime;
					}
					else
					{
						fPlayTime = m_fPlayTime;
					}

					bool isBlending = false;
					float fPlayTimeSub = 0.f;
					if (m_pMotionSub != nullptr &&
						m_motionPlayer.GetBlendWeight() > 0.f &&
						m_motionPlayer.GetBlendTime() > m_fBlemdTime)
					{
						isBlending = true;

						if (m_motionPlayerSub.IsInverse() == true)
						{
							fPlayTimeSub = m_pMotionSub->GetEndTime() - m_fPlayTimeSub;
						}
						else
						{
							fPlayTimeSub = m_fPlayTimeSub;
						}
					}

					const uint32_t nBoneCount = pSkeletonInst->GetBoneCount();
					if (nBoneCount > 0)
					{
						pSkeletonInst->SetDirty();

						m_motionPlayer.SetPlayTime(fPlayTime);

						m_pMotion->Update(&m_motionPlayer);

						if (isBlending == true)
						{
							m_motionPlayerSub.SetPlayTime(fPlayTimeSub);

							m_pMotionSub->Update(&m_motionPlayerSub);

							const float fBlendWeight = 1.f - (1.f / m_motionPlayer.GetBlendWeight() * m_fBlemdTime);

							for (uint32_t i = 0; i < nBoneCount; ++i)
							{
								ISkeletonInstance::IBone* pBone = pSkeletonInst->GetBone(i);
								if (pBone == nullptr)
									break;

								const String::StringID& strBoneName = pBone->GetName();

								const IMotion::Keyframe* pKeyframe1 = m_motionPlayer.GetKeyframe(strBoneName);
								const IMotion::Keyframe* pKeyframe2 = m_motionPlayerSub.GetKeyframe(strBoneName);

								Math::Matrix matMotion;
								if (pKeyframe1 != nullptr && pKeyframe2 == nullptr)
								{
									Math::Matrix::Compose(pKeyframe1->f3Pos, pKeyframe1->quatRotation, pKeyframe1->f3Scale, matMotion);
								}
								else if (pKeyframe1 == nullptr && pKeyframe2 != nullptr)
								{
									Math::Matrix::Compose(pKeyframe2->f3Pos, pKeyframe2->quatRotation, pKeyframe2->f3Scale, matMotion);
								}
								else if (pKeyframe1 != nullptr && pKeyframe2 != nullptr)
								{
									IMotion::Keyframe keyframe;

									Math::Vector3::Lerp(pKeyframe1->f3Pos, pKeyframe2->f3Pos, fBlendWeight, keyframe.f3Pos);
									Math::Vector3::Lerp(pKeyframe1->f3Scale, pKeyframe2->f3Scale, fBlendWeight, keyframe.f3Scale);
									Math::Quaternion::Slerp(pKeyframe1->quatRotation, pKeyframe2->quatRotation, fBlendWeight, keyframe.quatRotation);

									Math::Matrix::Compose(keyframe.f3Scale, keyframe.quatRotation, keyframe.f3Pos, matMotion);
								}
								else
								{
									matMotion = Math::Matrix::Identity;
								}

								pBone->SetMotionData(matMotion);
							}
						}
						else
						{
							for (uint32_t i = 0; i < nBoneCount; ++i)
							{
								ISkeletonInstance::IBone* pBone = pSkeletonInst->GetBone(i);
								if (pBone == nullptr)
									break;

								const String::StringID& strBoneName = pBone->GetName();
								const IMotion::Keyframe* pKeyframe1 = m_motionPlayer.GetKeyframe(strBoneName);
								if (pKeyframe1 != nullptr)
								{
									Math::Matrix matMotion;
									Math::Matrix::Compose(pKeyframe1->f3Scale, pKeyframe1->quatRotation, pKeyframe1->f3Pos, matMotion);
									pBone->SetMotionData(matMotion);
								}
								else
								{
									pBone->SetMotionData(Math::Matrix::Identity);
								}
							}
						}
					}
				}

				float fEndTime = m_pMotion->GetEndTime();
				if (m_fPlayTime >= fEndTime)
				{
					if (m_motionPlayer.IsLoop() == false)
					{
						pSkeletonInst->SetIdentity();

						m_pMotion->DecreaseReference();
						m_pMotion = nullptr;

						return;
					}
					else
					{
						m_fPlayTime -= fEndTime;
					}
				}

				m_fBlemdTime += fElapsedTime;
				m_fPlayTime += fElapsedTime * m_motionPlayer.GetPlaySpeed();
				m_fPlayTimeSub += fElapsedTime * m_motionPlayerSub.GetPlaySpeed();
			}
			else
			{
				m_pMotion->DecreaseReference();
				m_pMotion = nullptr;
			}
		}

		void MotionSystem::Play(IMotion* pMotion, const MotionState* pMotionState)
		{
			if (m_pMotionSub != nullptr)
			{
				m_pMotionSub->DecreaseReference();
				m_pMotionSub = nullptr;
			}

			m_pMotionSub = m_pMotion;
			m_fPlayTimeSub = m_fPlayTime;
			m_motionPlayerSub = m_motionPlayer;

			m_pMotion = pMotion;

			if (m_pMotion != nullptr)
			{
				m_pMotion->IncreaseReference();

				m_fPlayTime = m_pMotion->GetStartTime();
			}
			else
			{
				m_fPlayTime = 0.f;
			}

			m_fBlemdTime = 0.f;

			m_motionPlayer.Reset();

			if (pMotionState != nullptr)
			{
				m_motionPlayer.SetMotionState(*pMotionState);
			}
		}

		void MotionSystem::Stop(float fStopTime)
		{
			m_fStopTimeCheck = fStopTime;
			m_fStopTime = 0.f;
			m_isStop = true;
		}
	}
}