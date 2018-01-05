#include "stdafx.h"
#include "Motion.h"

namespace EastEngine
{
	namespace Graphics
	{
		Motion::Bone::Bone(const String::StringID& strBoneName, const std::vector<Keyframe>& _vecKeyframes, float fFrameInterval)
			: m_strBoneName(strBoneName)
			, m_fFrameInterval(fFrameInterval)
		{
			m_vecKeyframes = std::move(_vecKeyframes);
		}

		Motion::Bone::~Bone()
		{
		}

		void Motion::Bone::Update(float fPlayTime, IMotionPlayer* pPlayInfo)
		{
			if (m_vecKeyframes.empty() == true)
				return;

			Keyframe keyframe;

			if (fPlayTime < m_vecKeyframes.front().fTime)
			{
				if (pPlayInfo->IsInverse() == true)
				{
					keyframe = m_vecKeyframes.back();

					pPlayInfo->SetCaching(m_strBoneName, m_vecKeyframes.size() - 1);
				}
				else
				{
					keyframe = m_vecKeyframes.back();

					pPlayInfo->SetCaching(m_strBoneName, 0);
				}
			}
			else if (fPlayTime > m_vecKeyframes.back().fTime)
			{
				if (pPlayInfo->IsInverse() == true)
				{
					keyframe = m_vecKeyframes.back();

					pPlayInfo->SetCaching(m_strBoneName, m_vecKeyframes.size() - 1);
				}
				else
				{
					keyframe = m_vecKeyframes.back();

					pPlayInfo->SetCaching(m_strBoneName, 0);
				}
			}
			else
			{
				if (pPlayInfo->IsInverse() == true)
				{
					size_t nKeyframeIndex = pPlayInfo->GetCaching(m_strBoneName);
					if (nKeyframeIndex == IMotionPlayer::eInvalidCachingIndex || nKeyframeIndex <= 1 || nKeyframeIndex >= m_vecKeyframes.size())
					{
						nKeyframeIndex = m_vecKeyframes.size() - 1;
					}
					
					for (size_t i = nKeyframeIndex; i >= 1; --i)
					{
						const Keyframe& keyframe1 = m_vecKeyframes[i - 1];
						const Keyframe& keyframe2 = m_vecKeyframes[i];

						if (keyframe1.fTime <= fPlayTime && fPlayTime <= keyframe2.fTime)
						{
							float lerpPercent = (fPlayTime - keyframe1.fTime) / (keyframe2.fTime - keyframe1.fTime);

							Math::Vector3::Lerp(keyframe1.f3Pos, keyframe2.f3Pos, lerpPercent, keyframe.f3Pos);
							Math::Vector3::Lerp(keyframe1.f3Scale, keyframe2.f3Scale, lerpPercent, keyframe.f3Scale);
							Math::Quaternion::Lerp(keyframe1.quatRotation, keyframe2.quatRotation, lerpPercent, keyframe.quatRotation);

							pPlayInfo->SetCaching(m_strBoneName, i);

							break;
						}
					}
				}
				else
				{
					size_t nKeyframeIndex = pPlayInfo->GetCaching(m_strBoneName);
					if (nKeyframeIndex == IMotionPlayer::eInvalidCachingIndex)
					{
						nKeyframeIndex = 0;
					}

					size_t nSize = m_vecKeyframes.size() - 1;
					for (size_t i = nKeyframeIndex; i < nSize; ++i)
					{
						const Keyframe& keyframe1 = m_vecKeyframes[i];
						const Keyframe& keyframe2 = m_vecKeyframes[i + 1];

						if (keyframe1.fTime <= fPlayTime && fPlayTime <= keyframe2.fTime)
						{
							float lerpPercent = (fPlayTime - keyframe1.fTime) / (keyframe2.fTime - keyframe1.fTime);

							Math::Vector3::Lerp(keyframe1.f3Pos, keyframe2.f3Pos, lerpPercent, keyframe.f3Pos);
							Math::Vector3::Lerp(keyframe1.f3Scale, keyframe2.f3Scale, lerpPercent, keyframe.f3Scale);
							Math::Quaternion::Lerp(keyframe1.quatRotation, keyframe2.quatRotation, lerpPercent, keyframe.quatRotation);

							pPlayInfo->SetCaching(m_strBoneName, i);

							break;
						}
					}
				}
			}

			pPlayInfo->SetKeyframe(m_strBoneName, keyframe);
		}

		Motion::Motion(const String::StringID& strName, const char* strFilePath)
			: m_nReferenceCount(0)
			, m_strName(strName)
			, m_strFilePath(strFilePath)
			, m_fStartTime(0.f)
			, m_fEndTime(0.f)
			, m_fFrameInterval(0.f)
		{
		}

		Motion::~Motion()
		{
			m_vecBonesIndexing.clear();
			m_umapBones.clear();
			m_clnBones.clear();
		}

		void Motion::Update(float fPlayTime, IMotionPlayer* pPlayInfo)
		{
			std::for_each(m_clnBones.begin(), m_clnBones.end(), [&](Bone& bone)
			{
				bone.Update(fPlayTime, pPlayInfo);
			});
		}

		const IMotion::IBone* Motion::GetBone(const String::StringID& strBoneName) const
		{
			auto iter = m_umapBones.find(strBoneName);
			if (iter != m_umapBones.end())
				return iter->second;

			return nullptr;
		}

		void Motion::AddBoneKeyframes(const String::StringID& strBoneName, const std::vector<Keyframe>& vecKeyframes)
		{
			auto iter_result = m_clnBones.emplace(strBoneName, vecKeyframes, m_fFrameInterval);
			Bone* pBone = &(*iter_result);
			m_vecBonesIndexing.emplace_back(pBone);
			m_umapBones.emplace(strBoneName, pBone);
		}

		void Motion::SetInfo(float fStartTime, float fEndTime, float fFrameInterval)
		{
			m_fStartTime = fStartTime;
			m_fEndTime = fEndTime;
			m_fFrameInterval = fFrameInterval;
		}
	}
}