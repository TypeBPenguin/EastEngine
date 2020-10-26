#include "stdafx.h"
#include "RigidBody.h"

#include "PhysicsUtil.h"
#include "PhysicsSystem.h"
#include "PhysicsMaterial.h"
#include "PhysicsShape.h"

#include "Articulation.h"

namespace est
{
	namespace physics
	{
#define ImplActorFunc(Actor, m_pActor)													\
		void Actor::SetName(const string::StringID& name)								\
		{																				\
			m_name = name;																\
																						\
			const std::string multiName = string::WideToMulti(m_name.c_str());			\
			m_pActor->setName(multiName.c_str());										\
		}																				\
																						\
		const string::StringID& Actor::GetName() const									\
		{																				\
			return m_name;																\
		}																				\
																						\
		void Actor::SetUserData(void* pUserData)										\
		{																				\
			m_pUserData = pUserData;													\
		}																				\
																						\
		void* Actor::GetUserData() const												\
		{																				\
			return m_pUserData;															\
		}																				\
																						\
		Bounds Actor::GetWorldBounds(float inflation) const								\
		{																				\
			const physx::PxBounds3 bounds = m_pActor->getWorldBounds(inflation);		\
			return Convert<const Bounds>(bounds);										\
		}																				\
																						\
		void Actor::SetActorFlag(ActorFlag flag, bool isEnable)							\
		{																				\
			m_pActor->setActorFlag(Convert<physx::PxActorFlag::Enum>(flag), isEnable);	\
		}																				\
																						\
		void Actor::SetActorFlags(ActorFlags flags)										\
		{																				\
			m_pActor->setActorFlags(Convert<physx::PxActorFlags>(flags));				\
		}																				\
																						\
		Actor::ActorFlags Actor::GetActorFlags() const									\
		{																				\
			const physx::PxActorFlags flags = m_pActor->getActorFlags();				\
			return Convert<const ActorFlags>(flags);									\
		}																				\
																						\
		void Actor::SetDominanceGroup(DominanceGroup dominanceGroup)					\
		{																				\
			m_pActor->setDominanceGroup(dominanceGroup.group);							\
		}																				\
																						\
		Actor::DominanceGroup Actor::GetDominanceGroup() const							\
		{																				\
			return m_pActor->getDominanceGroup();										\
		}

#define ImplRigidActorFunc(RigidActor, m_pRigidActor)																																																	\
		void RigidActor::SetGlobalTransform(const Transform& pose, bool isEnableAutoWake)																																								\
		{																																																												\
			m_pRigidActor->setGlobalPose(Convert<const physx::PxTransform>(pose), isEnableAutoWake);																																					\
		}																																																												\
																																																														\
		Transform RigidActor::GetGlobalTransform() const																																																\
		{																																																												\
			const physx::PxTransform transform = m_pRigidActor->getGlobalPose();																																										\
			return Convert<const Transform>(transform);																																																	\
		}																																																												\
																																																														\
		void RigidActor::AttachShape(const std::shared_ptr<IShape>& pShape)																																												\
		{																																																												\
			auto iter = std::find(m_pShapes.begin(), m_pShapes.end(), pShape);																																											\
			if (iter == m_pShapes.end())																																																				\
			{																																																											\
				if (m_pRigidActor->attachShape(*util::GetInterface(pShape.get())) == false)																																			\
				{																																																										\
					LOG_ERROR(L"failed to attach shape");																																																\
				}																																																										\
				else																																																									\
				{																																																										\
					m_pShapes.emplace_back(pShape);																																																		\
				}																																																										\
			}																																																											\
		}																																																												\
																																																														\
		void RigidActor::DetachShape(const std::shared_ptr<IShape>& pShape, bool isEnableWakeOnLostTouch)																																				\
		{																																																												\
			auto iter = std::find(m_pShapes.begin(), m_pShapes.end(), pShape);																																											\
			if (iter != m_pShapes.end())																																																				\
			{																																																											\
				m_pRigidActor->detachShape(*util::GetInterface(pShape.get()), isEnableWakeOnLostTouch);																																\
				m_pShapes.erase(iter);																																																					\
			}																																																											\
		}																																																												\
																																																														\
		uint32_t RigidActor::GetNumShapes() const																																																		\
		{																																																												\
			return static_cast<uint32_t>(m_pShapes.size());																																																\
		}																																																												\
																																																														\
		uint32_t RigidActor::GetShapes(std::shared_ptr<IShape>* ppUserBuffer, uint32_t bufferSize, uint32_t startIndex) const																															\
		{																																																												\
			const uint32_t numShapes = GetNumShapes();																																																	\
			uint32_t result = 0;																																																						\
			for (uint32_t i = 0; i < bufferSize; ++i)																																																	\
			{																																																											\
				if (startIndex + i < numShapes)																																																			\
				{																																																										\
					ppUserBuffer[startIndex + i] = m_pShapes[i];																																														\
					++result;																																																							\
				}																																																										\
			}																																																											\
			return result;																																																								\
		}																																																												\
																																																														\
		std::shared_ptr<IShape> RigidActor::GetShape(uint32_t index) const																																												\
		{																																																												\
			if (index >= GetNumShapes())																																																				\
				return nullptr;																																																							\
																																																														\
			return m_pShapes[index];																																																					\
		}

#define ImplRigidBodyFunc(RigidBody, m_pRigidBody)																							\
		void RigidBody::SetCenterMassLocalPose(const Transform& pose)																		\
		{																																	\
			m_pRigidBody->setCMassLocalPose(Convert<const physx::PxTransform>(pose));														\
		}																																	\
																																			\
		Transform RigidBody::GetCenterMassLocalPose()																						\
		{																																	\
			const physx::PxTransform transform = m_pRigidBody->getCMassLocalPose();															\
			return Convert<const Transform>(transform);																						\
		}																																	\
																																			\
		void RigidBody::SetMass(float mass)																									\
		{																																	\
			m_pRigidBody->setMass(mass);																									\
		}																																	\
																																			\
		float RigidBody::GetMass() const																									\
		{																																	\
			return m_pRigidBody->getMass();																									\
		}																																	\
																																			\
		float RigidBody::GetInvMass() const																									\
		{																																	\
			return m_pRigidBody->getInvMass();																								\
		}																																	\
																																			\
		void RigidBody::SetMassSpaceInertiaTensor(const math::float3& m)																	\
		{																																	\
			m_pRigidBody->setMassSpaceInertiaTensor(Convert<const physx::PxVec3>(m));														\
		}																																	\
																																			\
		math::float3 RigidBody::GetMassSpaceInertiaTensor() const																			\
		{																																	\
			const physx::PxVec3 tensor = m_pRigidBody->getMassSpaceInertiaTensor();															\
			return Convert<const math::float3>(tensor);																						\
		}																																	\
																																			\
		math::float3 RigidBody::GetMassSpaceInvInertiaTensor() const																		\
		{																																	\
			const physx::PxVec3 invTensor = m_pRigidBody->getMassSpaceInertiaTensor();														\
			return Convert<const math::float3>(invTensor);																					\
		}																																	\
																																			\
		void RigidBody::SetLinearVelocity(const math::float3& linearVelocity, bool isEnableAutoWake)										\
		{																																	\
			m_pRigidBody->setLinearVelocity(Convert<const physx::PxVec3>(linearVelocity), isEnableAutoWake);								\
		}																																	\
																																			\
		math::float3 RigidBody::GetLinearVelocity() const																					\
		{																																	\
			const physx::PxVec3 linearVelocity = m_pRigidBody->getLinearVelocity();															\
			return Convert<const math::float3>(linearVelocity);																				\
		}																																	\
																																			\
		void RigidBody::SetAngularVelocity(const math::float3& angularVelocity, bool isEnableAutoWake)										\
		{																																	\
			m_pRigidBody->setAngularVelocity(Convert<const physx::PxVec3>(angularVelocity), isEnableAutoWake);								\
		}																																	\
																																			\
		math::float3 RigidBody::GetAngularVelocity() const																					\
		{																																	\
			const physx::PxVec3 angularVelocity = m_pRigidBody->getAngularVelocity();														\
			return Convert<const math::float3>(angularVelocity);																			\
		}																																	\
																																			\
		void RigidBody::AddForce(const math::float3& force, ForceMode mode, bool isEnableAutoWake)											\
		{																																	\
			m_pRigidBody->addForce(Convert<const physx::PxVec3>(force), Convert<physx::PxForceMode::Enum>(mode), isEnableAutoWake);			\
		}																																	\
																																			\
		void RigidBody::ClearForce(ForceMode mode)																							\
		{																																	\
			m_pRigidBody->clearForce(Convert<physx::PxForceMode::Enum>(mode));																\
		}																																	\
																																			\
		void RigidBody::AddTorque(const math::float3& torque, ForceMode mode, bool isEnableAutoWake)										\
		{																																	\
			m_pRigidBody->addTorque(Convert<const physx::PxVec3>(torque), Convert<physx::PxForceMode::Enum>(mode), isEnableAutoWake);		\
		}																																	\
																																			\
		void RigidBody::ClearTorque(ForceMode mode)																							\
		{																																	\
			m_pRigidBody->clearTorque(Convert<physx::PxForceMode::Enum>(mode));																\
		}																																	\
																																			\
		void RigidBody::SetFlag(Flag flag, bool isEnable)																					\
		{																																	\
			m_pRigidBody->setRigidBodyFlag(Convert<const physx::PxRigidBodyFlag::Enum>(flag), isEnable);									\
		}																																	\
																																			\
		void RigidBody::SetFlags(Flags flags)																								\
		{																																	\
			m_pRigidBody->setRigidBodyFlags(Convert<const physx::PxRigidBodyFlags>(flags));													\
		}																																	\
																																			\
		RigidBody::Flags RigidBody::GetFlags() const																						\
		{																																	\
			const physx::PxRigidBodyFlags flags = m_pRigidBody->getRigidBodyFlags();														\
			return Convert<const RigidBody::Flags>(flags);																					\
		}																																	\
																																			\
		void RigidBody::SetMinCCDAdvanceCoefficient(float advanceCoefficient)																\
		{																																	\
			m_pRigidBody->setMinCCDAdvanceCoefficient(advanceCoefficient);																	\
		}																																	\
																																			\
		float RigidBody::GetMinCCDAdvanceCoefficient() const																				\
		{																																	\
			return m_pRigidBody->getMinCCDAdvanceCoefficient();																				\
		}																																	\
																																			\
		void RigidBody::SetMaxDepenetrationVelocity(float biasClamp)																		\
		{																																	\
			m_pRigidBody->setMaxDepenetrationVelocity(biasClamp);																			\
		}																																	\
																																			\
		float RigidBody::GetMaxDepenetrationVelocity() const																				\
		{																																	\
			return m_pRigidBody->getMaxDepenetrationVelocity();																				\
		}																																	\
																																			\
		void RigidBody::SetMaxContactImpulse(float maxImpulse)																				\
		{																																	\
			m_pRigidBody->setMaxContactImpulse(maxImpulse);																					\
		}																																	\
																																			\
		float RigidBody::GetMaxContactImpulse() const																						\
		{																																	\
			return m_pRigidBody->getMaxContactImpulse();																					\
		}

		///////////////////////////////////////////////////////////////////////////////////

		RigidStatic::RigidStatic(physx::PxRigidStatic* pRigidStatic)
			: m_pRigidStatic(pRigidStatic)
		{
			m_pRigidStatic->userData = this;
		}

		RigidStatic::~RigidStatic()
		{
			Remove(GetScene());

			m_pRigidStatic->release();
			m_pRigidStatic->userData = nullptr;
			m_pRigidStatic = nullptr;
		}

		ImplActorFunc(RigidStatic, m_pRigidStatic);
		ImplRigidActorFunc(RigidStatic, m_pRigidStatic);

		void RigidStatic::Remove(physx::PxScene* pScene, bool isEnableWakeOnLostTouch)
		{
			if (m_isValid == true)
			{
				pScene->removeActor(*GetInterface(), isEnableWakeOnLostTouch);
				m_isValid = false;
			}
		}

		RigidDynamic::RigidDynamic(physx::PxRigidDynamic* pRigidDynamic)
			: m_pRigidDynamic(pRigidDynamic)
		{
			m_pRigidDynamic->userData = this;
		}

		RigidDynamic::~RigidDynamic()
		{
			Remove(GetScene());

			m_pRigidDynamic->release();
			m_pRigidDynamic->userData = nullptr;
			m_pRigidDynamic = nullptr;
		}
		ImplActorFunc(RigidDynamic, m_pRigidDynamic);
		ImplRigidActorFunc(RigidDynamic, m_pRigidDynamic);
		ImplRigidBodyFunc(RigidDynamic, m_pRigidDynamic);

		void RigidDynamic::SetKinematicTarget(const Transform& destination)
		{
			m_pRigidDynamic->setKinematicTarget(Convert<const physx::PxTransform>(destination));
		}

		bool RigidDynamic::GetKinematicTarget(Transform& target_out)
		{
			return m_pRigidDynamic->getKinematicTarget(Convert<physx::PxTransform>(target_out));
		}

		void RigidDynamic::SetLinearDamping(float linearDamp)
		{
			m_pRigidDynamic->setLinearDamping(linearDamp);
		}

		float RigidDynamic::GetLinearDamping() const
		{
			return m_pRigidDynamic->getLinearDamping();
		}

		void RigidDynamic::SetAngularDamping(float angularDamp)
		{
			m_pRigidDynamic->setAngularDamping(angularDamp);
		}

		float RigidDynamic::GetAngularDamping() const
		{
			return m_pRigidDynamic->getAngularDamping();
		}

		void RigidDynamic::SetMaxAngularVelocity(float maxAngularVelocity)
		{
			m_pRigidDynamic->setMaxAngularVelocity(maxAngularVelocity);
		}

		float RigidDynamic::GetMaxAngularVelocity() const
		{
			return m_pRigidDynamic->getMaxAngularVelocity();
		}

		bool RigidDynamic::IsSleeping() const
		{
			return m_pRigidDynamic->isSleeping();
		}

		void RigidDynamic::SetSleepThreshold(float threshold)
		{
			m_pRigidDynamic->setSleepThreshold(threshold);
		}

		float RigidDynamic::GetSleepThreshold() const
		{
			return m_pRigidDynamic->getSleepThreshold();
		}

		void RigidDynamic::SetStabilizationThreshold(float threshold)
		{
			m_pRigidDynamic->setStabilizationThreshold(threshold);
		}

		float RigidDynamic::GetStabilizationThreshold() const
		{
			return m_pRigidDynamic->getStabilizationThreshold();
		}

		void RigidDynamic::SetLockFlag(LockFlag flag, bool isEnable)
		{
			m_pRigidDynamic->setRigidDynamicLockFlag(Convert<physx::PxRigidDynamicLockFlag::Enum>(flag), isEnable);
		}

		void RigidDynamic::SetLockFlags(LockFlags flags)
		{
			m_pRigidDynamic->setRigidDynamicLockFlags(Convert<physx::PxRigidDynamicLockFlags>(flags));
		}

		void RigidDynamic::SetWakeCounter(float wakeCounterValue)
		{
			m_pRigidDynamic->setWakeCounter(wakeCounterValue);
		}

		float RigidDynamic::GetWakeCounter() const
		{
			return m_pRigidDynamic->getWakeCounter();
		}

		void RigidDynamic::WakeUp()
		{
			m_pRigidDynamic->wakeUp();
		}

		void RigidDynamic::PutToSleep()
		{
			m_pRigidDynamic->putToSleep();
		}

		void RigidDynamic::SetSolverIterationCounts(uint32_t minPositionIters, uint32_t minVelocityIters)
		{
			m_pRigidDynamic->setSolverIterationCounts(minPositionIters, minVelocityIters);
		}

		void RigidDynamic::GetSolverIterationCounts(uint32_t& minPositionIters, uint32_t& minVelocityIters) const
		{
			m_pRigidDynamic->getSolverIterationCounts(minPositionIters, minVelocityIters);
		}

		void RigidDynamic::SetContactReportThreshold(float threshold)
		{
			m_pRigidDynamic->setContactReportThreshold(threshold);
		}

		float RigidDynamic::GetContactReportThreshold() const
		{
			return m_pRigidDynamic->getContactReportThreshold();
		}

		void RigidDynamic::Remove(physx::PxScene* pScene, bool isEnableWakeOnLostTouch)
		{
			if (m_isValid == true)
			{
				pScene->removeActor(*GetInterface(), isEnableWakeOnLostTouch);
				m_isValid = false;
			}
		}

		ArticulationLink::ArticulationLink(physx::PxArticulationLink* pArticulationLink)
			: m_pArticulationLink(pArticulationLink)
		{
			m_pArticulationLink->userData = this;
		}

		ArticulationLink::~ArticulationLink()
		{
			Remove(GetScene());

			m_pArticulationLink->release();
			m_pArticulationLink->userData = nullptr;
			m_pArticulationLink = nullptr;
		}
		ImplActorFunc(ArticulationLink, m_pArticulationLink);
		ImplRigidActorFunc(ArticulationLink, m_pArticulationLink);
		ImplRigidBodyFunc(ArticulationLink, m_pArticulationLink);

		IArticulation* ArticulationLink::GetArticulation() const
		{
			return static_cast<IArticulation*>(m_pArticulationLink->getArticulation().userData);
		}

		IArticulationJointBase* ArticulationLink::GetInboundJoint()
		{
			if (m_pArticulationLink->getInboundJoint() == nullptr)
				return nullptr;

			if (m_pInboundJoint == nullptr)
			{
				physx::PxArticulationJointBase* pInboundJoint = m_pArticulationLink->getInboundJoint();
				if (pInboundJoint == nullptr)
					return nullptr;

				physx::PxArticulationJointReducedCoordinate* p = dynamic_cast<physx::PxArticulationJointReducedCoordinate*>(pInboundJoint);
				if (p != nullptr)
				{
					m_pInboundJoint = std::make_unique<ArticulationJointReducedCoordinate>(p);
				}
				else
				{
					physx::PxArticulationJoint* p2 = dynamic_cast<physx::PxArticulationJoint*>(pInboundJoint);
					if (p2 != nullptr)
					{
						m_pInboundJoint = std::make_unique<ArticulationJoint>(p2);
					}
				}
			}
			return m_pInboundJoint.get();
		}

		uint32_t ArticulationLink::GetNumChildren() const
		{
			return m_pArticulationLink->getNbChildren();
		}

		uint32_t ArticulationLink::GetChildren(IArticulationLink** ppUserBuffer, uint32_t bufferSize, uint32_t startIndex) const
		{
			std::vector<physx::PxArticulationLink*> pInterfaces(bufferSize);
			const uint32_t result = m_pArticulationLink->getChildren(pInterfaces.data(), bufferSize, startIndex);

			for (uint32_t i = 0; i < bufferSize; ++i)
			{
				if (pInterfaces[i] != nullptr)
				{
					ppUserBuffer[i] = static_cast<IArticulationLink*>(pInterfaces[i]->userData);
				}
			}
			return result;
		}

		void ArticulationLink::Remove(physx::PxScene* pScene, bool isEnableWakeOnLostTouch)
		{
			if (m_isValid == true)
			{
				pScene->removeActor(*GetInterface(), isEnableWakeOnLostTouch);
				m_isValid = false;
			}
		}
	}
}