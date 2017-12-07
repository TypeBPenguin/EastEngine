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

			virtual void SetCaching(const String::StringID& strBoneName, int nIndex) override;
			virtual int GetCaching(const String::StringID& strBoneName) override;

			virtual void SetKeyframe(const String::StringID& strBoneName, const IMotion::Keyframe& keyframe);
			virtual const IMotion::Keyframe* GetKeyframe(const String::StringID& strBoneName);

			virtual void Reset() override;

			virtual float GetPlayTime() const override { return m_fPlayTime; }

			virtual float GetPlaySpeed() const override { return m_motionState.fSpeed; }
			virtual float GetBlendWeight() const override { return m_motionState.fBlendWeight; }
			virtual float GetBlendTime() const override { return m_motionState.fBlendTime; }
			virtual bool IsInverse() const override { return m_motionState.isInverse; }
			virtual bool IsLoop() const override { return m_motionState.isLoop; }

		public:
			void SetMotionState(const MotionState& motionState) { m_motionState = motionState; }
			void SetPlayTime(float fPlayTime) { m_fPlayTime = fPlayTime; }

		private:
			float m_fPlayTime;

			MotionState m_motionState;

			struct KeyframeCaching
			{
				int index = -1;

				KeyframeCaching(int nIndex);

				int operator = (int source) { index = source; return index; }
				operator int() const { return index; }
			};
			std::unordered_map<String::StringID, KeyframeCaching> m_umapKeyframeIndexCaching;
			std::unordered_map<String::StringID, IMotion::Keyframe> m_umapKeyframe;
		};

		class MotionSystem : public IMotionSystem
		{
		public:
			MotionSystem(IModel* pModel);
			virtual ~MotionSystem();

		public:
			virtual void Update(float fElapsedTime, ISkeletonInstance* pSkeletonInst) override;

			virtual void Play(IMotion* pMotion, const MotionState* pMotionState = nullptr) override;
			virtual void Stop(float fStopTime) override;

			virtual IMotion* GetMotion() override { return m_pMotion; }

			virtual bool IsPlayingMotion() override { return m_pMotion != nullptr; }

			virtual float GetPlayTime() const override { return m_fPlayTime; }
			virtual float GetPlayTimeSub() const override { return m_fPlayTimeSub; }

		private:
			float m_fPlayTime;
			float m_fPlayTimeSub;
			float m_fBlemdTime;

			float m_fStopTime;
			float m_fStopTimeCheck;
			bool m_isStop;

			IMotion* m_pMotion;
			IMotion* m_pMotionSub;

			MotionPlayer m_motionPlayer;
			MotionPlayer m_motionPlayerSub;

			IModel* m_pModel;
		};
	}
}