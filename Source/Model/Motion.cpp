#include "stdafx.h"
#include "Motion.h"

namespace EastEngine
{
	namespace Graphics
	{
		static boost::object_pool<Motion> s_poolMotion;
		static boost::object_pool<Motion::Bone> g_poolMotionBone;

		void MotionDestroyer(Motion* pMotion)
		{
			s_poolMotion.destroy(pMotion);
		}

		std::shared_ptr<Motion> AllocateMotion(const String::StringID& strName, const char* strFilePath)
		{
			return std::shared_ptr<Motion>(s_poolMotion.construct(strName, strFilePath), MotionDestroyer);
		}

		Motion::Bone::Bone(const String::StringID& strBoneName, const std::vector<Keyframe>& _vecKeyframes)
			: m_strBoneName(strBoneName)
		{
			m_vecKeyframes = std::move(_vecKeyframes);
		}

		Motion::Bone::~Bone()
		{
		}

		void Motion::Bone::Update(IMotionPlayer* pPlayInfo)
		{
			if (m_vecKeyframes.empty() == true)
				return;

			float fPlayTime = pPlayInfo->GetPlayTime();
			Keyframe keyframe;

			if (fPlayTime < m_vecKeyframes.front().fTimePos)
			{
				keyframe = m_vecKeyframes.front();

				if (pPlayInfo->IsInverse() == true)
				{
					pPlayInfo->SetCaching(m_strBoneName, m_vecKeyframes.size() - 1);
				}
				else
				{
					pPlayInfo->SetCaching(m_strBoneName, 0);
				}
			}
			else if (fPlayTime > m_vecKeyframes.back().fTimePos)
			{
				keyframe = m_vecKeyframes.back();

				if (pPlayInfo->IsInverse() == true)
				{
					pPlayInfo->SetCaching(m_strBoneName, m_vecKeyframes.size() - 1);
				}
				else
				{
					pPlayInfo->SetCaching(m_strBoneName, 0);
				}
			}
			else
			{
				if (pPlayInfo->IsInverse() == true)
				{
					int nKeyframeIndex = pPlayInfo->GetCaching(m_strBoneName);
					if (nKeyframeIndex == -1 || nKeyframeIndex >= static_cast<int>(m_vecKeyframes.size()))
					{
						nKeyframeIndex = m_vecKeyframes.size() - 1;
					}
					
					for (int i = nKeyframeIndex; i >= 1; --i)
					{
						const Keyframe& keyframe1 = m_vecKeyframes[i - 1];
						const Keyframe& keyframe2 = m_vecKeyframes[i];

						if (keyframe1.fTimePos <= fPlayTime && fPlayTime <= keyframe2.fTimePos)
						{
							float lerpPercent = (fPlayTime - fPlayTime) / (keyframe2.fTimePos - keyframe1.fTimePos);

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
					int nKeyframeIndex = pPlayInfo->GetCaching(m_strBoneName);
					if (nKeyframeIndex == -1)
					{
						nKeyframeIndex = 0;
					}

					uint32_t nSize = m_vecKeyframes.size() - 1;
					for (uint32_t i = nKeyframeIndex; i < nSize; ++i)
					{
						const Keyframe& keyframe1 = m_vecKeyframes[i];
						const Keyframe& keyframe2 = m_vecKeyframes[i + 1];

						if (keyframe1.fTimePos <= fPlayTime && fPlayTime <= keyframe2.fTimePos)
						{
							float lerpPercent = (fPlayTime - keyframe1.fTimePos) / (keyframe2.fTimePos - keyframe1.fTimePos);

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
		{
		}

		Motion::~Motion()
		{
			std::for_each(m_vecBones.begin(), m_vecBones.end(), [](Bone* pBone)
			{
				g_poolMotionBone.destroy(pBone);
			});
			m_vecBones.clear();
			m_umapBones.clear();
		}

		void Motion::Update(IMotionPlayer* pPlayInfo)
		{
			for (auto& iter : m_umapBones)
			{
				iter.second->Update(pPlayInfo);
			}
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
			Bone* pBone = g_poolMotionBone.construct(strBoneName, vecKeyframes);
			m_vecBones.emplace_back(pBone);
			m_umapBones.emplace(strBoneName, pBone);
		}

		void Motion::CalcClipTime()
		{
			float fStartTime = D3D11_FLOAT32_MAX;
			float fEndTime = 0.f;
			for (const auto& pBone : m_vecBones)
			{
				fStartTime = Math::Min(fStartTime, pBone->GetStartTime());
				fEndTime = Math::Max(fEndTime, pBone->GetEndTime());
			}

			m_fStartTime = fStartTime;
			m_fEndTime = fEndTime;
		}
	}
}