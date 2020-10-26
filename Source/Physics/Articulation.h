#pragma once

#include "PhysicsInterface.h"

namespace est
{
	namespace physics
	{
#define DeclArticulationBase																										\
		virtual void SetSolverIterationCounts(uint32_t minPositionIters, uint32_t minVelocityIters = 1) override;					\
		virtual void GetSolverIterationCounts(uint32_t& minPositionIters, uint32_t& minVelocityIters) const override;				\
																																	\
		virtual bool IsSleeping() const override;																					\
		virtual void SetSleepThreshold(float threshold) override;																	\
		virtual float GetSleepThreshold() const override;																			\
																																	\
		virtual void SetStabilizationThreshold(float threshold) override;															\
		virtual float GetStabilizationThreshold() const override;																	\
																																	\
		virtual void SetWakeCounter(float wakeCounterValue) override;																\
		virtual float GetWakeCounter() const override;																				\
																																	\
		virtual void WakeUp() override;																								\
		virtual void PutToSleep() override;																							\
																																	\
		virtual IArticulationLink* CreateLink(IArticulationLink* pParent, const Transform& pose) override;							\
		virtual uint32_t GetNumLinks() const override;																				\
																																	\
		virtual uint32_t GetLinks(IArticulationLink** ppUserBuffer, uint32_t bufferSize, uint32_t startIndex = 0) const override;	\
																																	\
		virtual void SetName(const string::StringID& name) override;																\
		virtual const string::StringID& GetName() const override;																	\
																																	\
		virtual Bounds GetWorldBounds(float inflation = 1.01f) const override;														\
																																	\
		virtual IAggregate* GetAggregate() const override;																			\
																																	\
	protected:																														\
		string::StringID m_name;																									\
																																	\
		std::vector<std::unique_ptr<IArticulationLink>> m_articulationLinks;														\
		std::unique_ptr<IAggregate> m_pAggregate;

		class Articulation : public IArticulation
		{
		public:
			Articulation(physx::PxArticulation* pArticulation);
			virtual ~Articulation();

		public:
			DeclArticulationBase;

		public:
			virtual void SetMaxProjectionIterations(uint32_t iterations) override;
			virtual uint32_t GetMaxProjectionIterations() const override;

			virtual void SetSeparationTolerance(float tolerance) override;
			virtual float GetSeparationTolerance() const override;

			virtual void SetInternalDriveIterations(uint32_t iterations) override;
			virtual uint32_t GetInternalDriveIterations() const override;

			virtual void SetExternalDriveIterations(uint32_t iterations) override;
			virtual uint32_t GetExternalDriveIterations() const override;

		public:
			void Remove(physx::PxScene* pScene, bool isEnableWakeOnLostTouch = true);

		public:
			physx::PxArticulation* GetInterface() const { return m_pArticulation; }

		private:
			physx::PxArticulation* m_pArticulation{ nullptr };
			bool m_isValid{ false };
		};

		class ArticulationReducedCoordinate : public IArticulationReducedCoordinate
		{
		public:
			ArticulationReducedCoordinate(physx::PxArticulationReducedCoordinate* pArticulationReducedCoordinate);
			virtual ~ArticulationReducedCoordinate();

		public:
			DeclArticulationBase;

		public:
			virtual void SetArticulationFlags(Flags flags) override;
			virtual void SetArticulationFlag(Flag flag, bool isEnable) override;

			virtual Flags GetArticulationFlags() const override;

			virtual uint32_t GetDofs() const override;

			virtual void AddLoopJoint(IJoint* pJoint) override;
			virtual void RemoveLoopJoint(IJoint* pJoint) override;
			virtual uint32_t GetNumLoopJoints() const override;

			virtual uint32_t GetLoopJoints(IJoint** ppUserBuffer, uint32_t bufferSize, uint32_t startIndex = 0) const override;
			virtual uint32_t GetCoefficentMatrixSize() const override;

			virtual void TeleportRootLink(const Transform& pose, bool isEnableAutoWake) override;

		public:
			void Remove(physx::PxScene* pScene, bool isEnableWakeOnLostTouch = true);

		public:
			physx::PxArticulationReducedCoordinate* GetInterface() const { return m_pArticulationReducedCoordinate; }

		private:
			physx::PxArticulationReducedCoordinate* m_pArticulationReducedCoordinate{ nullptr };

			std::vector<IJoint*> m_joints;

			bool m_isValid{ false };
		};

#define DeclArticulationJointBase												\
		virtual IArticulationLink* GetParentArticulationLink() const override;	\
																				\
		virtual void SetParentPose(const Transform& pose) override;				\
		virtual Transform GetParentPose() const override;						\
																				\
		virtual IArticulationLink* GetChildArticulationLink() const override;	\
																				\
		virtual void SetChildPose(const Transform& pose) override;				\
		virtual Transform GetChildPose() const override;

		class ArticulationJoint : public IArticulationJoint
		{
		public:
			ArticulationJoint(physx::PxArticulationJoint* pArticulationJoint);
			virtual ~ArticulationJoint();

		public:
			DeclArticulationJointBase;

		public:
			virtual void SetTargetOrientation(const math::Quaternion& orientation) override;
			virtual math::Quaternion GetTargetOrientation() const override;

			virtual void SetTargetVelocity(const math::float3& velocity) override;
			virtual math::float3 GetTargetVelocity() const override;

			virtual void SetDriveType(DriveType emDriveType) override;
			virtual DriveType GetDriveType() const override;

			virtual void SetStiffness(float spring) override;
			virtual float GetStiffness() const override;

			virtual void SetDamping(float damping) override;
			virtual float GetDamping() const override;

			virtual void SetInternalCompliance(float compliance) override;
			virtual float GetInternalCompliance() const override;

			virtual void SetExternalCompliance(float compliance) override;
			virtual float GetExternalCompliance() const override;

			virtual void SetSwingLimit(float zLimit, float yLimit) override;
			virtual void GetSwingLimit(float& zLimit, float& yLimit) const override;

			virtual void SetTangentialStiffness(float spring) override;
			virtual float GetTangentialStiffness() const override;

			virtual void SetTangentialDamping(float damping) override;
			virtual float GetTangentialDamping() const override;

			virtual void SetSwingLimitContactDistance(float contactDistance) override;
			virtual float GetSwingLimitContactDistance() const override;

			virtual void SetSwingLimitEnabled(bool isEnabled) override;
			virtual bool GetSwingLimitEnabled() const override;

			virtual void SetTwistLimit(float lower, float upper) override;
			virtual void GetTwistLimit(float& lower, float& upper) const override;

			virtual void SetTwistLimitEnabled(bool isEnabled) override;
			virtual bool GetTwistLimitEnabled() const override;

			virtual void SetTwistLimitContactDistance(float contactDistance) override;
			virtual float GetTwistLimitContactDistance() const override;

		private:
			physx::PxArticulationJoint* m_pArticulationJoint{ nullptr };
		};

		class ArticulationJointReducedCoordinate : public IArticulationJointReducedCoordinate
		{
		public:
			ArticulationJointReducedCoordinate(physx::PxArticulationJointReducedCoordinate* pArticulationJoint);
			virtual ~ArticulationJointReducedCoordinate();

		public:
			DeclArticulationJointBase;

		public:
			virtual	void SetJointType(Type emJointType) override;
			virtual	Type GetJointType() const override;

			virtual	void SetMotion(Axis emAxis, Motion emMotion) override;
			virtual	Motion GetMotion(Axis emAxis) const override;

			virtual void SetLimit(Axis emAxis, const float lowLimit, const float highLimit) override;
			virtual void GetLimit(Axis emAxis, float& lowLimit, float& highLimit) override;
			virtual void SetDrive(Axis emAxis, const float stiffness, const float damping, const float maxForce, bool isAccelerationDrive = false) override;
			virtual void GetDrive(Axis emAxis, float& stiffness, float& damping, float& maxForce, bool& isAcceleration) override;
			virtual void SetDriveTarget(Axis emAxis, const float target) override;
			virtual void SetDriveVelocity(Axis emAxis, const float targetVel) override;
			virtual float GetDriveTarget(Axis emAxis) override;
			virtual float GetDriveVelocity(Axis emAxis) override;

			virtual	void SetFrictionCoefficient(const float coefficient) override;
			virtual	float GetFrictionCoefficient() const override;

			virtual void SetMaxJointVelocity(const float maxJointV) override;
			virtual float GetMaxJointVelocity() const override;

		private:
			physx::PxArticulationJointReducedCoordinate* m_pArticulationJointReducedCoordinate{ nullptr };
		};
	}
}