#include "stdafx.h"
#include "Motion.h"

#include "CommonLib/FileStream.h"

namespace eastengine
{
	namespace graphics
	{
		Motion::Bone::Bone(const string::StringID& boneName, std::vector<Keyframe>&& vecKeyframes, float fFrameInterval)
			: m_boneName(boneName)
			, m_frameInterval(fFrameInterval)
			, m_vecKeyframes(std::move(vecKeyframes))
		{
		}

		Motion::Bone::Bone(const string::StringID& boneName, const Keyframe* pKeyframes, size_t keyframeCount, float fFrameInterval)
			: m_boneName(boneName)
			, m_frameInterval(fFrameInterval)
			, m_vecKeyframes(pKeyframes, pKeyframes + keyframeCount)
		{
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

							math::float3::Lerp(keyframe1.transform.scale, keyframe2.transform.scale, lerpPercent, transform.scale);
							math::Quaternion::Lerp(keyframe1.transform.rotation, keyframe2.transform.rotation, lerpPercent, transform.rotation);
							math::float3::Lerp(keyframe1.transform.position, keyframe2.transform.position, lerpPercent, transform.position);

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

								math::float3::Lerp(keyframe1.transform.scale, keyframe2.transform.scale, lerpPercent, transform.scale);
								math::Quaternion::Lerp(keyframe1.transform.rotation, keyframe2.transform.rotation, lerpPercent, transform.rotation);
								math::float3::Lerp(keyframe1.transform.position, keyframe2.transform.position, lerpPercent, transform.position);

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

							math::float3::Lerp(keyframe1.transform.scale, keyframe2.transform.scale, lerpPercent, transform.scale);
							math::Quaternion::Lerp(keyframe1.transform.rotation, keyframe2.transform.rotation, lerpPercent, transform.rotation);
							math::float3::Lerp(keyframe1.transform.position, keyframe2.transform.position, lerpPercent, transform.position);

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

								math::float3::Lerp(keyframe1.transform.scale, keyframe2.transform.scale, lerpPercent, transform.scale);
								math::Quaternion::Lerp(keyframe1.transform.rotation, keyframe2.transform.rotation, lerpPercent, transform.rotation);
								math::float3::Lerp(keyframe1.transform.position, keyframe2.transform.position, lerpPercent, transform.position);

								break;
							}
						}
					}
				}
			}

			pRecorder->SetTransform(m_boneName, transform);
		}

		Motion::Motion(Key key)
			: m_key(key)
		{
		}

		Motion::Motion(Motion&& source) noexcept
			: m_key(std::move(source.m_key))
			, m_strName(std::move(source.m_strName))
			, m_strFilePath(std::move(source.m_strFilePath))
			, m_vecBones(std::move(source.m_vecBones))
			, m_rmapBones(std::move(source.m_rmapBones))
			, m_startTime(std::move(source.m_startTime))
			, m_endTime(std::move(source.m_endTime))
			, m_frameInterval(std::move(source.m_frameInterval))
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
					bone.Update(m_frameInterval, pRecorder, fPlayTime, isInverse);
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

		const IMotion::IBone* Motion::GetBone(const string::StringID& boneName) const
		{
			auto iter = m_rmapBones.find(boneName);
			if (iter != m_rmapBones.end())
				return iter->second;

			return nullptr;
		}

		void Motion::AddBoneKeyframes(const string::StringID& boneName, std::vector<Keyframe>&& vecKeyframes)
		{
			m_vecBones.emplace_back(boneName, std::move(vecKeyframes), m_frameInterval);
			m_rmapBones.emplace(boneName, &m_vecBones.back());
		}

		void Motion::SetInfo(float fStartTime, float fEndTime, float fFrameInterval)
		{
			m_startTime = fStartTime;
			m_endTime = fEndTime;
			m_frameInterval = fFrameInterval;
		}

		bool Motion::LoadFile(const char* path)
		{
			file::Stream file;
			if (file.Open(path, file::eReadBinary) == false)
			{
				LOG_WARNING("Can't open to file : %s", path);
				return false;
			}

			const BYTE* pBuffer = file.GetBuffer();
			{
				const string::StringID name = file::Stream::ToString(&pBuffer);

				m_startTime = *file::Stream::To<float>(&pBuffer);
				m_endTime = *file::Stream::To<float>(&pBuffer);
				m_frameInterval = *file::Stream::To<float>(&pBuffer);

				const uint32_t boneCount = *file::Stream::To<uint32_t>(&pBuffer);
				m_vecBones.reserve(boneCount);
				m_rmapBones.reserve(boneCount);

				for (uint32_t i = 0; i < boneCount; ++i)
				{
					const string::StringID boneName = file::Stream::ToString(&pBuffer);

					const uint32_t keyFrameCount = *file::Stream::To<uint32_t>(&pBuffer);
					const Keyframe* pKeyframes = file::Stream::To<Keyframe>(&pBuffer, keyFrameCount);

					m_vecBones.emplace_back(boneName, pKeyframes, keyFrameCount, m_frameInterval);
					m_rmapBones.emplace(boneName, &m_vecBones.back());
				}
			}

			file.Close();

			return true;
		}
	}
}