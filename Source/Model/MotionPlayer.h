#pragma once

#include "ModelInterface.h"

namespace EastEngine
{
	namespace Graphics
	{
		class MotionPlayer : public IMotionPlayer
		{
		public:
			MotionPlayer();
			virtual ~MotionPlayer();

		public:
			virtual void SetCaching(const String::StringID& strBoneName, size_t nIndex) override;
			virtual size_t GetCaching(const String::StringID& strBoneName) const override;

			virtual void SetKeyframe(const String::StringID& strBoneName, const IMotion::Keyframe& keyframe) override;
			virtual const IMotion::Keyframe* GetKeyframe(const String::StringID& strBoneName) const override;

			virtual float GetPlayTime() const override { return m_fPlayTime; }

			virtual float GetSpeed() const override { return m_playBackInfo.fSpeed; }
			virtual float GetWeight() const override { return m_playBackInfo.fWeight; }
			virtual float GetBlendTime() const override { return m_playBackInfo.fBlendTime; }
			virtual int GetMaxLoopCount() const override { return m_playBackInfo.nLoopCount; }
			virtual int GetLoopCount() const override { return m_nLoonCount; }
			virtual bool IsLoop() const override { return m_playBackInfo.nLoopCount == MotionPlaybackInfo::eMaxLoopCount; }
			virtual bool IsInverse() const override { return m_playBackInfo.isInverse; }

			virtual IMotion* GetMotion() const override { return m_pMotion; }
			virtual bool IsPlaying() const override { return m_pMotion != nullptr; }

		public:
			bool Update(float fElapsedTime);

			void Play(IMotion* pMotion, const MotionPlaybackInfo* pPlayback);
			void Stop(float fStopTime);

			void Reset();

		private:
			bool m_isStop;

			float m_fPlayTime;

			float m_fStopTime;
			float m_fStopTimeCheck;

			uint32_t m_nLoonCount;

			IMotion* m_pMotion;
			MotionPlaybackInfo m_playBackInfo;

			struct keyframeCachingT {};
			using KeyframeCaching = Type<keyframeCachingT, size_t>;

			std::unordered_map<String::StringID, KeyframeCaching> m_umapKeyframeIndexCaching;
			std::unordered_map<String::StringID, IMotion::Keyframe> m_umapKeyframe;
		};
	}
}