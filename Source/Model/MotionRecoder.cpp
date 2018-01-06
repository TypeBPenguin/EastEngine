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
			auto iter = m_umapMotionData.find(strBoneName);
			if (iter != m_umapMotionData.end())
			{
				iter->second.caching.value = nIndex;
			}
			else
			{
				m_umapMotionData.emplace(strBoneName, nIndex);
			}
		}

		size_t MotionRecoder::GetCaching(const String::StringID& strBoneName) const
		{
			auto iter = m_umapMotionData.find(strBoneName);
			if (iter != m_umapMotionData.end())
				return iter->second.caching.value;

			return eInvalidCachingIndex;
		}

		void MotionRecoder::SetKeyframe(const String::StringID& strBoneName, const IMotion::Keyframe& keyframe)
		{
			auto iter = m_umapMotionData.find(strBoneName);
			if (iter != m_umapMotionData.end())
			{
				iter->second = keyframe;
			}
			else
			{
				m_umapMotionData.emplace(strBoneName, keyframe);
			}
		}

		const IMotion::Keyframe* MotionRecoder::GetKeyframe(const String::StringID& strBoneName) const
		{
			auto iter = m_umapMotionData.find(strBoneName);
			if (iter != m_umapMotionData.end())
				return &iter->second.keyframe;

			return nullptr;
		}

		void MotionRecoder::Clear()
		{
			for (auto& iter : m_umapMotionData)
			{
				iter.second.caching.value = eInvalidCachingIndex;
			}
		}
	}
}