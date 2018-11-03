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
			virtual ~MotionRecorder();

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

			virtual void SetLastPlayTime(float fLastPlayTime) override { m_fLastPlayTime = fLastPlayTime; }
			virtual float GetLastPlayTime() const override { return m_fLastPlayTime; }

		public:
			void Clear(float fStartTime);

		private:
			std::unordered_map<string::StringID, math::Transform> m_umapMotionData;
			std::queue<const IMotionEvent*> m_queueEvents;
			float m_fLastPlayTime{ 0.f };
		};
	}
}