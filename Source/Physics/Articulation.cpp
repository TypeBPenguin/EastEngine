#include "stdafx.h"
#include "Articulation.h"

#include "PhysicsUtil.h"
#include "RigidBody.h"

namespace est
{
	namespace physics
	{
#define ImplArticulationBase(ClassName, m_pArticulationBase)																							\
		void ClassName::SetSolverIterationCounts(uint32_t minPositionIters, uint32_t minVelocityIters)													\
		{																																				\
			m_pArticulationBase->setSolverIterationCounts(minPositionIters, minVelocityIters);															\
		}																																				\
																																						\
		void ClassName::GetSolverIterationCounts(uint32_t& minPositionIters, uint32_t& minVelocityIters) const											\
		{																																				\
			m_pArticulationBase->setSolverIterationCounts(minPositionIters, minVelocityIters);															\
		}																																				\
																																						\
		bool ClassName::IsSleeping() const																												\
		{																																				\
			return m_pArticulationBase->isSleeping();																									\
		}																																				\
																																						\
		void ClassName::SetSleepThreshold(float threshold)																								\
		{																																				\
			m_pArticulationBase->setSleepThreshold(threshold);																							\
		}																																				\
																																						\
		float ClassName::GetSleepThreshold() const																										\
		{																																				\
			return m_pArticulationBase->getSleepThreshold();																							\
		}																																				\
																																						\
		void ClassName::SetStabilizationThreshold(float threshold)																						\
		{																																				\
			m_pArticulationBase->setStabilizationThreshold(threshold);																					\
		}																																				\
																																						\
		float ClassName::GetStabilizationThreshold() const																								\
		{																																				\
			return m_pArticulationBase->getStabilizationThreshold();																					\
		}																																				\
																																						\
		void ClassName::SetWakeCounter(float wakeCounterValue)																							\
		{																																				\
			m_pArticulationBase->setWakeCounter(wakeCounterValue);																						\
		}																																				\
																																						\
		float ClassName::GetWakeCounter() const																											\
		{																																				\
			return m_pArticulationBase->getWakeCounter();																								\
		}																																				\
																																						\
		void ClassName::WakeUp()																														\
		{																																				\
			m_pArticulationBase->wakeUp();																												\
		}																																				\
																																						\
		void ClassName::PutToSleep()																													\
		{																																				\
			m_pArticulationBase->putToSleep();																											\
		}																																				\
																																						\
		IArticulationLink* ClassName::CreateLink(IArticulationLink* pParent, const Transform& pose)														\
		{																																				\
			ArticulationLink* pLinkParent = static_cast<ArticulationLink*>(pParent);																	\
																																						\
			physx::PxArticulationLink* pLink = m_pArticulationBase->createLink(static_cast<physx::PxArticulationLink*>(pLinkParent->GetInterface()), Convert<const physx::PxTransform>(pose));	\
			if (pLink != nullptr)																														\
			{																																			\
				return m_articulationLinks.emplace_back(std::make_unique<ArticulationLink>(pLink)).get();												\
			}																																			\
																																						\
			return nullptr;																																\
		}																																				\
																																						\
		uint32_t ClassName::GetNumLinks() const																											\
		{																																				\
			return m_pArticulationBase->getNbLinks();																									\
		}																																				\
																																						\
		uint32_t ClassName::GetLinks(IArticulationLink** ppUserBuffer, uint32_t bufferSize, uint32_t startIndex) const									\
		{																																				\
			std::vector<physx::PxArticulationLink*> pInterfaces(bufferSize);																			\
			const uint32_t result = m_pArticulationBase->getLinks(pInterfaces.data(), bufferSize, startIndex);											\
																																						\
			for (uint32_t i = 0; i < bufferSize; ++i)																									\
			{																																			\
				if (pInterfaces[i] != nullptr)																											\
				{																																		\
					ppUserBuffer[i] = static_cast<IArticulationLink*>(pInterfaces[i]->userData);														\
				}																																		\
			}																																			\
			return result;																																\
		}																																				\
																																						\
		void ClassName::SetName(const string::StringID& name)																							\
		{																																				\
			m_name = name;																																\
																																						\
			const std::string multiName = string::WideToMulti(m_name.c_str());																			\
			m_pArticulationBase->setName(multiName.c_str());																							\
		}																																				\
																																						\
		const string::StringID& ClassName::GetName() const																								\
		{																																				\
			return m_name;																																\
		}																																				\
																																						\
		Bounds ClassName::GetWorldBounds(float inflation) const																							\
		{																																				\
			const physx::PxBounds3 bounds = m_pArticulationBase->getWorldBounds(inflation);																\
			return Convert<const Bounds>(bounds);																										\
		}																																				\
																																						\
		IAggregate* ClassName::GetAggregate() const																										\
		{																																				\
			return m_pAggregate.get();																													\
		}

		Articulation::Articulation(physx::PxArticulation* pArticulation)
			: m_pArticulation(pArticulation)
		{
			m_pArticulation->userData = this;
		}

		Articulation::~Articulation()
		{
			Remove(GetScene());

			m_pArticulation->release();
			m_pArticulation->userData = nullptr;
			m_pArticulation = nullptr;
		}
		ImplArticulationBase(Articulation, m_pArticulation);

		void Articulation::SetMaxProjectionIterations(uint32_t iterations)
		{
			m_pArticulation->setMaxProjectionIterations(iterations);
		}

		uint32_t Articulation::GetMaxProjectionIterations() const
		{
			return m_pArticulation->getMaxProjectionIterations();
		}

		void Articulation::SetSeparationTolerance(float tolerance)
		{
			m_pArticulation->setSeparationTolerance(tolerance);
		}

		float Articulation::GetSeparationTolerance() const
		{
			return m_pArticulation->getSeparationTolerance();
		}

		void Articulation::SetInternalDriveIterations(uint32_t iterations)
		{
			m_pArticulation->setInternalDriveIterations(iterations);
		}

		uint32_t Articulation::GetInternalDriveIterations() const
		{
			return m_pArticulation->getInternalDriveIterations();
		}

		void Articulation::SetExternalDriveIterations(uint32_t iterations)
		{
			m_pArticulation->setExternalDriveIterations(iterations);
		}

		uint32_t Articulation::GetExternalDriveIterations() const
		{
			return m_pArticulation->getExternalDriveIterations();
		}

		void Articulation::Remove(physx::PxScene* pScene, bool isEnableWakeOnLostTouch)
		{
			if (m_isValid == true)
			{
				pScene->removeArticulation(*GetInterface(), isEnableWakeOnLostTouch);
				m_isValid = false;
			}
		}

		ArticulationReducedCoordinate::ArticulationReducedCoordinate(physx::PxArticulationReducedCoordinate* pArticulationReducedCoordinate)
			: m_pArticulationReducedCoordinate(pArticulationReducedCoordinate)
		{
			m_pArticulationReducedCoordinate->userData = this;
		}

		ArticulationReducedCoordinate::~ArticulationReducedCoordinate()
		{
			Remove(GetScene());

			m_pArticulationReducedCoordinate->release();
			m_pArticulationReducedCoordinate->userData = nullptr;
			m_pArticulationReducedCoordinate = nullptr;
		}
		ImplArticulationBase(ArticulationReducedCoordinate, m_pArticulationReducedCoordinate);

		void ArticulationReducedCoordinate::SetArticulationFlags(Flags flags)
		{
			m_pArticulationReducedCoordinate->setArticulationFlags(Convert<physx::PxArticulationFlags>(flags));
		}

		void ArticulationReducedCoordinate::SetArticulationFlag(Flag flag, bool isEnable)
		{
			m_pArticulationReducedCoordinate->setArticulationFlag(Convert<physx::PxArticulationFlag::Enum>(flag), isEnable);
		}

		ArticulationReducedCoordinate::Flags ArticulationReducedCoordinate::GetArticulationFlags() const
		{
			const physx::PxArticulationFlags flags = m_pArticulationReducedCoordinate->getArticulationFlags();
			return Convert<const ArticulationReducedCoordinate::Flags>(flags);
		}

		uint32_t ArticulationReducedCoordinate::GetDofs() const
		{
			return m_pArticulationReducedCoordinate->getDofs();
		}

		void ArticulationReducedCoordinate::AddLoopJoint(IJoint* pJoint)
		{
			//m_joints.emplace_back(pJoint);
			//
			//m_pArticulationReducedCoordinate->addLoopJoint(pJoint);
		}

		void ArticulationReducedCoordinate::RemoveLoopJoint(IJoint* pJoint)
		{
			//m_pArticulationReducedCoordinate->removeLoopJoint();
		}

		uint32_t ArticulationReducedCoordinate::GetNumLoopJoints() const
		{
			return m_pArticulationReducedCoordinate->getNbLoopJoints();
		}

		uint32_t ArticulationReducedCoordinate::GetLoopJoints(IJoint** ppUserBuffer, uint32_t bufferSize, uint32_t startIndex) const
		{
			std::vector<physx::PxJoint*> pInterfaces(bufferSize);
			const uint32_t result = m_pArticulationReducedCoordinate->getLoopJoints(pInterfaces.data(), bufferSize, startIndex);

			for (uint32_t i = 0; i < bufferSize; ++i)
			{
				if (pInterfaces[i] != nullptr)
				{
					ppUserBuffer[i] = static_cast<IJoint*>(pInterfaces[i]->userData);
				}
			}
			return result;
		}

		uint32_t ArticulationReducedCoordinate::GetCoefficentMatrixSize() const
		{
			return m_pArticulationReducedCoordinate->getCoefficentMatrixSize();
		}

		void ArticulationReducedCoordinate::TeleportRootLink(const Transform& pose, bool isEnableAutoWake)
		{
			m_pArticulationReducedCoordinate->teleportRootLink(Convert<const physx::PxTransform>(pose), isEnableAutoWake);
		}

		void ArticulationReducedCoordinate::Remove(physx::PxScene* pScene, bool isEnableWakeOnLostTouch)
		{
			if (m_isValid == true)
			{
				pScene->removeArticulation(*GetInterface(), isEnableWakeOnLostTouch);
				m_isValid = false;
			}
		}

#define ImplArticulationJointBase(ClassType, Interface)															\
		IArticulationLink* ClassType::GetParentArticulationLink() const											\
		{																										\
			physx::PxArticulationLink& articulationLink = Interface->getParentArticulationLink();				\
			return static_cast<IArticulationLink*>(articulationLink.userData);									\
		}																										\
																												\
		void ClassType::SetParentPose(const Transform& pose)													\
		{																										\
			Interface->setParentPose(Convert<const physx::PxTransform>(pose));									\
		}																										\
																												\
		Transform ClassType::GetParentPose() const																\
		{																										\
			const physx::PxTransform transform = Interface->getParentPose();									\
			return Convert<const Transform>(transform);															\
		}																										\
																												\
		IArticulationLink* ClassType::GetChildArticulationLink() const											\
		{																										\
			physx::PxArticulationLink& articulationLink = Interface->getChildArticulationLink();				\
			return static_cast<IArticulationLink*>(articulationLink.userData);									\
		}																										\
																												\
		void ClassType::SetChildPose(const Transform& pose)														\
		{																										\
			Interface->setChildPose(Convert<const physx::PxTransform>(pose));									\
		}																										\
																												\
		Transform ClassType::GetChildPose() const																\
		{																										\
			const physx::PxTransform transform = Interface->getChildPose();										\
			return Convert<const Transform>(transform);															\
		}

		ArticulationJoint::ArticulationJoint(physx::PxArticulationJoint* pArticulationJoint)
			: m_pArticulationJoint(pArticulationJoint)
		{
		}

		ArticulationJoint::~ArticulationJoint()
		{
		}
		ImplArticulationJointBase(ArticulationJoint, m_pArticulationJoint);

		void ArticulationJoint::SetTargetOrientation(const math::Quaternion& orientation)
		{
			m_pArticulationJoint->setTargetOrientation(Convert<const physx::PxQuat>(orientation));
		}

		math::Quaternion ArticulationJoint::GetTargetOrientation() const
		{
			const physx::PxQuat orientation = m_pArticulationJoint->getTargetOrientation();
			return Convert<const math::Quaternion>(orientation);
		}

		void ArticulationJoint::SetTargetVelocity(const math::float3& velocity)
		{
			m_pArticulationJoint->setTargetVelocity(Convert<const physx::PxVec3>(velocity));
		}

		math::float3 ArticulationJoint::GetTargetVelocity() const
		{
			const physx::PxVec3 velocity = m_pArticulationJoint->getTargetVelocity();
			return Convert<const math::float3>(velocity);
		}

		void ArticulationJoint::SetDriveType(DriveType emDriveType)
		{
			m_pArticulationJoint->setDriveType(Convert<physx::PxArticulationJointDriveType::Enum>(emDriveType));
		}

		ArticulationJoint::DriveType ArticulationJoint::GetDriveType() const
		{
			const physx::PxArticulationJointDriveType::Enum driveType = m_pArticulationJoint->getDriveType();
			return Convert<const ArticulationJoint::DriveType>(driveType);
		}

		void ArticulationJoint::SetStiffness(float spring)
		{
			m_pArticulationJoint->setStiffness(spring);
		}

		float ArticulationJoint::GetStiffness() const
		{
			return m_pArticulationJoint->getStiffness();
		}

		void ArticulationJoint::SetDamping(float damping)
		{
			m_pArticulationJoint->setDamping(damping);
		}

		float ArticulationJoint::GetDamping() const
		{
			return m_pArticulationJoint->getDamping();
		}

		void ArticulationJoint::SetInternalCompliance(float compliance)
		{
			m_pArticulationJoint->setInternalCompliance(compliance);
		}

		float ArticulationJoint::GetInternalCompliance() const
		{
			return m_pArticulationJoint->getInternalCompliance();
		}

		void ArticulationJoint::SetExternalCompliance(float compliance)
		{
			m_pArticulationJoint->setExternalCompliance(compliance);
		}

		float ArticulationJoint::GetExternalCompliance() const
		{
			return m_pArticulationJoint->getExternalCompliance();
		}

		void ArticulationJoint::SetSwingLimit(float zLimit, float yLimit)
		{
			m_pArticulationJoint->setSwingLimit(zLimit, yLimit);
		}

		void ArticulationJoint::GetSwingLimit(float& zLimit, float& yLimit) const
		{
			m_pArticulationJoint->getSwingLimit(zLimit, yLimit);
		}

		void ArticulationJoint::SetTangentialStiffness(float spring)
		{
			m_pArticulationJoint->setTangentialStiffness(spring);
		}

		float ArticulationJoint::GetTangentialStiffness() const
		{
			return m_pArticulationJoint->getTangentialStiffness();
		}

		void ArticulationJoint::SetTangentialDamping(float damping)
		{
			m_pArticulationJoint->setTangentialDamping(damping);
		}

		float ArticulationJoint::GetTangentialDamping() const
		{
			return m_pArticulationJoint->getTangentialDamping();
		}

		void ArticulationJoint::SetSwingLimitContactDistance(float contactDistance)
		{
			m_pArticulationJoint->setSwingLimitContactDistance(contactDistance);
		}

		float ArticulationJoint::GetSwingLimitContactDistance() const
		{
			return m_pArticulationJoint->getSwingLimitContactDistance();
		}

		void ArticulationJoint::SetSwingLimitEnabled(bool isEnabled)
		{
			m_pArticulationJoint->setSwingLimitEnabled(isEnabled);
		}

		bool ArticulationJoint::GetSwingLimitEnabled() const
		{
			return m_pArticulationJoint->getSwingLimitEnabled();
		}

		void ArticulationJoint::SetTwistLimit(float lower, float upper)
		{
			m_pArticulationJoint->setTwistLimit(lower, upper);
		}

		void ArticulationJoint::GetTwistLimit(float& lower, float& upper) const
		{
			m_pArticulationJoint->getTwistLimit(lower, upper);
		}

		void ArticulationJoint::SetTwistLimitEnabled(bool isEnabled)
		{
			m_pArticulationJoint->setTwistLimitEnabled(isEnabled);
		}

		bool ArticulationJoint::GetTwistLimitEnabled() const
		{
			return m_pArticulationJoint->getTwistLimitEnabled();
		}

		void ArticulationJoint::SetTwistLimitContactDistance(float contactDistance)
		{
			m_pArticulationJoint->setTwistLimitContactDistance(contactDistance);
		}

		float ArticulationJoint::GetTwistLimitContactDistance() const
		{
			return m_pArticulationJoint->getTwistLimitContactDistance();
		}

		ArticulationJointReducedCoordinate::ArticulationJointReducedCoordinate(physx::PxArticulationJointReducedCoordinate* pArticulationJointBase)
			: m_pArticulationJointReducedCoordinate(pArticulationJointBase)
		{
		}

		ArticulationJointReducedCoordinate::~ArticulationJointReducedCoordinate()
		{
		}
		ImplArticulationJointBase(ArticulationJointReducedCoordinate, m_pArticulationJointReducedCoordinate);

		void ArticulationJointReducedCoordinate::SetJointType(Type emJointType)
		{
			m_pArticulationJointReducedCoordinate->setJointType(Convert<physx::PxArticulationJointType::Enum>(emJointType));
		}

		ArticulationJointReducedCoordinate::Type ArticulationJointReducedCoordinate::GetJointType() const
		{
			const physx::PxArticulationJointType::Enum emJointType = m_pArticulationJointReducedCoordinate->getJointType();
			return Convert<const ArticulationJointReducedCoordinate::Type>(emJointType);
		}

		void ArticulationJointReducedCoordinate::SetMotion(Axis emAxis, Motion emMotion)
		{
			m_pArticulationJointReducedCoordinate->setMotion(
				Convert<physx::PxArticulationAxis::Enum>(emAxis),
				Convert<physx::PxArticulationMotion::Enum>(emMotion));
		}

		ArticulationJointReducedCoordinate::Motion ArticulationJointReducedCoordinate::GetMotion(Axis emAxis) const
		{
			const physx::PxArticulationMotion::Enum emMotion = m_pArticulationJointReducedCoordinate->getMotion(Convert<physx::PxArticulationAxis::Enum>(emAxis));
			return Convert<const ArticulationJointReducedCoordinate::Motion>(emMotion);
		}

		void ArticulationJointReducedCoordinate::SetLimit(Axis emAxis, const float lowLimit, const float highLimit)
		{
			m_pArticulationJointReducedCoordinate->setLimit(Convert<physx::PxArticulationAxis::Enum>(emAxis), lowLimit, highLimit);
		}

		void ArticulationJointReducedCoordinate::GetLimit(Axis emAxis, float& lowLimit, float& highLimit)
		{
			m_pArticulationJointReducedCoordinate->getLimit(Convert<physx::PxArticulationAxis::Enum>(emAxis), lowLimit, highLimit);
		}

		void ArticulationJointReducedCoordinate::SetDrive(Axis emAxis, const float stiffness, const float damping, const float maxForce, bool isAccelerationDrive)
		{
			m_pArticulationJointReducedCoordinate->setDrive(Convert<physx::PxArticulationAxis::Enum>(emAxis), stiffness, damping, maxForce, isAccelerationDrive);
		}

		void ArticulationJointReducedCoordinate::GetDrive(Axis emAxis, float& stiffness, float& damping, float& maxForce, bool& isAcceleration)
		{
			m_pArticulationJointReducedCoordinate->getDrive(Convert<physx::PxArticulationAxis::Enum>(emAxis), stiffness, damping, maxForce, isAcceleration);
		}

		void ArticulationJointReducedCoordinate::SetDriveTarget(Axis emAxis, const float target)
		{
			m_pArticulationJointReducedCoordinate->setDriveTarget(Convert<physx::PxArticulationAxis::Enum>(emAxis), target);
		}

		void ArticulationJointReducedCoordinate::SetDriveVelocity(Axis emAxis, const float targetVel)
		{
			m_pArticulationJointReducedCoordinate->setDriveVelocity(Convert<physx::PxArticulationAxis::Enum>(emAxis), targetVel);
		}

		float ArticulationJointReducedCoordinate::GetDriveTarget(Axis emAxis)
		{
			return m_pArticulationJointReducedCoordinate->getDriveTarget(Convert<physx::PxArticulationAxis::Enum>(emAxis));
		}

		float ArticulationJointReducedCoordinate::GetDriveVelocity(Axis emAxis)
		{
			return m_pArticulationJointReducedCoordinate->getDriveVelocity(Convert<physx::PxArticulationAxis::Enum>(emAxis));
		}

		void ArticulationJointReducedCoordinate::SetFrictionCoefficient(const float coefficient)
		{
			m_pArticulationJointReducedCoordinate->setFrictionCoefficient(coefficient);
		}

		float ArticulationJointReducedCoordinate::GetFrictionCoefficient() const
		{
			return m_pArticulationJointReducedCoordinate->getFrictionCoefficient();
		}

		void ArticulationJointReducedCoordinate::SetMaxJointVelocity(const float maxJointV)
		{
			m_pArticulationJointReducedCoordinate->setMaxJointVelocity(maxJointV);
		}

		float ArticulationJointReducedCoordinate::GetMaxJointVelocity() const
		{
			return m_pArticulationJointReducedCoordinate->getMaxJointVelocity();
		}
	}
}