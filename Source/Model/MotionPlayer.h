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
			virtual ~MotionPlayer();

		public:
			virtual float GetPlayTime() const override { return m_fPlayTime; }
			virtual void SetPlayTime(float fPlayTime) override { m_fPlayTime = fPlayTime; }

			virtual float GetSpeed() const override { return m_playBackInfo.fSpeed; }
			virtual void SetSpeed(float fSpeed) override { m_playBackInfo.fSpeed = fSpeed; }

			virtual float GetWeight() const override { return m_playBackInfo.fWeight; }
			virtual void SetWeight(float fWeight) override { m_playBackInfo.fWeight = fWeight; }

			virtual float GetBlendWeight() const override;

			virtual float GetBlendTime() const override { return m_playBackInfo.fBlendTime; }
			virtual int GetMaxLoopCount() const override { return m_playBackInfo.nLoopCount; }
			virtual int GetLoopCount() const override { return m_nLoonCount; }
			virtual bool IsLoop() const override { return m_playBackInfo.nLoopCount == MotionPlaybackInfo::eMaxLoopCount; }
			virtual bool IsInverse() const override { return m_playBackInfo.isInverse; }

			virtual bool IsPause() const override { return m_isPause; }
			virtual void SetPause(bool isPause) override { m_isPause = isPause; }

			virtual IMotion* GetMotion() const override { return m_pMotion; }
			virtual bool IsPlaying() const override { return m_pMotion != nullptr && m_isPause == false; }

			virtual const IMotionEvent* PopEvent() override { return m_motionRecorder.PopEvent(); }

		public:
			bool Update(float fElapsedTime, bool isEnableTransformUpdate);

			void Play(IMotion* pMotion, const MotionPlaybackInfo* pPlayback);
			void Stop(float fStopTime);

			void Reset(float fStartTime);

			const math::Transform* GetTransform(const string::StringID& strBoneName) const;

		private:
			bool m_isStop;
			bool m_isPause;

			float m_fPlayTime;

			float m_fStopTime;
			float m_fStopTimeCheck;

			float m_fBlendTime;
			float m_fBlendWeight;

			uint32_t m_nLoonCount;

			IMotion* m_pMotion;

			MotionPlaybackInfo m_playBackInfo;
			MotionRecorder m_motionRecorder;
		};
	}
}