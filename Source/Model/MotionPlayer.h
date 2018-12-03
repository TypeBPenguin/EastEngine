#pragma once

#include "ModelInterface.h"
#include "MotionRecorder.h"

namespace eastengine
{
	namespace graphics
	{
		class MotionPlayer : public IMotionPlayer
		{
		public:
			MotionPlayer();
			MotionPlayer(const MotionPlayer& source);
			MotionPlayer(MotionPlayer&& source) noexcept;
			virtual ~MotionPlayer();

			MotionPlayer& operator = (const MotionPlayer& source);
			MotionPlayer& operator = (MotionPlayer&& source) noexcept;

		public:
			virtual float GetPlayTime() const override { return m_playTime; }
			virtual void SetPlayTime(float fPlayTime) override { m_playTime = fPlayTime; }

			virtual float GetSpeed() const override { return m_playBackInfo.speed; }
			virtual void SetSpeed(float speed) override { m_playBackInfo.speed = speed; }

			virtual float GetWeight() const override { return m_playBackInfo.weight; }
			virtual void SetWeight(float weight) override { m_playBackInfo.weight = weight; }

			virtual float GetBlendWeight() const override;

			virtual float GetBlendTime() const override { return m_playBackInfo.blendTime; }
			virtual int GetMaxLoopCount() const override { return m_playBackInfo.loopCount; }
			virtual int GetLoopCount() const override { return m_loonCount; }
			virtual bool IsLoop() const override { return m_playBackInfo.loopCount == MotionPlaybackInfo::eMaxLoopCount; }
			virtual bool IsInverse() const override { return m_playBackInfo.isInverse; }

			virtual bool IsPause() const override { return m_isPause; }
			virtual void SetPause(bool isPause) override { m_isPause = isPause; }

			virtual IMotion* GetMotion() const override { return m_pMotion; }
			virtual bool IsPlaying() const override { return m_pMotion != nullptr && m_isPause == false; }

			virtual const IMotionEvent* PopEvent() override { return m_motionRecorder.PopEvent(); }

		public:
			bool Update(float elapsedTime);

			void Play(IMotion* pMotion, const MotionPlaybackInfo* pPlayback, float blendTime);
			void Stop(float stopTime);

			const math::Transform* GetTransform(const string::StringID& boneName) const { return m_motionRecorder.GetTransform(boneName); }

		private:
			void Reset(float startTime);

		private:
			bool m_isStop{ false };
			bool m_isPause{ false };
			bool m_isRecordedLastFrame{ false };

			float m_playTime{ 0.f };

			float m_stopTime{ 0.f };
			float m_stopTimeCheck{ 0.f };

			float m_blendTime{ 0.f };
			float m_blendWeight{ 0.f };

			uint32_t m_loonCount{ 0 };

			MotionPlaybackInfo m_playBackInfo;
			MotionRecorder m_motionRecorder;

			IMotion* m_pMotion{ nullptr };
		};
	}
}