#include "stdafx.h"
#include "MotionRecorder.h"

namespace eastengine
{
	namespace graphics
	{
		MotionRecorder::MotionRecorder()
		{
		}

		MotionRecorder::MotionRecorder(const MotionRecorder& source)
		{
			*this = source;
		}

		MotionRecorder::MotionRecorder(MotionRecorder&& source) noexcept
		{
			*this = std::move(source);
		}

		MotionRecorder::~MotionRecorder()
		{
		}

		MotionRecorder& MotionRecorder::operator = (const MotionRecorder& source)
		{
			m_rmapMotionData = source.m_rmapMotionData;
			m_queueEvents = source.m_queueEvents;
			m_lastPlayTime = source.m_lastPlayTime;

			return *this;
		}

		MotionRecorder& MotionRecorder::operator = (MotionRecorder&& source) noexcept
		{
			m_rmapMotionData = std::move(source.m_rmapMotionData);
			m_queueEvents = std::move(source.m_queueEvents);
			m_lastPlayTime = std::move(source.m_lastPlayTime);

			return *this;
		}

		void MotionRecorder::SetTransform(const string::StringID& strBoneName, const math::Transform& transform)
		{
			m_rmapMotionData[strBoneName] = transform;
		}

		const math::Transform* MotionRecorder::GetTransform(const string::StringID& strBoneName) const
		{
			auto iter = m_rmapMotionData.find(strBoneName);
			if (iter != m_rmapMotionData.end())
				return &iter->second;

			return nullptr;
		}

		void MotionRecorder::Clear(float fStartTime)
		{
			m_rmapMotionData.clear();
			m_queueEvents = {};
			m_lastPlayTime = fStartTime;
		}
	}
}