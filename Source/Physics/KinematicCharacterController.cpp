#include "stdafx.h"
#include "KinematicCharacterController.h"

#include "PhysicsSystem.h"
#include "MathConvertor.h"
#include "GhostObject.h"

#include "BulletDynamics/Character/btKinematicCharacterController.h"

namespace EastEngine
{
	namespace Physics
	{
		KinematicCharacterController::KinematicCharacterController()
			: m_pDynamicsWorld(nullptr)
			, m_pController(nullptr)
			, m_pGhostObject(nullptr)
		{
		}

		KinematicCharacterController::~KinematicCharacterController()
		{
			if (m_pDynamicsWorld != nullptr)
			{
				m_pDynamicsWorld->removeAction(m_pController);
				m_pDynamicsWorld = nullptr;
			}

			SafeDelete(m_pController);
		}

		KinematicCharacterController* KinematicCharacterController::Create(GhostObject* pGhostObject, float fStepHeight)
		{
			KinematicCharacterController* pController = new KinematicCharacterController;
			if (pController->init(pGhostObject, fStepHeight) == false)
			{
				SafeDelete(pController);
				return nullptr;
			}

			return pController;
		}

		void KinematicCharacterController::SetWalkDirection(const Math::Vector3& f3WalkVelocity)
		{
			m_pController->setWalkDirection(Math::ConvertToBt(f3WalkVelocity));
		}

		void KinematicCharacterController::SetVelocityForTimeInterval(const Math::Vector3& f3Velocity, float fTimeInterval)
		{
			m_pController->setVelocityForTimeInterval(Math::ConvertToBt(f3Velocity), fTimeInterval);
		}

		void KinematicCharacterController::Reset()
		{
			m_pController->reset(m_pDynamicsWorld);
		}

		void KinematicCharacterController::Warp(const Math::Vector3& f3Pos)
		{
			m_pController->warp(Math::ConvertToBt(f3Pos));
		}

		void KinematicCharacterController::SetFallSpeed(float fFallSpeed)
		{
			m_pController->setFallSpeed(fFallSpeed);
		}

		void KinematicCharacterController::SetJumpSpeed(float fJumpSpeed)
		{
			m_pController->setJumpSpeed(fJumpSpeed);
		}

		void KinematicCharacterController::SetMaxJumpHeight(float fMaxJumpHeight)
		{
			m_pController->setMaxJumpHeight(fMaxJumpHeight);
		}

		bool KinematicCharacterController::IsEnableJump() const
		{
			return m_pController->canJump();
		}

		void KinematicCharacterController::Jump()
		{
			m_pController->jump();
		}

		void KinematicCharacterController::SetGravity(float fGravity)
		{
			m_pController->setGravity(fGravity);
		}

		float KinematicCharacterController::GetGravity() const
		{
			return m_pController->getGravity();
		}

		void KinematicCharacterController::SetMaxSlope(float fSlopeRadians)
		{
			m_pController->setMaxSlope(fSlopeRadians);
		}

		float KinematicCharacterController::GetMaxSlope()
		{
			return m_pController->getMaxSlope();
		}

		bool KinematicCharacterController::IsOnGround() const
		{
			return m_pController->onGround();
		}

		void KinematicCharacterController::SetUpInterpolate(bool isUpInterpolate)
		{
			m_pController->setUpInterpolate(isUpInterpolate);
		}

		bool KinematicCharacterController::init(GhostObject* pGhostObject, float fStepHeight)
		{
			m_pGhostObject = pGhostObject;

			m_pDynamicsWorld = PhysicsSystem::GetInstance()->GetDynamicsWorld();

			m_pController = new btKinematicCharacterController(m_pGhostObject->GetInterface(), static_cast<btConvexShape*>(m_pGhostObject->GetCollisionShape()), fStepHeight);
			m_pDynamicsWorld->addCharacter(m_pController);

			return true;
		}
	}
}