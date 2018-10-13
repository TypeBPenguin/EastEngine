#include "stdafx.h"
#include "KinematicCharacterController.h"

#include "PhysicsSystem.h"
#include "MathConvertor.h"
#include "GhostObject.h"

#include "BulletDynamics/Character/btKinematicCharacterController.h"

namespace eastengine
{
	namespace physics
	{
		class KinematicCharacterController::Impl
		{
		public:
			Impl();
			~Impl();

		public:
			bool Initialize(GhostObject* pGhostObject, float fStepHeight);

		public:
			void SetWalkDirection(const math::Vector3& f3WalkVelocity);
			void SetVelocityForTimeInterval(const math::Vector3& f3Velocity, float fTimeInterval);

			void Reset();
			void Warp(const math::Vector3& f3Pos);

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
			btDiscreteDynamicsWorld* m_pDynamicsWorld{ nullptr };
			std::unique_ptr<btKinematicCharacterController> m_pController;
			GhostObject* m_pGhostObject{ nullptr };
		};

		KinematicCharacterController::Impl::Impl()
		{
		}

		KinematicCharacterController::Impl::~Impl()
		{
			if (m_pDynamicsWorld != nullptr)
			{
				m_pDynamicsWorld->removeAction(m_pController.get());
				m_pDynamicsWorld = nullptr;
			}
		}

		bool KinematicCharacterController::Impl::Initialize(GhostObject* pGhostObject, float fStepHeight)
		{
			m_pGhostObject = pGhostObject;

			m_pDynamicsWorld = System::GetInstance()->GetDynamicsWorld();

			m_pController = std::make_unique<btKinematicCharacterController>(m_pGhostObject->GetInterface(), static_cast<btConvexShape*>(m_pGhostObject->GetCollisionShape()), fStepHeight);
			m_pDynamicsWorld->addCharacter(m_pController.get());

			return true;
		}

		void KinematicCharacterController::Impl::SetWalkDirection(const math::Vector3& f3WalkVelocity)
		{
			m_pController->setWalkDirection(math::ConvertToBt(f3WalkVelocity));
		}

		void KinematicCharacterController::Impl::SetVelocityForTimeInterval(const math::Vector3& f3Velocity, float fTimeInterval)
		{
			m_pController->setVelocityForTimeInterval(math::ConvertToBt(f3Velocity), fTimeInterval);
		}

		void KinematicCharacterController::Impl::Reset()
		{
			m_pController->reset(m_pDynamicsWorld);
		}

		void KinematicCharacterController::Impl::Warp(const math::Vector3& f3Pos)
		{
			m_pController->warp(math::ConvertToBt(f3Pos));
		}

		void KinematicCharacterController::Impl::SetFallSpeed(float fFallSpeed)
		{
			m_pController->setFallSpeed(fFallSpeed);
		}

		void KinematicCharacterController::Impl::SetJumpSpeed(float fJumpSpeed)
		{
			m_pController->setJumpSpeed(fJumpSpeed);
		}

		void KinematicCharacterController::Impl::SetMaxJumpHeight(float fMaxJumpHeight)
		{
			m_pController->setMaxJumpHeight(fMaxJumpHeight);
		}

		bool KinematicCharacterController::Impl::IsEnableJump() const
		{
			return m_pController->canJump();
		}

		void KinematicCharacterController::Impl::Jump()
		{
			m_pController->jump();
		}

		void KinematicCharacterController::Impl::SetGravity(float fGravity)
		{
			m_pController->setGravity(fGravity);
		}

		float KinematicCharacterController::Impl::GetGravity() const
		{
			return m_pController->getGravity();
		}

		void KinematicCharacterController::Impl::SetMaxSlope(float fSlopeRadians)
		{
			m_pController->setMaxSlope(fSlopeRadians);
		}

		float KinematicCharacterController::Impl::GetMaxSlope()
		{
			return m_pController->getMaxSlope();
		}

		bool KinematicCharacterController::Impl::IsOnGround() const
		{
			return m_pController->onGround();
		}

		void KinematicCharacterController::Impl::SetUpInterpolate(bool isUpInterpolate)
		{
			m_pController->setUpInterpolate(isUpInterpolate);
		}

		KinematicCharacterController::KinematicCharacterController()
			: m_pImpl{ std::make_unique<Impl>() }
		{
		}

		KinematicCharacterController::~KinematicCharacterController()
		{
		}

		KinematicCharacterController* KinematicCharacterController::Create(GhostObject* pGhostObject, float fStepHeight)
		{
			KinematicCharacterController* pController = new KinematicCharacterController;
			if (pController->Initialize(pGhostObject, fStepHeight) == false)
			{
				SafeDelete(pController);
				return nullptr;
			}

			return pController;
		}

		void KinematicCharacterController::SetWalkDirection(const math::Vector3& f3WalkVelocity)
		{
			m_pImpl->SetWalkDirection(f3WalkVelocity);
		}

		void KinematicCharacterController::SetVelocityForTimeInterval(const math::Vector3& f3Velocity, float fTimeInterval)
		{
			m_pImpl->SetVelocityForTimeInterval(f3Velocity, fTimeInterval);
		}

		void KinematicCharacterController::Reset()
		{
			m_pImpl->Reset();
		}

		void KinematicCharacterController::Warp(const math::Vector3& f3Pos)
		{
			m_pImpl->Warp(f3Pos);
		}

		void KinematicCharacterController::SetFallSpeed(float fFallSpeed)
		{
			m_pImpl->SetFallSpeed(fFallSpeed);
		}

		void KinematicCharacterController::SetJumpSpeed(float fJumpSpeed)
		{
			m_pImpl->SetJumpSpeed(fJumpSpeed);
		}

		void KinematicCharacterController::SetMaxJumpHeight(float fMaxJumpHeight)
		{
			m_pImpl->SetMaxJumpHeight(fMaxJumpHeight);
		}

		bool KinematicCharacterController::IsEnableJump() const
		{
			return m_pImpl->IsEnableJump();
		}

		void KinematicCharacterController::Jump()
		{
			m_pImpl->Jump();
		}

		void KinematicCharacterController::SetGravity(float fGravity)
		{
			m_pImpl->SetGravity(fGravity);
		}

		float KinematicCharacterController::GetGravity() const
		{
			return m_pImpl->GetGravity();
		}

		void KinematicCharacterController::SetMaxSlope(float fSlopeRadians)
		{
			m_pImpl->SetMaxSlope(fSlopeRadians);
		}

		float KinematicCharacterController::GetMaxSlope()
		{
			return m_pImpl->GetMaxSlope();
		}

		bool KinematicCharacterController::IsOnGround() const
		{
			return m_pImpl->IsOnGround();
		}

		void KinematicCharacterController::SetUpInterpolate(bool isUpInterpolate)
		{
			m_pImpl->SetUpInterpolate(isUpInterpolate);
		}

		bool KinematicCharacterController::Initialize(GhostObject* pGhostObject, float fStepHeight)
		{
			return m_pImpl->Initialize(pGhostObject, fStepHeight);
		}
	}
}