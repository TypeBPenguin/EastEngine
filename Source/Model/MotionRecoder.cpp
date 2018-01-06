#include "stdafx.h"
#include "MotionRecoder.h"

namespace EastEngine
{
	namespace Graphics
	{
		MotionRecoder::MotionRecoder()
		{
		}

		MotionRecoder::~MotionRecoder()
		{
		}

		void MotionRecoder::SetCaching(const String::StringID& strBoneName, size_t nIndex)
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

		size_t MotionRecoder::GetCaching(const String::StringID& strBoneName) const
		{
			auto iter = m_umapKeyframeIndexCaching.find(strBoneName);
			if (iter != m_umapKeyframeIndexCaching.end())
				return iter->second.value;

			return eInvalidCachingIndex;
		}

		void MotionRecoder::SetKeyframe(const String::StringID& strBoneName, const IMotion::Keyframe& keyframe)
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

		const IMotion::Keyframe* MotionRecoder::GetKeyframe(const String::StringID& strBoneName) const
		{
			auto iter = m_umapKeyframe.find(strBoneName);
			if (iter != m_umapKeyframe.end())
				return &iter->second;

			return nullptr;
		}

		void MotionRecoder::Clear()
		{
			for (auto& iter : m_umapKeyframeIndexCaching)
			{
				iter.second.value = eInvalidCachingIndex;
			}
		}
	}
}