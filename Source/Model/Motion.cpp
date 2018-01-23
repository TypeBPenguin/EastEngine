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

		void Motion::Bone::Update(IMotionRecoder* pRecoder, float fPlayTime, bool isInverse)
		{
			if (m_vecKeyframes.empty() == true)
				return;

			Keyframe keyframe;

			if (fPlayTime <= m_vecKeyframes.front().fTime)
			{
				if (isInverse == true)
				{
					keyframe = m_vecKeyframes.back();

					pRecoder->SetCaching(m_strBoneName, m_vecKeyframes.size() - 1);
				}
				else
				{
					keyframe = m_vecKeyframes.back();

					pRecoder->SetCaching(m_strBoneName, 0);
				}
			}
			else if (fPlayTime >= m_vecKeyframes.back().fTime)
			{
				if (isInverse == true)
				{
					keyframe = m_vecKeyframes.back();

					pRecoder->SetCaching(m_strBoneName, m_vecKeyframes.size() - 1);
				}
				else
				{
					keyframe = m_vecKeyframes.back();

					pRecoder->SetCaching(m_strBoneName, 0);
				}
			}
			else
			{
				if (isInverse == true)
				{
					size_t nKeyframeIndex = pRecoder->GetCaching(m_strBoneName);
					if (nKeyframeIndex == IMotionRecoder::eInvalidCachingIndex || nKeyframeIndex <= 1 || nKeyframeIndex >= m_vecKeyframes.size())
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

							Math::Vector3::Lerp(keyframe1.transform.scale, keyframe2.transform.scale, lerpPercent, keyframe.transform.scale);
							Math::Quaternion::Lerp(keyframe1.transform.rotation, keyframe2.transform.rotation, lerpPercent, keyframe.transform.rotation);
							Math::Vector3::Lerp(keyframe1.transform.position, keyframe2.transform.position, lerpPercent, keyframe.transform.position);

							pRecoder->SetCaching(m_strBoneName, i);

							break;
						}
					}
				}
				else
				{
					size_t nKeyframeIndex = pRecoder->GetCaching(m_strBoneName);
					if (nKeyframeIndex == IMotionRecoder::eInvalidCachingIndex)
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

							Math::Vector3::Lerp(keyframe1.transform.scale, keyframe2.transform.scale, lerpPercent, keyframe.transform.scale);
							Math::Quaternion::Lerp(keyframe1.transform.rotation, keyframe2.transform.rotation, lerpPercent, keyframe.transform.rotation);
							Math::Vector3::Lerp(keyframe1.transform.position, keyframe2.transform.position, lerpPercent, keyframe.transform.position);

							pRecoder->SetCaching(m_strBoneName, i);

							break;
						}
					}
				}
			}

			pRecoder->SetKeyframe(m_strBoneName, keyframe);
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

		void Motion::Update(IMotionRecoder* pRecoder, float fPlayTime, bool isInverse)
		{
			std::for_each(m_clnBones.begin(), m_clnBones.end(), [&](Bone& bone)
			{
				bone.Update(pRecoder, fPlayTime, isInverse);
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