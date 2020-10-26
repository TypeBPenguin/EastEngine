#pragma once

#include "PhysicsInterface.h"

namespace est
{
	namespace physics
	{
		class Constraint : public IConstraint
		{
		public:
			Constraint(physx::PxConstraint* pConstraint);
			virtual ~Constraint();

		public:
			virtual void GetActors(IRigidActor*& pActor0, IRigidActor*& pActor1) const override;
			virtual void SetActors(IRigidActor* pActor0, IRigidActor* pActor1) override;

			virtual void SetFlags(Flags flags) override;
			virtual void SetFlag(Flag flag, bool isEnable) override;
			virtual Flags GetFlags() const override;

			virtual void GetForce(math::float3& linear, math::float3& angular) const override;

			virtual bool IsValid() const override;

			virtual void SetBreakForce(float linear, float angular) override;
			virtual void GetBreakForce(float& linear, float& angular) const override;

			virtual void SetMinResponseThreshold(float threshold) override;
			virtual float GetMinResponseThreshold() const override;

		private:
			physx::PxConstraint* m_pConstraint{ nullptr };
		};
	}
}