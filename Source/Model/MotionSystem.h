#pragma once

#include "MotionPlayer.h"

namespace eastengine
{
	namespace graphics
	{
		class MotionSystem : public IMotionSystem
		{
		public:
			MotionSystem();
			MotionSystem(const MotionSystem& source);
			MotionSystem(MotionSystem&& source) noexcept;
			virtual ~MotionSystem();

			MotionSystem& operator = (const MotionSystem& source);
			MotionSystem& operator = (MotionSystem&& source) noexcept;

		public:
			virtual void Play(MotionLayers emLayer, IMotion* pMotion, const MotionPlaybackInfo* pMotionState = nullptr) override;
			virtual void Stop(MotionLayers emLayer, float stopTime) override;

			virtual IMotionPlayer* GetPlayer(MotionLayers emLayer) override { return &m_motionPlayers[emLayer]; }

		public:
			void Initialize(ISkeletonInstance* pSkeletonInstance);
			void Update(float elapsedTime);

		private:
			void SetIdentity(bool isMotionUpdated);
			void BlendingLayers(const MotionPlayer& player, const MotionPlayer& blendPlayer);
			void Binding();

		private:
			ISkeletonInstance* m_pSkeletonInstance{ nullptr };

			std::array<MotionPlayer, MotionLayers::eLayerCount> m_motionPlayers;
			std::array<MotionPlayer, MotionLayers::eLayerCount> m_blendMotionPlayers;

			struct MotionTransform
			{
				math::Transform motionTransform;
				math::Transform defaultTransform;
			};
			std::vector<MotionTransform> m_vecMotionTransforms;
		};
	}
}