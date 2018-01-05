#pragma once

#include "MotionPlayer.h"

namespace EastEngine
{
	namespace Graphics
	{
		class MotionSystem : public IMotionSystem
		{
		public:
			MotionSystem(ISkeletonInstance* pSkeletonInstance);
			virtual ~MotionSystem();

		public:
			virtual void Update(float fElapsedTime) override;

			virtual void Play(EmMotion::Layers emLayer, IMotion* pMotion, const MotionPlaybackInfo* pMotionState = nullptr) override;
			virtual void Stop(EmMotion::Layers emLayer, float fStopTime) override;

			const IMotionPlayer* GetPlayer(EmMotion::Layers emLayer) const { return &m_motionPlayers[emLayer]; }

		private:
			void BlendingLayers(const MotionPlayer& player);
			void Binding();

		private:
			float m_fBlemdTime;

			std::array<MotionPlayer, EmMotion::eLayerCount> m_motionPlayers;
			std::unordered_map<String::StringID, IMotion::Keyframe> m_umapKeyframe;

			ISkeletonInstance* m_pSkeletonInstance;
		};
	}
}