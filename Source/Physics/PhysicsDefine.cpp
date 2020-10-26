#include "stdafx.h"
#include "PhysicsDefine.h"

namespace est
{
	namespace physics
	{
		static_assert(ForceMode::eForce == physx::PxForceMode::eFORCE, "mismatch enum");
		static_assert(ForceMode::eImpulse == physx::PxForceMode::eIMPULSE, "mismatch enum");
		static_assert(ForceMode::eVelocityChange == physx::PxForceMode::eVELOCITY_CHANGE, "mismatch enum");
		static_assert(ForceMode::eAcceleration == physx::PxForceMode::eACCELERATION, "mismatch enum");
	}
}