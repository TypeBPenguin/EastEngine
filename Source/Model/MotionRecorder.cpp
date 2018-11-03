#include "stdafx.h"
#include "MotionRecorder.h"

namespace eastengine
{
	namespace graphics
	{
		MotionRecorder::MotionRecorder()
		{
		}

		MotionRecorder::~MotionRecorder()
		{
		}
		
		void MotionRecorder::SetTransform(const string::StringID& strBoneName, const math::Transform& transform)
		{
			m_umapMotionData[strBoneName] = transform;
		}

		const math::Transform* MotionRecorder::GetTransform(const string::StringID& strBoneName) const
		{
			auto iter = m_umapMotionData.find(strBoneName);
			if (iter != m_umapMotionData.end())
				return &iter->second;

			return nullptr;
		}

		void MotionRecorder::Clear(float fStartTime)
		{
			for (auto& iter : m_umapMotionData)
			{
				iter.second = math::Transform{};
			}

			m_fLastPlayTime = fStartTime;

			while (m_queueEvents.empty() == false)
			{
				m_queueEvents.pop();
			}
		}
	}
}