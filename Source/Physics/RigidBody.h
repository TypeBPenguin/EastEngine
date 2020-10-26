#pragma once

#include "PhysicsInterface.h"

namespace est
{
	namespace physics
	{
#define DeclActorFunc																	\
		virtual void SetName(const string::StringID& name) override;					\
		virtual const string::StringID& GetName() const override;						\
		virtual void SetUserData(void* pUserData) override;								\
		virtual void* GetUserData() const override;										\
		virtual Bounds GetWorldBounds(float inflation = 1.01f) const override;			\
		virtual void SetActorFlag(ActorFlag flag, bool isEnable) override;				\
		virtual void SetActorFlags(ActorFlags flags) override;							\
		virtual ActorFlags GetActorFlags() const override;								\
		virtual void SetDominanceGroup(DominanceGroup dominanceGroup) override;			\
		virtual DominanceGroup GetDominanceGroup() const override;						\

#define DeclRigidActorFunc																												\
		virtual void SetGlobalTransform(const Transform& pose, bool isEnableAutoWake = true) override;									\
		virtual Transform GetGlobalTransform() const override;																			\
																																		\
		virtual void AttachShape(const std::shared_ptr<IShape>& pShape) override;														\
		virtual void DetachShape(const std::shared_ptr<IShape>& pShape, bool isEnableWakeOnLostTouch = true) override;					\
		virtual uint32_t GetNumShapes() const override;																					\
		virtual uint32_t GetShapes(std::shared_ptr<IShape>* ppUserBuffer, uint32_t bufferSize, uint32_t startIndex = 0) const override;	\
		virtual std::shared_ptr<IShape> GetShape(uint32_t index) const override;														\

#define DeclRigidBodyFunc																												\
		virtual void SetCenterMassLocalPose(const Transform& pose) override;															\
		virtual Transform GetCenterMassLocalPose() override;																			\
																																		\
		virtual void SetMass(float mass) override;																						\
		virtual float GetMass() const override;																							\
		virtual float GetInvMass() const override;																						\
																																		\
		virtual void SetMassSpaceInertiaTensor(const math::float3& m) override;															\
		virtual	math::float3 GetMassSpaceInertiaTensor() const override;																\
		virtual math::float3 GetMassSpaceInvInertiaTensor() const override;																\
																																		\
		virtual void SetLinearVelocity(const math::float3& linearVelocity, bool isEnableAutoWake = true) override;						\
		virtual math::float3 GetLinearVelocity() const override;																		\
																																		\
		virtual void SetAngularVelocity(const math::float3& angularVelocity, bool isEnableAutoWake = true) override;					\
		virtual math::float3 GetAngularVelocity() const override;																		\
																																		\
		virtual void AddForce(const math::float3& force, ForceMode mode = ForceMode::eForce, bool isEnableAutoWake = true) override;	\
		virtual void ClearForce(ForceMode mode = ForceMode::eForce) override;															\
																																		\
		virtual void AddTorque(const math::float3& torque, ForceMode mode = ForceMode::eForce, bool isEnableAutoWake = true) override;	\
		virtual void ClearTorque(ForceMode mode = ForceMode::eForce) override;															\
																																		\
		virtual void SetFlag(Flag flag, bool isEnable) override;																		\
		virtual void SetFlags(Flags flags) override;																					\
		virtual Flags GetFlags() const override;																						\
																																		\
		virtual void SetMinCCDAdvanceCoefficient(float advanceCoefficient) override;													\
		virtual float GetMinCCDAdvanceCoefficient() const override;																		\
																																		\
		virtual void SetMaxDepenetrationVelocity(float biasClamp) override;																\
		virtual float GetMaxDepenetrationVelocity() const override;																		\
																																		\
		virtual void SetMaxContactImpulse(float maxImpulse) override;																	\
		virtual float GetMaxContactImpulse() const override;

		class RigidStatic : public IRigidStatic
		{
		public:
			RigidStatic(physx::PxRigidStatic* pRigidStatic);
			virtual ~RigidStatic();

		public:
			DeclActorFunc;
			DeclRigidActorFunc;

		public:
			physx::PxRigidStatic* GetInterface() const { return m_pRigidStatic; }

		public:
			void Remove(physx::PxScene* pScene, bool isEnableWakeOnLostTouch = true);

		private:
			physx::PxRigidStatic* m_pRigidStatic{ nullptr };
			string::StringID m_name;
			void* m_pUserData{ nullptr };

			bool m_isValid{ true };

			std::vector<std::shared_ptr<IShape>> m_pShapes;
		};

		class RigidDynamic : public IRigidDynamic
		{
		public:
			RigidDynamic(physx::PxRigidDynamic* pRigidDynamic);
			virtual ~RigidDynamic();

		public:
			DeclActorFunc;
			DeclRigidActorFunc;
			DeclRigidBodyFunc;

		public:
			virtual void SetKinematicTarget(const Transform& destination) override;
			virtual bool GetKinematicTarget(Transform& target_out) override;

			virtual void SetLinearDamping(float linearDamp) override;
			virtual float GetLinearDamping() const override;

			virtual void SetAngularDamping(float angularDamp) override;
			virtual float GetAngularDamping() const override;

			virtual void SetMaxAngularVelocity(float maxAngularVelocity) override;
			virtual float GetMaxAngularVelocity() const override;

			virtual bool IsSleeping() const override;
			virtual void SetSleepThreshold(float threshold) override;
			virtual float GetSleepThreshold() const override;

			virtual void SetStabilizationThreshold(float threshold) override;
			virtual float GetStabilizationThreshold() const override;

			virtual void SetLockFlag(LockFlag flag, bool isEnable) override;
			virtual void SetLockFlags(LockFlags flags) override;

			virtual void SetWakeCounter(float wakeCounterValue) override;
			virtual float GetWakeCounter() const override;
			virtual void WakeUp() override;
			virtual void PutToSleep() override;

			virtual void SetSolverIterationCounts(uint32_t minPositionIters, uint32_t minVelocityIters = 1) override;
			virtual void GetSolverIterationCounts(uint32_t& minPositionIters, uint32_t& minVelocityIters) const override;

			virtual void SetContactReportThreshold(float threshold) override;
			virtual float GetContactReportThreshold() const override;

		public:
			physx::PxRigidDynamic* GetInterface() const { return m_pRigidDynamic; }

		public:
			void Remove(physx::PxScene* pScene, bool isEnableWakeOnLostTouch = true);

		private:
			physx::PxRigidDynamic* m_pRigidDynamic{ nullptr };
			string::StringID m_name;
			void* m_pUserData{ nullptr };

			bool m_isValid{ false };

			std::vector<std::shared_ptr<IShape>> m_pShapes;
		};

		class ArticulationLink : public IArticulationLink
		{
		public:
			ArticulationLink(physx::PxArticulationLink* pArticulationLink);
			virtual ~ArticulationLink();

		public:
			DeclActorFunc;
			DeclRigidActorFunc;
			DeclRigidBodyFunc;

		public:
			virtual IArticulation* GetArticulation() const override;
			virtual IArticulationJointBase* GetInboundJoint() override;

			virtual uint32_t GetNumChildren() const override;
			virtual uint32_t GetChildren(IArticulationLink** ppUserBuffer, uint32_t bufferSize, uint32_t startIndex = 0) const override;

		public:
			physx::PxArticulationLink* GetInterface() const { return m_pArticulationLink; }

		public:
			void Remove(physx::PxScene* pScene, bool isEnableWakeOnLostTouch = true);

		private:
			physx::PxArticulationLink* m_pArticulationLink{ nullptr };
			string::StringID m_name;
			void* m_pUserData{ nullptr };

			bool m_isValid{ false };

			std::vector<std::shared_ptr<IShape>> m_pShapes;

			std::unique_ptr<IArticulationJointBase> m_pInboundJoint;
		};
	}
}