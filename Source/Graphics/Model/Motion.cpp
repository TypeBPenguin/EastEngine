#include "stdafx.h"
#include "Motion.h"

#include "CommonLib/FileStream.h"

#include "MotionRecorder.h"

namespace est
{
	namespace graphics
	{
		Motion::Bone::Bone(const string::StringID& boneName, std::vector<Keyframe>&& vecKeyframes, float frameInterval)
			: m_boneName(boneName)
			, m_frameInterval(frameInterval)
			, m_keyframes(std::move(vecKeyframes))
		{
		}

		Motion::Bone::Bone(const string::StringID& boneName, const Keyframe* pKeyframes, size_t keyframeCount, float frameInterval)
			: m_boneName(boneName)
			, m_frameInterval(frameInterval)
			, m_keyframes(pKeyframes, pKeyframes + keyframeCount)
		{
		}

		Motion::Bone::~Bone()
		{
		}

		void Motion::Bone::Update(float interval, MotionRecorder* pRecorder, float playTime, bool isInverse) const
		{
			if (m_keyframes.empty() == true)
				return;

			math::Transform transform;

			if (playTime <= m_keyframes.front().time)
			{
				if (isInverse == true)
				{
					transform = m_keyframes.back().transform;
				}
				else
				{
					transform = m_keyframes.front().transform;
				}
			}
			else if (playTime >= m_keyframes.back().time)
			{
				if (isInverse == true)
				{
					transform = m_keyframes.front().transform;
				}
				else
				{
					transform = m_keyframes.back().transform;
				}
			}
			else
			{
				const size_t keyframeCount = m_keyframes.size() - 1;
				const float diffTime = playTime - m_keyframes.front().time;
				size_t expectedIndex = static_cast<size_t>(diffTime / interval);

				bool isSuccessFind = false;

				if (isInverse == true)
				{
					while (0 < expectedIndex && expectedIndex <= keyframeCount)
					{
						const Keyframe& keyframe1 = m_keyframes[expectedIndex - 1];
						const Keyframe& keyframe2 = m_keyframes[expectedIndex];

						if (keyframe1.time <= playTime && playTime <= keyframe2.time)
						{
							const float lerpPercent = (playTime - keyframe1.time) / (keyframe2.time - keyframe1.time);
							math::Transform::Lerp(keyframe1.transform, keyframe2.transform, lerpPercent, transform);
							isSuccessFind = true;
							break;
						}
						else if (playTime < keyframe1.time)
						{
							--expectedIndex;
						}
						else if (keyframe2.time < playTime)
						{
							++expectedIndex;
						}
						else
						{
							break;
						}
					}
				}
				else
				{
					while (expectedIndex < keyframeCount)
					{
						const Keyframe& keyframe1 = m_keyframes[expectedIndex];
						const Keyframe& keyframe2 = m_keyframes[expectedIndex + 1];

						if (keyframe1.time <= playTime && playTime <= keyframe2.time)
						{
							const float lerpPercent = (playTime - keyframe1.time) / (keyframe2.time - keyframe1.time);
							math::Transform::Lerp(keyframe1.transform, keyframe2.transform, lerpPercent, transform);
							isSuccessFind = true;
							break;
						}
						else if (playTime < keyframe1.time)
						{
							--expectedIndex;
						}
						else if (keyframe2.time < playTime)
						{
							++expectedIndex;
						}
						else
						{
							break;
						}
					}
				}

				if (isSuccessFind == false)
				{
					// binary search
					size_t start = 0;
					size_t end = keyframeCount;

					while (true)
					{
						const size_t distance = end - start;
						if (distance < 1)
						{
							LOG_WARNING(L"Performance Warning, Please Check Motion Update Algorithm");
							break;
						}
						else if (distance == 1)
						{
							const Keyframe& keyframe1 = m_keyframes[start];
							const Keyframe& keyframe2 = m_keyframes[end];

							if (keyframe1.time <= playTime && playTime <= keyframe2.time)
							{
								const float lerpPercent = (playTime - keyframe1.time) / (keyframe2.time - keyframe1.time);
								math::Transform::Lerp(keyframe1.transform, keyframe2.transform, lerpPercent, transform);
								break;
							}
							else
							{
								LOG_WARNING(L"Performance Warning, Please Check Motion Update Algorithm");
								break;
							}
						}

						const size_t pivot = (end + start) / 2;
						{
							const Keyframe& keyframe1 = m_keyframes[start];
							const Keyframe& keyframe2 = m_keyframes[pivot];

							if (keyframe1.time <= playTime && playTime <= keyframe2.time)
							{
								end = pivot;
								continue;
							}
						}
						{
							const Keyframe& keyframe1 = m_keyframes[pivot];
							const Keyframe& keyframe2 = m_keyframes[end];

							if (keyframe1.time <= playTime && playTime <= keyframe2.time)
							{
								start = pivot;
								continue;
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
			, m_name(std::move(source.m_name))
			, m_filePath(std::move(source.m_filePath))
			, m_bones(std::move(source.m_bones))
			, m_rmapBones(std::move(source.m_rmapBones))
			, m_startTime(std::move(source.m_startTime))
			, m_endTime(std::move(source.m_endTime))
			, m_frameInterval(std::move(source.m_frameInterval))
			, m_events(std::move(source.m_events))
		{
		}

		Motion::~Motion()
		{
			m_events.clear();
		}

		const IMotion::IBone* Motion::GetBone(const string::StringID& boneName) const
		{
			auto iter = m_rmapBones.find(boneName);
			if (iter != m_rmapBones.end())
				return iter->second;

			return nullptr;
		}

		void Motion::Update(MotionRecorder* pRecorder, float playTime, bool isInverse) const
		{
			if (math::IsEqual(pRecorder->GetLastPlayTime(), playTime) == true && pRecorder->IsEmpty() == false)
				return;

			std::for_each(m_bones.begin(), m_bones.end(), [&](const Bone& bone)
				{
					bone.Update(m_frameInterval, pRecorder, playTime, isInverse);
				});

			std::for_each(m_events.begin(), m_events.end(), [&](const std::unique_ptr<const IMotionEvent>& pEvent)
				{
					if (pRecorder->GetLastPlayTime() <= pEvent->time && pEvent->time < playTime)
					{
						pRecorder->PushEvent(pEvent.get());
					}
				});
			pRecorder->SetLastPlayTime(playTime);
		}

		void Motion::AddBoneKeyframes(const string::StringID& boneName, std::vector<Keyframe>&& vecKeyframes)
		{
			m_bones.emplace_back(boneName, std::move(vecKeyframes), m_frameInterval);
			m_rmapBones.emplace(boneName, &m_bones.back());
		}

		void Motion::SetInfo(float startTime, float endTime, float frameInterval)
		{
			m_startTime = startTime;
			m_endTime = endTime;
			m_frameInterval = frameInterval;
		}

		bool Motion::LoadFile(const wchar_t* path)
		{
			file::Stream file;
			if (file.Open(path, file::eReadBinary) == false)
			{
				LOG_WARNING(L"Can't open to file : %s", path);
				return false;
			}

			BinaryReader binaryReader = file.GetBinaryReader();
			{
				const string::StringID name = binaryReader.ReadString();

				m_startTime = binaryReader;
				m_endTime = binaryReader;
				m_frameInterval = binaryReader;

				const uint32_t boneCount = binaryReader;
				m_bones.reserve(boneCount);
				m_rmapBones.reserve(boneCount);

				for (uint32_t i = 0; i < boneCount; ++i)
				{
					const string::StringID boneName = binaryReader.ReadString();

					const uint32_t keyFrameCount = binaryReader;
					const Keyframe* pKeyframes = &binaryReader.Read<Keyframe>(keyFrameCount);

					m_bones.emplace_back(boneName, pKeyframes, keyFrameCount, m_frameInterval);
					m_rmapBones.emplace(boneName, &m_bones.back());
				}
			}

			file.Close();

			return true;
		}
	}
}