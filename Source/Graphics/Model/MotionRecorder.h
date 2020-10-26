#pragma once

#include "ModelInterface.h"

namespace est
{
	namespace graphics
	{
		class MotionRecorder
		{
		public:
			MotionRecorder();
			MotionRecorder(const MotionRecorder& source);
			MotionRecorder(MotionRecorder&& source) noexcept;
			~MotionRecorder();

			MotionRecorder& operator = (const MotionRecorder& source);
			MotionRecorder& operator = (MotionRecorder&& source) noexcept;

		public:
			void SetTransform(const string::StringID& boneName, const math::Transform& keyframe);
			const math::Transform* GetTransform(const string::StringID& boneName) const;

			void PushEvent(const IMotionEvent* pMotionEvent) { m_queueEvents.push(pMotionEvent); }
			const IMotionEvent* PopEvent()
			{
				if (m_queueEvents.empty() == true)
					return nullptr;

				const IMotionEvent* pEvent = m_queueEvents.front();
				m_queueEvents.pop();
				return pEvent;
			}

			void SetLastPlayTime(float lastPlayTime) { m_lastPlayTime = lastPlayTime; }
			float GetLastPlayTime() const { return m_lastPlayTime; }

		public:
			void Clear(float fStartTime);

		private:
			tsl::robin_map<string::StringID, math::Transform> m_rmapMotionData;
			std::queue<const IMotionEvent*> m_queueEvents;
			float m_lastPlayTime{ 0.f };
		};
	}
}