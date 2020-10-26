#include "stdafx.h"
#include "Joint.h"

#include "PhysicsUtil.h"
#include "RigidBody.h"
#include "Constraint.h"

namespace est
{
	namespace physics
	{
#define ImplJointFunction(ClassType, Interface)																								\
		void ClassType::SetActors(IRigidActor* pActor0, IRigidActor* pActor1)																\
		{																																	\
			physx::PxRigidActor* pOrgActor0 = util::GetInterface(pActor0);																	\
			physx::PxRigidActor* pOrgActor1 = util::GetInterface(pActor1);																	\
			Interface->setActors(pOrgActor0, pOrgActor1);																					\
		}																																	\
																																			\
		void ClassType::GetActors(IRigidActor*& pActor0, IRigidActor*& pActor1) const														\
		{																																	\
			physx::PxRigidActor* pOrgActor0 = nullptr;																						\
			physx::PxRigidActor* pOrgActor1 = nullptr;																						\
			Interface->getActors(pOrgActor0, pOrgActor1);																					\
																																			\
			if (pOrgActor0 != nullptr)																										\
			{																																\
				pActor0 = static_cast<IRigidActor*>(pOrgActor0->userData);																	\
			}																																\
																																			\
			if (pOrgActor1 != nullptr)																										\
			{																																\
				pActor1 = static_cast<IRigidActor*>(pOrgActor1->userData);																	\
			}																																\
		}																																	\
																																			\
		void ClassType::SetLocalPose(ActorIndex emActor, const Transform& localPose)														\
		{																																	\
			Interface->setLocalPose(Convert<physx::PxJointActorIndex::Enum>(emActor), Convert<const physx::PxTransform>(localPose));		\
		}																																	\
																																			\
		Transform ClassType::GetLocalPose(ActorIndex emActor) const																			\
		{																																	\
			const physx::PxTransform transform = Interface->getLocalPose(Convert<physx::PxJointActorIndex::Enum>(emActor));					\
			return Convert<const Transform>(transform);																						\
		}																																	\
																																			\
		Transform ClassType::GetRelativeTransform() const																					\
		{																																	\
			const physx::PxTransform transform = Interface->getRelativeTransform();															\
			return Convert<const Transform>(transform);																						\
		}																																	\
																																			\
		math::float3 ClassType::GetRelativeLinearVelocity() const																			\
		{																																	\
			const physx::PxVec3 velocity = Interface->getRelativeLinearVelocity();															\
			return Convert<const math::float3>(velocity);																					\
		}																																	\
																																			\
		math::float3 ClassType::GetRelativeAngularVelocity() const																			\
		{																																	\
			const physx::PxVec3 velocity = Interface->getRelativeAngularVelocity();															\
			return Convert<const math::float3>(velocity);																					\
		}																																	\
																																			\
		void ClassType::SetBreakForce(float force, float torque)																			\
		{																																	\
			Interface->setBreakForce(force, torque);																						\
		}																																	\
																																			\
		void ClassType::GetBreakForce(float& force, float& torque) const																	\
		{																																	\
			Interface->getBreakForce(force, torque);																						\
		}																																	\
																																			\
		void ClassType::SetConstraintFlags(IConstraint::Flags flags)																		\
		{																																	\
			Interface->setConstraintFlags(Convert<physx::PxConstraintFlags>(flags));														\
		}																																	\
																																			\
		void ClassType::SetConstraintFlag(IConstraint::Flag flag, bool isEnable)															\
		{																																	\
			Interface->setConstraintFlag(Convert<physx::PxConstraintFlag::Enum>(flag), isEnable);											\
		}																																	\
																																			\
		IConstraint::Flags ClassType::GetConstraintFlags() const																			\
		{																																	\
			const physx::PxConstraintFlags flags = Interface->getConstraintFlags();															\
			return Convert<const IConstraint::Flags>(flags);																				\
		}																																	\
																																			\
		void ClassType::SetInvMassScale0(float invMassScale)																				\
		{																																	\
			Interface->setInvMassScale0(invMassScale);																						\
		}																																	\
																																			\
		float ClassType::GetInvMassScale0() const																							\
		{																																	\
			return Interface->getInvMassScale0();																							\
		}																																	\
																																			\
		void ClassType::SetInvInertiaScale0(float invInertiaScale)																			\
		{																																	\
			Interface->setInvInertiaScale0(invInertiaScale);																				\
		}																																	\
																																			\
		float ClassType::GetInvInertiaScale0() const																						\
		{																																	\
			return Interface->getInvInertiaScale0();																						\
		}																																	\
																																			\
		void ClassType::SetInvMassScale1(float invMassScale)																				\
		{																																	\
			Interface->setInvMassScale1(invMassScale);																						\
		}																																	\
																																			\
		float ClassType::GetInvMassScale1() const																							\
		{																																	\
			return Interface->getInvMassScale1();																							\
		}																																	\
																																			\
		void ClassType::SetInvInertiaScale1(float invInertiaScale)																			\
		{																																	\
			Interface->setInvInertiaScale1(invInertiaScale);																				\
		}																																	\
																																			\
		float ClassType::GetInvInertiaScale1() const																						\
		{																																	\
			return Interface->getInvInertiaScale1();																						\
		}																																	\
																																			\
		IConstraint* ClassType::GetConstraint()																								\
		{																																	\
			if (m_pConstraint == nullptr)																									\
			{																																\
				m_pConstraint = std::make_unique<Constraint>(Interface->getConstraint());													\
			}																																\
			return m_pConstraint.get();																										\
		}																																	\
																																			\
		void ClassType::SetName(const string::StringID& name)																				\
		{																																	\
			m_name = name;																													\
			const std::string multiName = string::WideToMulti(m_name.c_str());																\
			Interface->setName(multiName.c_str());																							\
		}																																	\
																																			\
		const string::StringID& ClassType::GetName() const																					\
		{																																	\
			return m_name;																													\
		}

		ContactJoint::ContactJoint(physx::PxContactJoint* pContactJoint)
			: m_pContactJoint(pContactJoint)
		{
			m_pContactJoint->userData = this;
		}

		ContactJoint::~ContactJoint()
		{
			m_pContactJoint->userData = nullptr;
			m_pContactJoint->release();
			m_pContactJoint = nullptr;
		}
		ImplJointFunction(ContactJoint, m_pContactJoint);

		void ContactJoint::SetContact(const math::float3& contact)
		{
			m_pContactJoint->setContact(Convert<const physx::PxVec3>(contact));
		}

		void ContactJoint::SetContactNormal(const math::float3& contactNormal)
		{
			m_pContactJoint->setContactNormal(Convert<const physx::PxVec3>(contactNormal));
		}

		void ContactJoint::SetPenetration(const float penetration)
		{
			m_pContactJoint->setPenetration(penetration);
		}

		math::float3 ContactJoint::GetContact() const
		{
			const physx::PxVec3 contact = m_pContactJoint->getContact();
			return Convert<const math::float3>(contact);
		}

		math::float3 ContactJoint::GetContactNormal() const
		{
			const physx::PxVec3 contactNormal = m_pContactJoint->getContactNormal();
			return Convert<const math::float3>(contactNormal);
		}

		float ContactJoint::GetPenetration() const
		{
			return m_pContactJoint->getPenetration();
		}

		float ContactJoint::GetResititution() const
		{
			return m_pContactJoint->getResititution();
		}

		void ContactJoint::SetResititution(float resititution)
		{
			m_pContactJoint->setResititution(resititution);
		}

		float ContactJoint::GetBounceThreshold() const
		{
			return m_pContactJoint->getBounceThreshold();
		}

		void ContactJoint::SetBounceThreshold(float bounceThreshold)
		{
			m_pContactJoint->setBounceThreshold(bounceThreshold);
		}

		D6Joint::D6Joint(physx::PxD6Joint* pD6Joint)
			: m_pD6Joint(pD6Joint)
		{
			m_pD6Joint->userData = this;
		}

		D6Joint::~D6Joint()
		{
			m_pD6Joint->userData = nullptr;
			m_pD6Joint->release();
			m_pD6Joint = nullptr;
		}
		ImplJointFunction(D6Joint, m_pD6Joint);

		void D6Joint::SetMotion(Axis emAxis, Motion emMotionType)
		{
			m_pD6Joint->setMotion(Convert<physx::PxD6Axis::Enum>(emAxis), Convert<physx::PxD6Motion::Enum>(emMotionType));
		}

		D6Joint::Motion D6Joint::GetMotion(Axis emAxis) const
		{
			const physx::PxD6Motion::Enum emMotionType = m_pD6Joint->getMotion(Convert<physx::PxD6Axis::Enum>(emAxis));
			return Convert<const D6Joint::Motion>(emMotionType);
		}

		float D6Joint::GetTwistAngle() const
		{
			return m_pD6Joint->getTwistAngle();
		}

		float D6Joint::GetSwingYAngle() const
		{
			return m_pD6Joint->getSwingYAngle();
		}

		float D6Joint::GetSwingZAngle() const
		{
			return m_pD6Joint->getSwingZAngle();
		}

		void D6Joint::SetDistanceLimit(const JointLinearLimit& limit)
		{
			m_pD6Joint->setDistanceLimit(Convert<const physx::PxJointLinearLimit>(limit));
		}

		JointLinearLimit D6Joint::GetDistanceLimit() const
		{
			const physx::PxJointLinearLimit limit = m_pD6Joint->getDistanceLimit();
			return Convert<const JointLinearLimit>(limit);
		}

		void D6Joint::SetLinearLimit(const JointLinearLimit& limit)
		{
			m_pD6Joint->setLinearLimit(Convert<const physx::PxJointLinearLimit>(limit));
		}

		JointLinearLimit D6Joint::GetLinearLimit() const
		{
			const physx::PxJointLinearLimit limit = m_pD6Joint->getLinearLimit();
			return Convert<const JointLinearLimit>(limit);
		}

		void D6Joint::SetTwistLimit(const JointAngularLimitPair& limit)
		{
			m_pD6Joint->setTwistLimit(Convert<const physx::PxJointAngularLimitPair>(limit));
		}

		JointAngularLimitPair D6Joint::GetTwistLimit() const
		{
			const physx::PxJointAngularLimitPair limit = m_pD6Joint->getTwistLimit();
			return Convert<const JointAngularLimitPair>(limit);
		}

		void D6Joint::SetSwingLimit(const JointLimitCone& limit)
		{
			m_pD6Joint->setSwingLimit(Convert<const physx::PxJointLimitCone>(limit));
		}

		JointLimitCone D6Joint::GetSwingLimit() const
		{
			const physx::PxJointLimitCone limit = m_pD6Joint->getSwingLimit();
			return Convert<const JointLimitCone>(limit);
		}

		void D6Joint::SetPyramidSwingLimit(const JointLimitPyramid& limit)
		{
			m_pD6Joint->setPyramidSwingLimit(Convert<const physx::PxJointLimitPyramid>(limit));
		}

		JointLimitPyramid D6Joint::GetPyramidSwingLimit() const
		{
			const physx::PxJointLimitPyramid limit = m_pD6Joint->getPyramidSwingLimit();
			return Convert<const JointLimitPyramid>(limit);
		}

		void D6Joint::SetDrive(Drive emDriveIndex, const JointDrive& drive)
		{
			m_pD6Joint->setDrive(Convert<physx::PxD6Drive::Enum>(emDriveIndex), Convert<const physx::PxD6JointDrive>(drive));
		}

		D6Joint::JointDrive D6Joint::GetDrive(Drive emDriveIndex) const
		{
			const physx::PxD6JointDrive jointDrive = m_pD6Joint->getDrive(Convert<physx::PxD6Drive::Enum>(emDriveIndex));
			return Convert<const D6Joint::JointDrive>(jointDrive);
		}

		void D6Joint::SetDrivePosition(const Transform& pose, bool isEnableAutoWake)
		{
			m_pD6Joint->setDrivePosition(Convert<const physx::PxTransform>(pose), isEnableAutoWake);
		}

		Transform D6Joint::GetDrivePosition() const
		{
			const physx::PxTransform transform = m_pD6Joint->getDrivePosition();
			return Convert<const Transform>(transform);
		}

		void D6Joint::SetDriveVelocity(const math::float3& linear, const math::float3& angular, bool isEnableAutoWake)
		{
			m_pD6Joint->setDriveVelocity(Convert<const physx::PxVec3>(linear), Convert<const physx::PxVec3>(angular), isEnableAutoWake);
		}

		void D6Joint::GetDriveVelocity(math::float3& linear, math::float3& angular) const
		{
			m_pD6Joint->getDriveVelocity(Convert<physx::PxVec3>(linear), Convert<physx::PxVec3>(angular));
		}

		void D6Joint::SetProjectionLinearTolerance(float tolerance)
		{
			m_pD6Joint->setProjectionLinearTolerance(tolerance);
		}

		float D6Joint::GetProjectionLinearTolerance() const
		{
			return m_pD6Joint->getProjectionLinearTolerance();
		}

		void D6Joint::SetProjectionAngularTolerance(float tolerance)
		{
			m_pD6Joint->setProjectionAngularTolerance(tolerance);
		}

		float D6Joint::GetProjectionAngularTolerance() const
		{
			return m_pD6Joint->getProjectionAngularTolerance();
		}

		DistanceJoint::DistanceJoint(physx::PxDistanceJoint* pDistanceJoint)
			: m_pDistanceJoint(pDistanceJoint)
		{
			m_pDistanceJoint->userData = this;
		}

		DistanceJoint::~DistanceJoint()
		{
			m_pDistanceJoint->userData = nullptr;
			m_pDistanceJoint->release();
			m_pDistanceJoint = nullptr;
		}
		ImplJointFunction(DistanceJoint, m_pDistanceJoint);

		float DistanceJoint::GetDistance() const
		{
			return m_pDistanceJoint->getDistance();
		}

		void DistanceJoint::SetMinDistance(float distance)
		{
			m_pDistanceJoint->setMinDistance(distance);
		}

		float DistanceJoint::GetMinDistance() const
		{
			return m_pDistanceJoint->getMinDistance();
		}

		void DistanceJoint::SetMaxDistance(float distance)
		{
			m_pDistanceJoint->setMaxDistance(distance);
		}

		float DistanceJoint::GetMaxDistance() const
		{
			return m_pDistanceJoint->getMaxDistance();
		}

		void DistanceJoint::SetTolerance(float tolerance)
		{
			m_pDistanceJoint->setTolerance(tolerance);
		}

		float DistanceJoint::GetTolerance() const
		{
			return m_pDistanceJoint->getTolerance();
		}

		void DistanceJoint::SetStiffness(float stiffness)
		{
			m_pDistanceJoint->setStiffness(stiffness);
		}

		float DistanceJoint::GetStiffness() const
		{
			return m_pDistanceJoint->getStiffness();
		}

		void DistanceJoint::SetDamping(float damping)
		{
			m_pDistanceJoint->setDamping(damping);
		}

		float DistanceJoint::GetDamping() const
		{
			return m_pDistanceJoint->getDamping();
		}

		void DistanceJoint::SetDistanceJointFlags(Flags flags)
		{
			m_pDistanceJoint->setDistanceJointFlags(Convert<physx::PxDistanceJointFlags>(flags));
		}

		void DistanceJoint::SetDistanceJointFlags(Flag flag, bool isEnable)
		{
			m_pDistanceJoint->setDistanceJointFlag(Convert<physx::PxDistanceJointFlag::Enum>(flag), isEnable);
		}

		DistanceJoint::Flags DistanceJoint::GetDistanceJointFlags() const
		{
			const physx::PxDistanceJointFlags flags = m_pDistanceJoint->getDistanceJointFlags();
			return Convert<const DistanceJoint::Flags>(flags);
		}

		FixedJoint::FixedJoint(physx::PxFixedJoint* pFixedJoint)
			: m_pFixedJoint(pFixedJoint)
		{
			m_pFixedJoint->userData = this;
		}

		FixedJoint::~FixedJoint()
		{
			m_pFixedJoint->userData = nullptr;
			m_pFixedJoint->release();
			m_pFixedJoint = nullptr;
		}
		ImplJointFunction(FixedJoint, m_pFixedJoint);

		void FixedJoint::SetProjectionLinearTolerance(float tolerance)
		{
			m_pFixedJoint->setProjectionLinearTolerance(tolerance);
		}

		float FixedJoint::GetProjectionLinearTolerance() const
		{
			return m_pFixedJoint->getProjectionLinearTolerance();
		}

		void FixedJoint::SetProjectionAngularTolerance(float tolerance)
		{
			m_pFixedJoint->setProjectionAngularTolerance(tolerance);
		}

		float FixedJoint::GetProjectionAngularTolerance() const
		{
			return m_pFixedJoint->getProjectionAngularTolerance();
		}

		PrismaticJoint::PrismaticJoint(physx::PxPrismaticJoint* pPrismaticJoint)
			: m_pPrismaticJoint(pPrismaticJoint)
		{
			m_pPrismaticJoint->userData = this;
		}

		PrismaticJoint::~PrismaticJoint()
		{
			m_pPrismaticJoint->userData = nullptr;
			m_pPrismaticJoint->release();
			m_pPrismaticJoint = nullptr;
		}
		ImplJointFunction(PrismaticJoint, m_pPrismaticJoint);

		float PrismaticJoint::GetPosition() const
		{
			return m_pPrismaticJoint->getPosition();
		}

		float PrismaticJoint::GetVelocity() const
		{
			return m_pPrismaticJoint->getVelocity();
		}

		void PrismaticJoint::SetLimit(const JointLinearLimitPair& limit)
		{
			m_pPrismaticJoint->setLimit(Convert<const physx::PxJointLinearLimitPair>(limit));
		}

		JointLinearLimitPair PrismaticJoint::GetLimit() const
		{
			const physx::PxJointLinearLimitPair limit = m_pPrismaticJoint->getLimit();
			return Convert<const JointLinearLimitPair>(limit);
		}

		void PrismaticJoint::SetPrismaticJointFlags(Flags flags)
		{
			m_pPrismaticJoint->setPrismaticJointFlags(Convert<physx::PxPrismaticJointFlags>(flags));
		}

		void PrismaticJoint::SetPrismaticJointFlag(Flag flag, bool isEnable)
		{
			m_pPrismaticJoint->setPrismaticJointFlag(Convert<physx::PxPrismaticJointFlag::Enum>(flag), isEnable);
		}

		PrismaticJoint::Flags PrismaticJoint::GetPrismaticJointFlags() const
		{
			const physx::PxPrismaticJointFlags flags = m_pPrismaticJoint->getPrismaticJointFlags();
			return Convert<const PrismaticJoint::Flags>(flags);
		}

		void PrismaticJoint::SetProjectionLinearTolerance(float tolerance)
		{
			m_pPrismaticJoint->setProjectionLinearTolerance(tolerance);
		}

		float PrismaticJoint::GetProjectionLinearTolerance() const
		{
			return m_pPrismaticJoint->getProjectionLinearTolerance();
		}

		void PrismaticJoint::SetProjectionAngularTolerance(float tolerance)
		{
			m_pPrismaticJoint->setProjectionAngularTolerance(tolerance);
		}

		float PrismaticJoint::GetProjectionAngularTolerance() const
		{
			return m_pPrismaticJoint->getProjectionAngularTolerance();
		}

		RevoluteJoint::RevoluteJoint(physx::PxRevoluteJoint* pRevoluteJoint)
			: m_pRevoluteJoint(pRevoluteJoint)
		{
			m_pRevoluteJoint->userData = this;
		}

		RevoluteJoint::~RevoluteJoint()
		{
			m_pRevoluteJoint->userData = nullptr;
			m_pRevoluteJoint->release();
			m_pRevoluteJoint = nullptr;
		}
		ImplJointFunction(RevoluteJoint, m_pRevoluteJoint);

		float RevoluteJoint::GetAngle() const
		{
			return m_pRevoluteJoint->getAngle();
		}

		float RevoluteJoint::GetVelocity() const
		{
			return m_pRevoluteJoint->getVelocity();
		}

		void RevoluteJoint::SetLimit(const JointAngularLimitPair& limit)
		{
			m_pRevoluteJoint->setLimit(Convert<const physx::PxJointAngularLimitPair>(limit));
		}

		JointAngularLimitPair RevoluteJoint::GetLimit() const
		{
			const physx::PxJointAngularLimitPair limit = m_pRevoluteJoint->getLimit();
			return Convert<const JointAngularLimitPair>(limit);
		}

		void RevoluteJoint::SetDriveVelocity(float velocity, bool isEnableAutoWake)
		{
			m_pRevoluteJoint->setDriveVelocity(velocity, isEnableAutoWake);
		}

		float RevoluteJoint::GetDriveVelocity() const
		{
			return m_pRevoluteJoint->getDriveVelocity();
		}

		void RevoluteJoint::SetDriveForceLimit(float limit)
		{
			m_pRevoluteJoint->setDriveForceLimit(limit);
		}

		float RevoluteJoint::GetDriveForceLimit() const
		{
			return m_pRevoluteJoint->getDriveForceLimit();
		}

		void RevoluteJoint::SetDriveGearRatio(float ratio)
		{
			m_pRevoluteJoint->setDriveGearRatio(ratio);
		}

		float RevoluteJoint::GetDriveGearRatio() const
		{
			return m_pRevoluteJoint->getDriveGearRatio();
		}

		void RevoluteJoint::SetRevoluteJointFlags(Flags flags)
		{
			m_pRevoluteJoint->setRevoluteJointFlags(Convert<physx::PxRevoluteJointFlags>(flags));
		}

		void RevoluteJoint::SetRevoluteJointFlag(Flag flag, bool isEnable)
		{
			m_pRevoluteJoint->setRevoluteJointFlag(Convert<physx::PxRevoluteJointFlag::Enum>(flag), isEnable);
		}

		RevoluteJoint::Flags RevoluteJoint::GetRevoluteJointFlags() const
		{
			const physx::PxRevoluteJointFlags flags = m_pRevoluteJoint->getRevoluteJointFlags();
			return Convert<const RevoluteJoint::Flags>(flags);
		}

		void RevoluteJoint::SetProjectionLinearTolerance(float tolerance)
		{
			m_pRevoluteJoint->setProjectionLinearTolerance(tolerance);
		}

		float RevoluteJoint::GetProjectionLinearTolerance() const
		{
			return m_pRevoluteJoint->getProjectionLinearTolerance();
		}

		void RevoluteJoint::SetProjectionAngularTolerance(float tolerance)
		{
			m_pRevoluteJoint->setProjectionAngularTolerance(tolerance);
		}

		float RevoluteJoint::GetProjectionAngularTolerance() const
		{
			return m_pRevoluteJoint->getProjectionAngularTolerance();
		}

		SphericalJoint::SphericalJoint(physx::PxSphericalJoint* pSphericalJoint)
			: m_pSphericalJoint(pSphericalJoint)
		{
			m_pSphericalJoint->userData = this;
		}

		SphericalJoint::~SphericalJoint()
		{
			m_pSphericalJoint->userData = nullptr;
			m_pSphericalJoint->release();
			m_pSphericalJoint = nullptr;
		}
		ImplJointFunction(SphericalJoint, m_pSphericalJoint);

		JointLimitCone SphericalJoint::GetLimitCone() const
		{
			const physx::PxJointLimitCone limit = m_pSphericalJoint->getLimitCone();
			return Convert<const JointLimitCone>(limit);
		}

		void SphericalJoint::SetLimitCone(const JointLimitCone& limit)
		{
			m_pSphericalJoint->setLimitCone(Convert<const physx::PxJointLimitCone>(limit));
		}

		float SphericalJoint::GetSwingYAngle() const
		{
			return m_pSphericalJoint->getSwingYAngle();
		}

		float SphericalJoint::GetSwingZAngle() const
		{
			return m_pSphericalJoint->getSwingZAngle();
		}

		void SphericalJoint::SetSphericalJointFlags(Flags flags)
		{
			m_pSphericalJoint->setSphericalJointFlags(Convert<physx::PxSphericalJointFlags>(flags));
		}

		void SphericalJoint::SetSphericalJointFlag(Flag flag, bool isEnable)
		{
			m_pSphericalJoint->setSphericalJointFlag(Convert<physx::PxSphericalJointFlag::Enum>(flag), isEnable);
		}

		SphericalJoint::Flags SphericalJoint::GetSphericalJointFlags() const
		{
			const physx::PxSphericalJointFlags flags = m_pSphericalJoint->getSphericalJointFlags();
			return Convert<const SphericalJoint::Flags>(flags);
		}

		void SphericalJoint::SetProjectionLinearTolerance(float tolerance)
		{
			m_pSphericalJoint->setProjectionLinearTolerance(tolerance);
		}

		float SphericalJoint::GetProjectionLinearTolerance() const
		{
			return m_pSphericalJoint->getProjectionLinearTolerance();
		}
	}
}