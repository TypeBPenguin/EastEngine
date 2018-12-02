#pragma once

#include "ModelInterface.h"

namespace eastengine
{
	namespace graphics
	{
		class MotionRecorder : public IMotionRecorder
		{
		public:
			MotionRecorder();
			MotionRecorder(const MotionRecorder& source);
			MotionRecorder(MotionRecorder&& source) noexcept;
			virtual ~MotionRecorder();

			MotionRecorder& operator = (const MotionRecorder& source);
			MotionRecorder& operator = (MotionRecorder&& source) noexcept;

		public:
			virtual void SetTransform(const string::StringID& strBoneName, const math::Transform& keyframe) override;
			virtual const math::Transform* GetTransform(const string::StringID& strBoneName) const override;

			virtual void PushEvent(const IMotionEvent* pMotionEvent) override { m_queueEvents.push(pMotionEvent); }
			virtual const IMotionEvent* PopEvent() override
			{
				if (m_queueEvents.empty() == true)
					return nullptr;

				const IMotionEvent* pEvent = m_queueEvents.front();
				m_queueEvents.pop();
				return pEvent;
			}

			virtual void SetLastPlayTime(float fLastPlayTime) override { m_lastPlayTime = fLastPlayTime; }
			virtual float GetLastPlayTime() const override { return m_lastPlayTime; }

		public:
			void Clear(float fStartTime);

		private:
			tsl::robin_map<string::StringID, math::Transform> m_rmapMotionData;
			std::queue<const IMotionEvent*> m_queueEvents;
			float m_lastPlayTime{ 0.f };
		};
	}
}