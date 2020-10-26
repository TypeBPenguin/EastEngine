#include "stdafx.h"
#include "Constraint.h"

#include "PhysicsUtil.h"

namespace est
{
	namespace physics
	{
		Constraint::Constraint(physx::PxConstraint* pConstraint)
			: m_pConstraint(pConstraint)
		{
		}

		Constraint::~Constraint()
		{
		}

		void Constraint::GetActors(IRigidActor*& pActor0, IRigidActor*& pActor1) const
		{
			physx::PxRigidActor* pOrgActor0 = nullptr;
			physx::PxRigidActor* pOrgActor1 = nullptr;
			m_pConstraint->getActors(pOrgActor0, pOrgActor1);

			if (pOrgActor0 != nullptr)
			{
				pActor0 = static_cast<IRigidActor*>(pOrgActor0->userData);
			}

			if (pOrgActor1 != nullptr)
			{
				pActor1 = static_cast<IRigidActor*>(pOrgActor1->userData);
			}
		}

		void Constraint::SetActors(IRigidActor* pActor0, IRigidActor* pActor1)
		{
			physx::PxRigidActor* pOrgActor0 = util::GetInterface(pActor0);
			physx::PxRigidActor* pOrgActor1 = util::GetInterface(pActor1);
			m_pConstraint->setActors(pOrgActor0, pOrgActor1);
		}

		void Constraint::SetFlags(Flags flags)
		{
			m_pConstraint->setFlags(Convert<physx::PxConstraintFlags>(flags));
		}

		void Constraint::SetFlag(Flag flag, bool isEnable)
		{
			m_pConstraint->setFlag(Convert<physx::PxConstraintFlag::Enum>(flag), isEnable);
		}

		Constraint::Flags Constraint::GetFlags() const
		{
			const physx::PxConstraintFlags flags = m_pConstraint->getFlags();
			return Convert<const Constraint::Flags>(flags);
		}

		void Constraint::GetForce(math::float3& linear, math::float3& angular) const
		{
			m_pConstraint->getForce(Convert<physx::PxVec3>(linear), Convert<physx::PxVec3>(angular));
		}

		bool Constraint::IsValid() const
		{
			return m_pConstraint->isValid();
		}

		void Constraint::SetBreakForce(float linear, float angular)
		{
			m_pConstraint->setBreakForce(linear, angular);
		}

		void Constraint::GetBreakForce(float& linear, float& angular) const
		{
			m_pConstraint->getBreakForce(linear, angular);
		}

		void Constraint::SetMinResponseThreshold(float threshold)
		{
			m_pConstraint->setMinResponseThreshold(threshold);
		}

		float Constraint::GetMinResponseThreshold() const
		{
			return m_pConstraint->getMinResponseThreshold();
		}
	}
}