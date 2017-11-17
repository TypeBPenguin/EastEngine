#pragma once

class btDiscreteDynamicsWorld;
class btKinematicCharacterController;

namespace EastEngine
{
	namespace Physics
	{
		class GhostObject;

		class KinematicCharacterController
		{
		private:
			KinematicCharacterController();

		public:
			~KinematicCharacterController();

			static KinematicCharacterController* Create(GhostObject* pGhostObject, float fStepHeight);

		public:
			void SetWalkDirection(const Math::Vector3& f3WalkVelocity);
			void SetVelocityForTimeInterval(const Math::Vector3& f3Velocity, float fTimeInterval);

			void Reset();
			void Warp(const Math::Vector3& f3Pos);

			void SetFallSpeed(float fFallSpeed);
			void SetJumpSpeed(float fJumpSpeed);
			void SetMaxJumpHeight(float fMaxJumpHeight);
			bool IsEnableJump() const;

			void Jump();

			void SetGravity(float fGravity);
			float GetGravity() const;

			void SetMaxSlope(float fSlopeRadians);
			float GetMaxSlope();

			bool IsOnGround() const;
			void SetUpInterpolate(bool isUpInterpolate);

		private:
			bool init(GhostObject* pGhostObject, float fStepHeight);

		private:
			btDiscreteDynamicsWorld* m_pDynamicsWorld;
			btKinematicCharacterController* m_pController;
			GhostObject* m_pGhostObject;
		};
	}
}