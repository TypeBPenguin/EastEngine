#pragma once

#include "PhysicsInterface.h"

namespace est
{
	namespace physics
	{
#define DeclJointFunction																		\
		virtual void SetActors(IRigidActor* pActor0, IRigidActor* pActor1) override;			\
		virtual void GetActors(IRigidActor*& pActor0, IRigidActor*& pActor1) const override;	\
																								\
		virtual void SetLocalPose(ActorIndex emActor, const Transform& localPose) override;		\
		virtual Transform GetLocalPose(ActorIndex emActor) const override;						\
																								\
		virtual Transform GetRelativeTransform() const override;								\
		virtual math::float3 GetRelativeLinearVelocity() const override;						\
																								\
		virtual math::float3 GetRelativeAngularVelocity() const override;						\
		virtual void SetBreakForce(float force, float torque) override;							\
																								\
		virtual void GetBreakForce(float& force, float& torque) const override;					\
		virtual void SetConstraintFlags(IConstraint::Flags flags) override;						\
																								\
		virtual void SetConstraintFlag(IConstraint::Flag flag, bool isEnable) override;			\
		virtual IConstraint::Flags GetConstraintFlags() const override;							\
																								\
		virtual void SetInvMassScale0(float invMassScale) override;								\
		virtual float GetInvMassScale0() const override;										\
																								\
		virtual void SetInvInertiaScale0(float invInertiaScale) override;						\
		virtual float GetInvInertiaScale0() const override;										\
																								\
		virtual void SetInvMassScale1(float invMassScale) override;								\
		virtual float GetInvMassScale1() const override;										\
																								\
		virtual void SetInvInertiaScale1(float invInertiaScale) override;						\
		virtual float GetInvInertiaScale1() const override;										\
																								\
		virtual IConstraint* GetConstraint() override;											\
																								\
		virtual void SetName(const string::StringID& name) override;							\
		virtual const string::StringID& GetName() const override;

		class ContactJoint : public IContactJoint
		{
		public:
			ContactJoint(physx::PxContactJoint* m_pContactJoint);
			virtual ~ContactJoint();

		public:
			DeclJointFunction;

		public:
			virtual void SetContact(const math::float3& contact) override;
			virtual void SetContactNormal(const math::float3& contactNormal) override;
			virtual void SetPenetration(const float penetration) override;

			virtual math::float3 GetContact() const override;
			virtual math::float3 GetContactNormal() const override;
			virtual float GetPenetration() const override;

			virtual float GetResititution() const override;
			virtual void SetResititution(float resititution) override;
			virtual float GetBounceThreshold() const override;
			virtual void SetBounceThreshold(float bounceThreshold) override;

		private:
			physx::PxContactJoint* m_pContactJoint{ nullptr };
			std::unique_ptr<IConstraint> m_pConstraint;
			string::StringID m_name;
		};

		class D6Joint : public ID6Joint
		{
		public:
			D6Joint(physx::PxD6Joint* pD6Joint);
			virtual ~D6Joint();

		public:
			DeclJointFunction;

		public:
			virtual void SetMotion(Axis emAxis, Motion emMotionType) override;
			virtual Motion GetMotion(Axis emAxis) const override;

			virtual float GetTwistAngle() const override;

			virtual float GetSwingYAngle() const override;
			virtual float GetSwingZAngle() const override;

			virtual void SetDistanceLimit(const JointLinearLimit& limit) override;
			virtual JointLinearLimit GetDistanceLimit() const override;

			virtual void SetLinearLimit(const JointLinearLimit& limit) override;
			virtual JointLinearLimit GetLinearLimit() const override;

			virtual void SetTwistLimit(const JointAngularLimitPair& limit) override;
			virtual JointAngularLimitPair GetTwistLimit() const override;

			virtual void SetSwingLimit(const JointLimitCone& limit) override;
			virtual JointLimitCone GetSwingLimit() const override;

			virtual void SetPyramidSwingLimit(const JointLimitPyramid& limit) override;
			virtual JointLimitPyramid GetPyramidSwingLimit() const override;

			virtual void SetDrive(Drive emDriveIndex, const JointDrive& drive) override;
			virtual JointDrive GetDrive(Drive emDriveIndex) const override;

			virtual void SetDrivePosition(const Transform& pose, bool isEnableAutoWake = true) override;
			virtual Transform GetDrivePosition() const override;

			virtual void SetDriveVelocity(const math::float3& linear, const math::float3& angular, bool isEnableAutoWake = true) override;
			virtual void GetDriveVelocity(math::float3& linear, math::float3& angular) const override;

			virtual void SetProjectionLinearTolerance(float tolerance) override;
			virtual float GetProjectionLinearTolerance() const override;

			virtual void SetProjectionAngularTolerance(float tolerance) override;
			virtual float GetProjectionAngularTolerance() const override;

		private:
			physx::PxD6Joint* m_pD6Joint{ nullptr };
			std::unique_ptr<IConstraint> m_pConstraint;
			string::StringID m_name;
		};

		class DistanceJoint : public IDistanceJoint
		{
		public:
			DistanceJoint(physx::PxDistanceJoint* pDistanceJoint);
			virtual ~DistanceJoint();

		public:
			DeclJointFunction;

		public:
			virtual float GetDistance() const override;

			virtual void SetMinDistance(float distance) override;
			virtual float GetMinDistance() const override;

			virtual void SetMaxDistance(float distance) override;
			virtual float GetMaxDistance() const override;

			virtual void SetTolerance(float tolerance) override;
			virtual float GetTolerance() const override;

			virtual void SetStiffness(float stiffness) override;
			virtual float GetStiffness() const override;

			virtual void SetDamping(float damping) override;
			virtual float GetDamping() const override;

			virtual void SetDistanceJointFlags(Flags flags) override;
			virtual void SetDistanceJointFlags(Flag flag, bool isEnable) override;
			virtual Flags GetDistanceJointFlags() const override;

		private:
			physx::PxDistanceJoint* m_pDistanceJoint{ nullptr };
			std::unique_ptr<IConstraint> m_pConstraint;
			string::StringID m_name;
		};

		class FixedJoint : public IFixedJoint
		{
		public:
			FixedJoint(physx::PxFixedJoint* pFixedJoint);
			virtual ~FixedJoint();

		public:
			DeclJointFunction;

		public:
			virtual void SetProjectionLinearTolerance(float tolerance) override;
			virtual float GetProjectionLinearTolerance() const override;

			virtual void SetProjectionAngularTolerance(float tolerance) override;
			virtual float GetProjectionAngularTolerance() const override;

		private:
			physx::PxFixedJoint* m_pFixedJoint{ nullptr };
			std::unique_ptr<IConstraint> m_pConstraint;
			string::StringID m_name;
		};

		class PrismaticJoint : public IPrismaticJoint
		{
		public:
			PrismaticJoint(physx::PxPrismaticJoint* pPrismaticJoint);
			virtual ~PrismaticJoint();

		public:
			DeclJointFunction;

		public:
			virtual float GetPosition() const override;
			virtual float GetVelocity() const override;

			virtual void SetLimit(const JointLinearLimitPair& limit) override;
			virtual JointLinearLimitPair GetLimit() const override;

			virtual void SetPrismaticJointFlags(Flags flags) override;
			virtual void SetPrismaticJointFlag(Flag flag, bool isEnable) override;
			virtual Flags GetPrismaticJointFlags() const override;

			virtual void SetProjectionLinearTolerance(float tolerance) override;
			virtual float GetProjectionLinearTolerance() const override;

			virtual void SetProjectionAngularTolerance(float tolerance) override;
			virtual float GetProjectionAngularTolerance() const override;

		private:
			physx::PxPrismaticJoint* m_pPrismaticJoint{ nullptr };
			std::unique_ptr<IConstraint> m_pConstraint;
			string::StringID m_name;
		};

		class RevoluteJoint : public IRevoluteJoint
		{
		public:
			RevoluteJoint(physx::PxRevoluteJoint* pRevoluteJoint);
			virtual ~RevoluteJoint();

		public:
			DeclJointFunction;

		public:
			virtual float GetAngle() const override;
			virtual float GetVelocity() const override;

			virtual void SetLimit(const JointAngularLimitPair& limit) override;
			virtual JointAngularLimitPair GetLimit() const override;

			virtual void SetDriveVelocity(float velocity, bool isEnableAutoWake = true) override;
			virtual float GetDriveVelocity() const override;

			virtual void SetDriveForceLimit(float limit) override;
			virtual float GetDriveForceLimit() const override;

			virtual void SetDriveGearRatio(float ratio) override;
			virtual float GetDriveGearRatio() const override;

			virtual void SetRevoluteJointFlags(Flags flags) override;
			virtual void SetRevoluteJointFlag(Flag flag, bool isEnable) override;
			virtual Flags GetRevoluteJointFlags() const override;

			virtual void SetProjectionLinearTolerance(float tolerance) override;
			virtual float GetProjectionLinearTolerance() const override;

			virtual void SetProjectionAngularTolerance(float tolerance) override;
			virtual float GetProjectionAngularTolerance() const override;

		private:
			physx::PxRevoluteJoint* m_pRevoluteJoint{ nullptr };
			std::unique_ptr<IConstraint> m_pConstraint;
			string::StringID m_name;
		};

		class SphericalJoint : public ISphericalJoint
		{
		public:
			SphericalJoint(physx::PxSphericalJoint* pSphericalJoint);
			virtual ~SphericalJoint();

		public:
			DeclJointFunction;

		public:
			virtual JointLimitCone GetLimitCone() const override;
			virtual void SetLimitCone(const JointLimitCone& limit) override;

			virtual float GetSwingYAngle() const override;
			virtual float GetSwingZAngle() const override;

			virtual void SetSphericalJointFlags(Flags flags) override;
			virtual void SetSphericalJointFlag(Flag flag, bool isEnable) override;
			virtual Flags GetSphericalJointFlags() const override;

			virtual void SetProjectionLinearTolerance(float tolerance) override;
			virtual float GetProjectionLinearTolerance() const override;

		private:
			physx::PxSphericalJoint* m_pSphericalJoint{ nullptr };
			std::unique_ptr<IConstraint> m_pConstraint;
			string::StringID m_name;
		};
	}
}