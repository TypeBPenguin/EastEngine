#include "stdafx.h"
#include "Motion.h"

namespace eastengine
{
	namespace graphics
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

		void Motion::Bone::Update(float fInterval, IMotionRecorder* pRecorder, float fPlayTime, bool isInverse) const
		{
			if (m_vecKeyframes.empty() == true)
				return;

			math::Transform transform;

			if (fPlayTime <= m_vecKeyframes.front().fTime)
			{
				if (isInverse == true)
				{
					transform = m_vecKeyframes.back().transform;
				}
				else
				{
					transform = m_vecKeyframes.front().transform;
				}
			}
			else if (fPlayTime >= m_vecKeyframes.back().fTime)
			{
				if (isInverse == true)
				{
					transform = m_vecKeyframes.front().transform;
				}
				else
				{
					transform = m_vecKeyframes.back().transform;
				}
			}
			else
			{
				const size_t nSize = m_vecKeyframes.size() - 1;
				const float fDiffTime = fPlayTime - m_vecKeyframes.front().fTime;
				uint32_t nIntervalCount = static_cast<uint32_t>(fDiffTime / fInterval);

				if (isInverse == true)
				{
					bool isSuccess = false;
					while (0 < nIntervalCount && nIntervalCount <= nSize)
					{
						const Keyframe& keyframe1 = m_vecKeyframes[nIntervalCount - 1];
						const Keyframe& keyframe2 = m_vecKeyframes[nIntervalCount];

						if (keyframe1.fTime <= fPlayTime && fPlayTime <= keyframe2.fTime)
						{
							float lerpPercent = (fPlayTime - keyframe1.fTime) / (keyframe2.fTime - keyframe1.fTime);

							math::Vector3::Lerp(keyframe1.transform.scale, keyframe2.transform.scale, lerpPercent, transform.scale);
							math::Quaternion::Lerp(keyframe1.transform.rotation, keyframe2.transform.rotation, lerpPercent, transform.rotation);
							math::Vector3::Lerp(keyframe1.transform.position, keyframe2.transform.position, lerpPercent, transform.position);

							isSuccess = true;

							break;
						}
						else if (fPlayTime < keyframe1.fTime)
						{
							--nIntervalCount;
						}
						else if (keyframe2.fTime < fPlayTime)
						{
							++nIntervalCount;
						}
						else
						{
							break;
						}
					}

					if (isSuccess == false)
					{
						LOG_WARNING("Performance Warning, Please Check Motion Update Algorithm[Inverse]");

						for (size_t i = nSize; i >= 1; --i)
						{
							const Keyframe& keyframe1 = m_vecKeyframes[i - 1];
							const Keyframe& keyframe2 = m_vecKeyframes[i];

							if (keyframe1.fTime <= fPlayTime && fPlayTime <= keyframe2.fTime)
							{
								float lerpPercent = (fPlayTime - keyframe1.fTime) / (keyframe2.fTime - keyframe1.fTime);

								math::Vector3::Lerp(keyframe1.transform.scale, keyframe2.transform.scale, lerpPercent, transform.scale);
								math::Quaternion::Lerp(keyframe1.transform.rotation, keyframe2.transform.rotation, lerpPercent, transform.rotation);
								math::Vector3::Lerp(keyframe1.transform.position, keyframe2.transform.position, lerpPercent, transform.position);

								break;
							}
						}
					}
				}
				else
				{
					bool isSuccess = false;
					while (nIntervalCount < nSize)
					{
						const Keyframe& keyframe1 = m_vecKeyframes[nIntervalCount];
						const Keyframe& keyframe2 = m_vecKeyframes[nIntervalCount + 1];

						if (keyframe1.fTime <= fPlayTime && fPlayTime <= keyframe2.fTime)
						{
							float lerpPercent = (fPlayTime - keyframe1.fTime) / (keyframe2.fTime - keyframe1.fTime);

							math::Vector3::Lerp(keyframe1.transform.scale, keyframe2.transform.scale, lerpPercent, transform.scale);
							math::Quaternion::Lerp(keyframe1.transform.rotation, keyframe2.transform.rotation, lerpPercent, transform.rotation);
							math::Vector3::Lerp(keyframe1.transform.position, keyframe2.transform.position, lerpPercent, transform.position);

							isSuccess = true;

							break;
						}
						else if (fPlayTime < keyframe1.fTime)
						{
							--nIntervalCount;
						}
						else if (keyframe2.fTime < fPlayTime)
						{
							++nIntervalCount;
						}
						else
						{
							break;
						}
					}

					if (isSuccess == false)
					{
						LOG_WARNING("Performance Warning, Please Check Motion Update Algorithm");

						for (size_t i = 0; i < nSize; ++i)
						{
							const Keyframe& keyframe1 = m_vecKeyframes[i];
							const Keyframe& keyframe2 = m_vecKeyframes[i + 1];

							if (keyframe1.fTime <= fPlayTime && fPlayTime <= keyframe2.fTime)
							{
								float lerpPercent = (fPlayTime - keyframe1.fTime) / (keyframe2.fTime - keyframe1.fTime);

								math::Vector3::Lerp(keyframe1.transform.scale, keyframe2.transform.scale, lerpPercent, transform.scale);
								math::Quaternion::Lerp(keyframe1.transform.rotation, keyframe2.transform.rotation, lerpPercent, transform.rotation);
								math::Vector3::Lerp(keyframe1.transform.position, keyframe2.transform.position, lerpPercent, transform.position);

								break;
							}
						}
					}
				}
			}

			pRecorder->SetTransform(m_strBoneName, transform);
		}

		Motion::Motion(Key key)
			: m_key(key)
		{
		}

		//Motion::Motion(const Motion& source)
		//	: m_key(source.m_key)
		//	, m_strName(source.m_strName)
		//	, m_strFilePath(source.m_strFilePath)
		//	, m_vecBones(source.m_vecBones)
		//	, m_umapBones(source.m_umapBones)
		//	, m_fStartTime(source.m_fStartTime)
		//	, m_fEndTime(source.m_fEndTime)
		//	, m_fFrameInterval(source.m_fFrameInterval)
		//{
		//}

		Motion::Motion(Motion&& source) noexcept
			: m_key(std::move(source.m_key))
			, m_strName(std::move(source.m_strName))
			, m_strFilePath(std::move(source.m_strFilePath))
			, m_vecBones(std::move(source.m_vecBones))
			, m_umapBones(std::move(source.m_umapBones))
			, m_fStartTime(std::move(source.m_fStartTime))
			, m_fEndTime(std::move(source.m_fEndTime))
			, m_fFrameInterval(std::move(source.m_fFrameInterval))
			, m_vecEvents(std::move(source.m_vecEvents))
		{
		}

		Motion::~Motion()
		{
			std::for_each(m_vecEvents.begin(), m_vecEvents.end(), DeleteSTLObject());
			m_vecEvents.clear();
		}

		void Motion::Update(IMotionRecorder* pRecorder, float fPlayTime, bool isInverse, bool isEnableTransformUpdate) const
		{
			if (isEnableTransformUpdate == true)
			{
				std::for_each(m_vecBones.begin(), m_vecBones.end(), [&](const Bone& bone)
				{
					bone.Update(m_fFrameInterval, pRecorder, fPlayTime, isInverse);
				});
			}

			std::for_each(m_vecEvents.begin(), m_vecEvents.end(), [&](const IMotionEvent* pEvent)
			{
				if (pRecorder->GetLastPlayTime() <= pEvent->fTime && pEvent->fTime < fPlayTime)
				{
					pRecorder->PushEvent(pEvent);
				}
			});
			pRecorder->SetLastPlayTime(fPlayTime);
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
			m_vecBones.emplace_back(strBoneName, vecKeyframes, m_fFrameInterval);
			m_umapBones.emplace(strBoneName, &m_vecBones.back());
		}

		void Motion::SetInfo(float fStartTime, float fEndTime, float fFrameInterval)
		{
			m_fStartTime = fStartTime;
			m_fEndTime = fEndTime;
			m_fFrameInterval = fFrameInterval;
		}
	}
}