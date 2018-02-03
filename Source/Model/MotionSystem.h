#pragma once

#include "MotionPlayer.h"

namespace EastEngine
{
	namespace Graphics
	{
		class MotionSystem : public IMotionSystem
		{
		public:
			MotionSystem();
			virtual ~MotionSystem();

		public:
			virtual void Play(EmMotion::Layers emLayer, IMotion* pMotion, const MotionPlaybackInfo* pMotionState = nullptr) override;
			virtual void Stop(EmMotion::Layers emLayer, float fStopTime) override;

			virtual IMotionPlayer* GetPlayer(EmMotion::Layers emLayer) override { return &m_motionPlayers[emLayer]; }

		public:
			void Initialize(ISkeletonInstance* pSkeletonInstance);
			void Update(float fElapsedTime);

		private:
			void SetIdentity(bool isMotionUpdated);
			void BlendingLayers(const MotionPlayer& player);
			void Binding();

		private:
			float m_fBlemdTime;

			std::array<MotionPlayer, EmMotion::eLayerCount> m_motionPlayers;

			struct MotionTransform
			{
				Math::Transform motionTransform;
				Math::Transform defaultTransform;
			};
			std::vector<MotionTransform> m_vecMotionTransforms;

			ISkeletonInstance* m_pSkeletonInstance;
		};
	}
}