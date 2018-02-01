#include "stdafx.h"
#include "MotionRecorder.h"

namespace EastEngine
{
	namespace Graphics
	{
		MotionRecorder::MotionRecorder()
		{
		}

		MotionRecorder::~MotionRecorder()
		{
		}
		
		void MotionRecorder::SetTransform(const String::StringID& strBoneName, const Math::Transform& transform)
		{
			m_umapMotionData[strBoneName] = transform;
		}

		const Math::Transform* MotionRecorder::GetTransform(const String::StringID& strBoneName) const
		{
			auto iter = m_umapMotionData.find(strBoneName);
			if (iter != m_umapMotionData.end())
				return &iter->second;

			return nullptr;
		}

		void MotionRecorder::Clear()
		{
			for (auto& iter : m_umapMotionData)
			{
				iter.second = Math::Transform{};
			}
		}
	}
}