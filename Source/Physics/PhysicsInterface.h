#pragma once

#include "PhysicsDefine.h"

namespace est
{
	namespace physics
	{
		class IRigidActor;
		class IArticulationLink;
		class IAggregate;
		class IJoint;

		struct IGeometry
		{
			enum Type
			{
				eSphere = 0,
				ePlane,
				eCapsule,
				eBox,
				eConvexMesh,
				eTriangleMesh,
				eHeightField,

				TypeCount,
				eInvalid = TypeCount,
			};

			enum MeshFlag
			{
				eNone = 0,
				eDoubleSided = (1 << 1),
			};
			using MeshFlags = EnumFlags<MeshFlag, uint8_t>;

			virtual ~IGeometry() = default;
			virtual IGeometry::Type GetType() const = 0;
		};
		FLAGS_OPERATORS(IGeometry::MeshFlag, uint8_t);

		struct SphereGeometry : public IGeometry
		{
			float radius{ 0.f };

			virtual IGeometry::Type GetType() const { return IGeometry::Type::eSphere; }
		};

		struct PlaneGeometry : public IGeometry
		{
			math::Plane plane;

			virtual IGeometry::Type GetType() const { return IGeometry::Type::ePlane; }
		};

		struct CapsuleGeometry : public IGeometry
		{
			float radius{ 0.f };
			float halfHeight{ 0.f };

			virtual IGeometry::Type GetType() const { return IGeometry::Type::eCapsule; }
		};

		struct BoxGeometry : public IGeometry
		{
			math::float3 halfExtents{ 0.f };

			virtual IGeometry::Type GetType() const { return IGeometry::Type::eBox; }
		};

		struct ConvexMeshGeometry : public IGeometry
		{
			enum Flag
			{
				eNone = 0,
				eTightBounds = (1 << 0),
			};
			using Flags = EnumFlags<Flag, uint8_t>;

			math::float3 scale{ math::float3::One };
			math::Quaternion rotation;

			Flags flags{ Flag::eNone };

			uint32_t numVertices{ 0 };
			const math::float3* pVertices{ nullptr };

			uint32_t numIndices{ 0 };
			const uint32_t* pIndices{ nullptr };

			virtual IGeometry::Type GetType() const { return IGeometry::Type::eConvexMesh; }
		};
		FLAGS_OPERATORS(ConvexMeshGeometry::Flag, uint8_t);

		struct TriangleMeshGeometry : public IGeometry
		{
			math::float3 scale{ math::float3::One };
			math::Quaternion rotation;

			MeshFlags meshFlags{ MeshFlag::eNone };

			uint32_t numVertices{ 0 };
			const math::float3* pVertices{ nullptr };

			uint32_t numIndices{ 0 };
			const uint32_t* pIndices{ nullptr };

			virtual IGeometry::Type GetType() const { return IGeometry::Type::eTriangleMesh; }
		};

		struct HeightFieldGeometry : public IGeometry
		{
			enum Flag
			{
				eNone = 0,
				eNoBoundaryEdges = (1 << 0),
			};
			using Flags = EnumFlags<Flag, uint16_t>;

			uint32_t numRows{ 0 };
			uint32_t numColumns{ 0 };

			const float* pHeightFieldPoints{ nullptr };
			float thickness{ -1.f };
			float convexEdgeThreshold{ 0.f };

			Flags flags{ Flag::eNone };

			float heightScale{ 1.f };
			float rowScale{ 1.f };
			float columnScale{ 1.f };

			MeshFlags meshFlags{ MeshFlag::eNone };

			virtual IGeometry::Type GetType() const { return IGeometry::Type::eHeightField; }
		};
		FLAGS_OPERATORS(HeightFieldGeometry::Flag, uint16_t);

		class IMaterial
		{
		public:
			IMaterial() = default;
			virtual ~IMaterial() = default;

		public:
			enum Flag
			{
				eDisableFriction = 1 << 0,
				eDisableStrongFriction = 1 << 1,
			};
			using Flags = EnumFlags<Flag, uint16_t>;

			enum CombineMode
			{
				eAverage = 0,	//!< Average: (a + b)/2
				eMin = 1,		//!< Minimum: minimum(a,b)
				eMulitply = 2,	//!< Multiply: a*b
				eMax = 3,		//!< Maximum: maximum(a,b)
				eN_Values = 4,	//!< This is not a valid combine mode, it is a sentinel to denote the number of possible values. We assert that the variable's value is smaller than this.
				ePAD_32 = 0x7fffffff //!< This is not a valid combine mode, it is to assure that the size of the enum type is big enough.
			};

		public:
			virtual void SetDynamicFriction(float coef) = 0;
			virtual float GetDynamicFriction() const = 0;

			virtual void SetStaticFriction(float coef) = 0;
			virtual float GetStaticFriction() const = 0;

			virtual void SetRestitution(float rest) = 0;
			virtual float GetRestitution() const = 0;

			virtual void SetFlag(Flag flag, bool isEnable) = 0;
			virtual void SetFlags(Flags flags) = 0;

			virtual Flags GetFlags() const = 0;

			virtual void SetFrictionCombineMode(CombineMode combMode) = 0;
			virtual CombineMode GetFrictionCombineMode() const = 0;

			virtual void SetRestitutionCombineMode(CombineMode combMode) = 0;
			virtual CombineMode GetRestitutionCombineMode() const = 0;
		};
		FLAGS_OPERATORS(IMaterial::Flag, uint16_t);

		class IShape
		{
		public:
			IShape() = default;
			virtual ~IShape() = default;

		public:
			enum Flag
			{
				eNone = 0,
				eSimulationShape = (1 << 0), // The shape will partake in collision in the physical simulation.
				eSceneQueryShape = (1 << 1), // The shape will partake in scene queries (ray casts, overlap tests, sweeps, ...).
				eTriggerShape = (1 << 2), // The shape is a trigger which can send reports whenever other shapes enter/leave its volume.
				eVisualization = (1 << 3), // Enable debug renderer for this shape
			};
			using Flags = EnumFlags<Flag, uint8_t>;

		public:
			virtual IGeometry::Type GetType() const = 0;

			virtual void SetName(const string::StringID& name) = 0;
			virtual const string::StringID& GetName() const = 0;

		public:
			virtual void SetGeometry(const IGeometry* pGeometry) = 0;

			virtual void SetLocalPose(const Transform& pose) = 0;
			virtual Transform GetLocalPose() = 0;

			virtual void SetSimulationFilterData(const FilterData& filterData) = 0;
			virtual FilterData GetSimulationFilterData() const = 0;

			virtual void SetQueryFilterData(const FilterData& filterData) = 0;
			virtual FilterData GetQueryFilterData() const = 0;

			virtual void SetContactOffset(float contactOffset) = 0;
			virtual float GetContactOffset() const = 0;

			virtual void SetRestOffset(float restOffset) = 0;
			virtual float GetRestOffset() const = 0;

			virtual void SetFlag(Flag flag, bool isEnable) = 0;
			virtual void SetFlags(Flags flags) = 0;
			virtual Flags GetFlags() const = 0;

			virtual bool IsExclusive() const = 0;

		public:
			virtual uint32_t GetNumMaterials() const = 0;
			virtual std::shared_ptr<IMaterial> GetMaterial(uint32_t index) const = 0;
		};
		FLAGS_OPERATORS(IShape::Flag, uint8_t);

		class IArticulationBase
		{
		public:
			IArticulationBase() = default;
			virtual ~IArticulationBase() = default;

		public:
			enum Type
			{
				eReducedCoordinate = 0,
				eMaximumCoordinate = 1
			};

		public:
			virtual Type GetType() const = 0;

			virtual void SetSolverIterationCounts(uint32_t minPositionIters, uint32_t minVelocityIters = 1) = 0;
			virtual void GetSolverIterationCounts(uint32_t& minPositionIters, uint32_t& minVelocityIters) const = 0;

			virtual bool IsSleeping() const = 0;
			virtual void SetSleepThreshold(float threshold) = 0;
			virtual float GetSleepThreshold() const = 0;

			virtual void SetStabilizationThreshold(float threshold) = 0;
			virtual float GetStabilizationThreshold() const = 0;

			virtual void SetWakeCounter(float wakeCounterValue) = 0;
			virtual float GetWakeCounter() const = 0;

			virtual void WakeUp() = 0;
			virtual void PutToSleep() = 0;

			virtual IArticulationLink* CreateLink(IArticulationLink* pParent, const Transform& pose) = 0;
			virtual uint32_t GetNumLinks() const = 0;

			virtual uint32_t GetLinks(IArticulationLink** ppUserBuffer, uint32_t bufferSize, uint32_t startIndex = 0) const = 0;

			virtual void SetName(const string::StringID& name) = 0;
			virtual const string::StringID& GetName() const = 0;

			virtual Bounds GetWorldBounds(float inflation = 1.01f) const = 0;

			virtual IAggregate* GetAggregate() const = 0;
		};

		class IArticulation : public IArticulationBase
		{
		public:
			IArticulation() = default;
			virtual ~IArticulation() = default;

		public:
			virtual Type GetType() const { return Type::eMaximumCoordinate; }

		public:
			virtual void SetMaxProjectionIterations(uint32_t iterations) = 0;
			virtual uint32_t GetMaxProjectionIterations() const = 0;

			virtual void SetSeparationTolerance(float tolerance) = 0;
			virtual float GetSeparationTolerance() const = 0;

			virtual void SetInternalDriveIterations(uint32_t iterations) = 0;
			virtual uint32_t GetInternalDriveIterations() const = 0;

			virtual void SetExternalDriveIterations(uint32_t iterations) = 0;
			virtual uint32_t GetExternalDriveIterations() const = 0;
		};

		class IArticulationReducedCoordinate : public IArticulationBase
		{
		public:
			IArticulationReducedCoordinate() = default;
			virtual ~IArticulationReducedCoordinate() = default;

		public:
			enum Flag
			{
				eFixBase = (1 << 1),
			};
			using Flags = EnumFlags<Flag, uint8_t>;

		public:
			virtual Type GetType() const { return Type::eReducedCoordinate; }

		public:
			virtual void SetArticulationFlags(Flags flags) = 0;
			virtual void SetArticulationFlag(Flag flag, bool isEnable) = 0;

			virtual Flags GetArticulationFlags() const = 0;

			virtual uint32_t GetDofs() const = 0;

			virtual void AddLoopJoint(IJoint* pJoint) = 0;
			virtual void RemoveLoopJoint(IJoint* pJoint) = 0;
			virtual uint32_t GetNumLoopJoints() const = 0;

			virtual uint32_t GetLoopJoints(IJoint** ppUserBuffer, uint32_t bufferSize, uint32_t startIndex = 0) const = 0;
			virtual uint32_t GetCoefficentMatrixSize() const = 0;

			virtual void TeleportRootLink(const Transform& pose, bool isEnableAutoWake) = 0;
		};
		FLAGS_OPERATORS(IArticulationReducedCoordinate::Flag, uint8_t);

		class IArticulationJointBase
		{
		public:
			IArticulationJointBase() = default;
			virtual ~IArticulationJointBase() = default;

		public:
			virtual IArticulationLink* GetParentArticulationLink() const = 0;

			virtual void SetParentPose(const Transform& pose) = 0;
			virtual Transform GetParentPose() const = 0;

			virtual IArticulationLink* GetChildArticulationLink() const = 0;

			virtual void SetChildPose(const Transform& pose) = 0;
			virtual Transform GetChildPose() const = 0;
		};

		class IArticulationJoint : public IArticulationJointBase
		{
		public:
			IArticulationJoint() = default;
			virtual ~IArticulationJoint() = default;

		public:
			enum DriveType
			{
				eTarget = 0,
				eError = 1,
			};

		public:
			virtual void SetTargetOrientation(const math::Quaternion& orientation) = 0;
			virtual math::Quaternion GetTargetOrientation() const = 0;

			virtual void SetTargetVelocity(const math::float3& velocity) = 0;
			virtual math::float3 GetTargetVelocity() const = 0;

			virtual void SetDriveType(DriveType emDriveType) = 0;
			virtual DriveType GetDriveType() const = 0;

			virtual void SetStiffness(float spring) = 0;
			virtual float GetStiffness() const = 0;

			virtual void SetDamping(float damping) = 0;
			virtual float GetDamping() const = 0;

			virtual void SetInternalCompliance(float compliance) = 0;
			virtual float GetInternalCompliance() const = 0;

			virtual void SetExternalCompliance(float compliance) = 0;
			virtual float GetExternalCompliance() const = 0;

			virtual void SetSwingLimit(float zLimit, float yLimit) = 0;
			virtual void GetSwingLimit(float& zLimit, float& yLimit) const = 0;

			virtual void SetTangentialStiffness(float spring) = 0;
			virtual float GetTangentialStiffness() const = 0;

			virtual void SetTangentialDamping(float damping) = 0;
			virtual float GetTangentialDamping() const = 0;

			virtual void SetSwingLimitContactDistance(float contactDistance) = 0;
			virtual float GetSwingLimitContactDistance() const = 0;

			virtual void SetSwingLimitEnabled(bool isEnabled) = 0;
			virtual bool GetSwingLimitEnabled() const = 0;

			virtual void SetTwistLimit(float lower, float upper) = 0;
			virtual void GetTwistLimit(float& lower, float& upper) const = 0;

			virtual void SetTwistLimitEnabled(bool isEnabled) = 0;
			virtual bool GetTwistLimitEnabled() const = 0;

			virtual void SetTwistLimitContactDistance(float contactDistance) = 0;
			virtual float GetTwistLimitContactDistance() const = 0;
		};

		class IArticulationJointReducedCoordinate : public IArticulationJointBase
		{
		public:
			IArticulationJointReducedCoordinate() = default;
			virtual ~IArticulationJointReducedCoordinate() = default;

		public:
			enum Axis
			{
				eTwist = 0,
				eSwing1 = 1,
				eSwing2 = 2,
				eX = 3,
				eY = 4,
				eZ = 5,
			};

			enum Motion
			{
				eLocked = 0,
				eLimited = 1,
				eFree = 2,
			};

			enum Type
			{
				ePrismatic = 0,
				eRevolute = 1,
				eSpherical = 2,
				eFix = 3,
				eUndefined = 4,
			};

		public:
			virtual void SetJointType(Type emJointType) = 0;
			virtual Type GetJointType() const = 0;

			virtual void SetMotion(Axis emAxis, Motion emMotion) = 0;
			virtual Motion GetMotion(Axis emAxis) const = 0;

			virtual void SetLimit(Axis emAxis, const float lowLimit, const float highLimit) = 0;
			virtual void GetLimit(Axis emAxis, float& lowLimit, float& highLimit) = 0;
			virtual void SetDrive(Axis emAxis, const float stiffness, const float damping, const float maxForce, bool isAccelerationDrive = false) = 0;
			virtual void GetDrive(Axis emAxis, float& stiffness, float& damping, float& maxForce, bool& isAcceleration) = 0;
			virtual void SetDriveTarget(Axis emAxis, const float target) = 0;
			virtual void SetDriveVelocity(Axis emAxis, const float targetVel) = 0;
			virtual float GetDriveTarget(Axis emAxis) = 0;
			virtual float GetDriveVelocity(Axis emAxis) = 0;

			virtual void SetFrictionCoefficient(const float coefficient) = 0;
			virtual float GetFrictionCoefficient() const = 0;

			virtual void SetMaxJointVelocity(const float maxJointV) = 0;
			virtual float GetMaxJointVelocity() const = 0;
		};

		class IActor
		{
		public:
			IActor() = default;
			virtual ~IActor() = default;

		public:
			enum Type
			{
				eRigidStatic = 0,
				eRigidDynamic,
				eArticulationLink,

				TypeCount,
			};

			enum ActorFlag
			{
				eNone = 0,
				eVisualization = (1 << 0),
				eDisableGravity = (1 << 1),
				eSendSleepNotifies = (1 << 2),
				eDisableSimulation = (1 << 3),
			};
			using ActorFlags = EnumFlags<ActorFlag, uint8_t>;

			struct DominanceGroup
			{
				union
				{
					struct
					{
						uint8_t group : 5;
						uint8_t padding : 3;
					};
					uint8_t value;
				};

				DominanceGroup(uint8_t _group)
					: value(_group)
				{
				}

				DominanceGroup& operator = (uint8_t _group)
				{
					value = _group;
					return *this;
				}
				operator uint8_t() const { return group; }
			};

		public:
			virtual Type GetType() const = 0;

		public:
			virtual void SetName(const string::StringID& name) = 0;
			virtual const string::StringID& GetName() const = 0;

			virtual void SetUserData(void* pUserData) = 0;
			virtual void* GetUserData() const = 0;

			virtual Bounds GetWorldBounds(float inflation = 1.01f) const = 0;

			virtual void SetActorFlag(ActorFlag flag, bool isEnable) = 0;
			virtual void SetActorFlags(ActorFlags flags) = 0;
			virtual ActorFlags GetActorFlags() const = 0;

			virtual void SetDominanceGroup(DominanceGroup dominanceGroup) = 0;
			virtual DominanceGroup GetDominanceGroup() const = 0;
		};
		FLAGS_OPERATORS(IActor::ActorFlags, uint8_t);

		class IRigidActor : public IActor
		{
		public:
			IRigidActor() = default;
			virtual ~IRigidActor() = default;

		public:
			virtual void SetGlobalTransform(const Transform& pose, bool isEnableAutoWake = true) = 0;
			virtual Transform GetGlobalTransform() const = 0;

			virtual void AttachShape(const std::shared_ptr<IShape>& pShape) = 0;
			virtual void DetachShape(const std::shared_ptr<IShape>& pShape, bool isEnableWakeOnLostTouch = true) = 0;
			virtual uint32_t GetNumShapes() const = 0;
			virtual uint32_t GetShapes(std::shared_ptr<IShape>* ppUserBuffer, uint32_t bufferSize, uint32_t startIndex = 0) const = 0;
			virtual std::shared_ptr<IShape> GetShape(uint32_t index) const = 0;
		};

		class IRigidStatic : public IRigidActor
		{
		public:
			IRigidStatic() = default;
			virtual ~IRigidStatic() = default;

		public:
			virtual Type GetType() const override { return Type::eRigidStatic; }
		};

		class IRigidBody : public IRigidActor
		{
		public:
			IRigidBody() = default;
			virtual ~IRigidBody() = default;

		public:
			enum Flag
			{
				eNone = 0,
				eKinematic = (1 << 0), // Enables kinematic mode for the actor.
				eUseKinematicTargetForSceneQueries = (1 << 1), // Use the kinematic target transform for scene queries.
				eEnableCCD = (1 << 2), // Enables swept integration for the actor.
				eEnableCCD_Friction = (1 << 3), // Enabled CCD in swept integration for the actor.
				eEnablePoseIntegrationPreview = (1 << 4), // Register a rigid body for reporting pose changes by the simulation at an early stage.
				eEnableSpeculativeCCD = (1 << 5), // Register a rigid body to dynamicly adjust contact offset based on velocity. This can be used to achieve a CCD effect.
				eEnableCCD_MaxContactImpulse = (1 << 6), // Permit CCD to limit maxContactImpulse. This is useful for use-cases like a destruction system but can cause visual artefacts so is not enabled by default.
			};
			using Flags = EnumFlags<Flag, uint8_t>;

		public:
			virtual void SetCenterMassLocalPose(const Transform& pose) = 0;
			virtual Transform GetCenterMassLocalPose() = 0;

			virtual void SetMass(float mass) = 0;
			virtual float GetMass() const = 0;
			virtual float GetInvMass() const = 0;

			virtual void SetMassSpaceInertiaTensor(const math::float3& m) = 0;
			virtual math::float3 GetMassSpaceInertiaTensor() const = 0;
			virtual math::float3 GetMassSpaceInvInertiaTensor() const = 0;

			virtual void SetLinearVelocity(const math::float3& linearVelocity, bool isEnableAutoWake = true) = 0;
			virtual math::float3 GetLinearVelocity() const = 0;

			virtual void SetAngularVelocity(const math::float3& angularVelocity, bool isEnableAutoWake = true) = 0;
			virtual math::float3 GetAngularVelocity() const = 0;

			virtual void AddForce(const math::float3& force, ForceMode mode = ForceMode::eForce, bool isEnableAutoWake = true) = 0;
			virtual void ClearForce(ForceMode mode = ForceMode::eForce) = 0;

			virtual void AddTorque(const math::float3& torque, ForceMode mode = ForceMode::eForce, bool isEnableAutoWake = true) = 0;
			virtual void ClearTorque(ForceMode mode = ForceMode::eForce) = 0;

			virtual void SetFlag(Flag flag, bool isEnable) = 0;
			virtual void SetFlags(Flags flags) = 0;
			virtual Flags GetFlags() const = 0;

			virtual void SetMinCCDAdvanceCoefficient(float advanceCoefficient) = 0;
			virtual float GetMinCCDAdvanceCoefficient() const = 0;

			virtual void SetMaxDepenetrationVelocity(float biasClamp) = 0;
			virtual float GetMaxDepenetrationVelocity() const = 0;

			virtual void SetMaxContactImpulse(float maxImpulse) = 0;
			virtual float GetMaxContactImpulse() const = 0;
		};
		FLAGS_OPERATORS(IRigidBody::Flag, uint8_t);

		class IRigidDynamic : public IRigidBody
		{
		public:
			IRigidDynamic() = default;
			virtual ~IRigidDynamic() = default;

		public:
			enum LockFlag
			{
				eNone = 0,
				eLockLinear_X = (1 << 0),
				eLockLinear_Y = (1 << 1),
				eLockLinear_Z = (1 << 2),
				eLockAngular_X = (1 << 3),
				eLockAngular_Y = (1 << 4),
				eLockAngular_Z = (1 << 5)
			};
			using LockFlags = EnumFlags<LockFlag, uint16_t>;

		public:
			virtual Type GetType() const override { return Type::eRigidDynamic; }

		public:
			virtual void SetKinematicTarget(const Transform& destination) = 0;
			virtual bool GetKinematicTarget(Transform& target_out) = 0;

			virtual void SetLinearDamping(float linearDamp) = 0;
			virtual float GetLinearDamping() const = 0;

			virtual void SetAngularDamping(float angularDamp) = 0;
			virtual float GetAngularDamping() const = 0;

			virtual void SetMaxAngularVelocity(float maxAngularVelocity) = 0;
			virtual float GetMaxAngularVelocity() const = 0;

			virtual bool IsSleeping() const = 0;
			virtual void SetSleepThreshold(float threshold) = 0;
			virtual float GetSleepThreshold() const = 0;

			virtual void SetStabilizationThreshold(float threshold) = 0;
			virtual float GetStabilizationThreshold() const = 0;

			virtual void SetLockFlag(LockFlag flag, bool isEnable) = 0;
			virtual void SetLockFlags(LockFlags flags) = 0;

			virtual void SetWakeCounter(float wakeCounterValue) = 0;
			virtual float GetWakeCounter() const = 0;
			virtual void WakeUp() = 0;
			virtual void PutToSleep() = 0;

			virtual void SetSolverIterationCounts(uint32_t minPositionIters, uint32_t minVelocityIters = 1) = 0;
			virtual void GetSolverIterationCounts(uint32_t& minPositionIters, uint32_t& minVelocityIters) const = 0;

			virtual void SetContactReportThreshold(float threshold) = 0;
			virtual float GetContactReportThreshold() const = 0;
		};
		FLAGS_OPERATORS(IRigidDynamic::LockFlag, uint16_t);

		class IArticulationLink : public IRigidBody
		{
		public:
			IArticulationLink() = default;
			virtual ~IArticulationLink() = default;

		public:
			virtual Type GetType() const override { return Type::eArticulationLink; }

		public:
			virtual IArticulation* GetArticulation() const = 0;
			virtual IArticulationJointBase* GetInboundJoint() = 0;

			virtual uint32_t GetNumChildren() const = 0;
			virtual uint32_t GetChildren(IArticulationLink** ppUserBuffer, uint32_t bufferSize, uint32_t startIndex = 0) const = 0;
		};

		class IConstraint
		{
		public:
			IConstraint() = default;
			virtual ~IConstraint() = default;

		public:
			enum Flag
			{
				eBorken = 1 << 0, //!< whether the constraint is broken
				eProjectToActor0 = 1 << 1, //!< whether actor1 should get projected to actor0 for this constraint (note: projection of a static/kinematic actor to a dynamic actor will be ignored)
				eProjectToActor1 = 1 << 2, //!< whether actor0 should get projected to actor1 for this constraint (note: projection of a static/kinematic actor to a dynamic actor will be ignored)
				eProjection = eProjectToActor0 | eProjectToActor1, //!< whether the actors should get projected for this constraint (the direction will be chosen by PhysX)
				eCollisionEnabled = 1 << 3, //!< whether contacts should be generated between the objects this constraint constrains
				eVisualization = 1 << 4, //!< whether this constraint should be visualized, if constraint visualization is turned on
				eDriveLimitsAreForces = 1 << 5, //!< limits for drive strength are forces rather than impulses
				eImprovedSlerp = 1 << 7, //!< perform preprocessing for improved accuracy on D6 Slerp Drive (this flag will be removed in a future release when preprocessing is no longer required)
				eDisablePreprocessing = 1 << 8, //!< suppress constraint preprocessing, intended for use with rowResponseThreshold. May result in worse solver accuracy for ill-conditioned constraints.
				eEnableExtendedLimits = 1 << 9, //!< enables extended limit ranges for angular limits (e.g. limit values > PxPi or < -PxPi)
				eGpuCompatible = 1 << 10 //!< the constraint type is supported by gpu dynamic
			};
			using Flags = EnumFlags<Flag, uint16_t>;

		public:
			virtual void GetActors(IRigidActor*& pActor0, IRigidActor*& pActor1) const = 0;
			virtual void SetActors(IRigidActor* pActor0, IRigidActor* pActor1) = 0;

			virtual void SetFlags(Flags flags) = 0;
			virtual void SetFlag(Flag flag, bool isEnable) = 0;
			virtual Flags GetFlags() const = 0;

			virtual void GetForce(math::float3& linear, math::float3& angular) const = 0;

			virtual bool IsValid() const = 0;

			virtual void SetBreakForce(float linear, float angular) = 0;
			virtual void GetBreakForce(float& linear, float& angular) const = 0;

			virtual void SetMinResponseThreshold(float threshold) = 0;
			virtual float GetMinResponseThreshold() const = 0;
		};
		FLAGS_OPERATORS(IConstraint::Flag, uint16_t);

		class IJoint
		{
		public:
			IJoint() = default;
			virtual ~IJoint() = default;

		public:
			enum ActorIndex
			{
				eActor0 = 0,
				eActor1,
			};

		public:
			virtual void SetActors(IRigidActor* pActor0, IRigidActor* pActor1) = 0;
			virtual void GetActors(IRigidActor*& pActor0, IRigidActor*& pActor1) const = 0;

			virtual void SetLocalPose(ActorIndex emActor, const Transform& localPose) = 0;
			virtual Transform GetLocalPose(ActorIndex emActor) const = 0;

			virtual Transform GetRelativeTransform() const = 0;
			virtual math::float3 GetRelativeLinearVelocity() const = 0;

			virtual math::float3 GetRelativeAngularVelocity() const = 0;
			virtual void SetBreakForce(float force, float torque) = 0;

			virtual void GetBreakForce(float& force, float& torque) const = 0;
			virtual void SetConstraintFlags(IConstraint::Flags flags) = 0;

			virtual void SetConstraintFlag(IConstraint::Flag flag, bool isEnable) = 0;
			virtual IConstraint::Flags GetConstraintFlags() const = 0;

			virtual void SetInvMassScale0(float invMassScale) = 0;
			virtual float GetInvMassScale0() const = 0;

			virtual void SetInvInertiaScale0(float invInertiaScale) = 0;
			virtual float GetInvInertiaScale0() const = 0;

			virtual void SetInvMassScale1(float invMassScale) = 0;
			virtual float GetInvMassScale1() const = 0;

			virtual void SetInvInertiaScale1(float invInertiaScale) = 0;
			virtual float GetInvInertiaScale1() const = 0;

			virtual IConstraint* GetConstraint() = 0;

			virtual void SetName(const string::StringID& name) = 0;
			virtual const string::StringID& GetName() const = 0;
		};

		class IContactJoint : public IJoint
		{
		public:
			IContactJoint() = default;
			virtual ~IContactJoint() = default;

		public:
			virtual void SetContact(const math::float3& contact) = 0;
			virtual void SetContactNormal(const math::float3& contactNormal) = 0;
			virtual void SetPenetration(const float penetration) = 0;

			virtual math::float3 GetContact() const = 0;
			virtual math::float3 GetContactNormal() const = 0;
			virtual float GetPenetration() const = 0;

			virtual float GetResititution() const = 0;
			virtual void SetResititution(float resititution) = 0;
			virtual float GetBounceThreshold() const = 0;
			virtual void SetBounceThreshold(float bounceThreshold) = 0;
		};

		class ID6Joint : public IJoint
		{
		public:
			ID6Joint() = default;
			virtual ~ID6Joint() = default;

		public:
			enum Axis
			{
				eAxis_X = 0,
				eAxis_Y = 1,
				eAxis_Z = 2,
				eAxis_Twist = 3,
				eAxis_Swing1 = 4,
				eAxis_Swing2 = 5,
				AxisCount = 6,
			};

			enum Motion
			{
				eLocked,	//!< The DOF is locked, it does not allow relative motion.
				eLimited,	//!< The DOF is limited, it only allows motion within a specific range.
				eFree,		//!< The DOF is free and has its full range of motion.
			};

			enum Drive
			{
				eDrive_X = 0,
				eDrive_Y = 1,
				eDrive_Z = 2,
				eDrive_Swing = 3,
				eDrive_Twist = 4,
				eDrive_Slerp = 5,
				DriveCount = 6,
			};

			enum DriveFlag
			{
				eNone = 0,
				eAcceleration = 1,
			};
			using DriveFlags = EnumFlags<DriveFlag, uint32_t>;

			struct JointDrive : public Spring
			{
				float forceLimit{ std::numeric_limits<float>::max() };
				DriveFlags flags{ DriveFlag::eNone };

				JointDrive(float driveStiffness, float driveDamping, float driveForceLimit, bool isAcceleration = false)
					: Spring(driveStiffness, driveDamping)
					, forceLimit(driveForceLimit)
					, flags(isAcceleration == true ? DriveFlag::eAcceleration : DriveFlag::eNone)
				{
				}
			};

		public:
			virtual void SetMotion(Axis emAxis, Motion emMotionType) = 0;
			virtual Motion GetMotion(Axis emAxis) const = 0;

			virtual float GetTwistAngle() const = 0;

			virtual float GetSwingYAngle() const = 0;
			virtual float GetSwingZAngle() const = 0;

			virtual void SetDistanceLimit(const JointLinearLimit& limit) = 0;
			virtual JointLinearLimit GetDistanceLimit() const = 0;

			virtual void SetLinearLimit(const JointLinearLimit& limit) = 0;
			virtual JointLinearLimit GetLinearLimit() const = 0;

			virtual void SetTwistLimit(const JointAngularLimitPair& limit) = 0;
			virtual JointAngularLimitPair GetTwistLimit() const = 0;

			virtual void SetSwingLimit(const JointLimitCone& limit) = 0;
			virtual JointLimitCone GetSwingLimit() const = 0;

			virtual void SetPyramidSwingLimit(const JointLimitPyramid& limit) = 0;
			virtual JointLimitPyramid GetPyramidSwingLimit() const = 0;

			virtual void SetDrive(Drive emDriveIndex, const JointDrive& drive) = 0;
			virtual JointDrive GetDrive(Drive emDriveIndex) const = 0;

			virtual void SetDrivePosition(const Transform& pose, bool isEnableAutoWake = true) = 0;
			virtual Transform GetDrivePosition() const = 0;

			virtual void SetDriveVelocity(const math::float3& linear, const math::float3& angular, bool isEnableAutoWake = true) = 0;
			virtual void GetDriveVelocity(math::float3& linear, math::float3& angular) const = 0;

			virtual void SetProjectionLinearTolerance(float tolerance) = 0;
			virtual float GetProjectionLinearTolerance() const = 0;

			virtual void SetProjectionAngularTolerance(float tolerance) = 0;
			virtual float GetProjectionAngularTolerance() const = 0;
		};
		FLAGS_OPERATORS(ID6Joint::DriveFlag, uint32_t);

		class IDistanceJoint : public IJoint
		{
		public:
			IDistanceJoint() = default;
			virtual ~IDistanceJoint() = default;

		public:
			enum Flag
			{
				eMaxDistanceEnabled = 1 << 1,
				eMinDistanceEnabled = 1 << 2,
				eSpringEnabled = 1 << 3
			};
			using Flags = EnumFlags<Flag, uint16_t>;

		public:
			virtual float GetDistance() const = 0;

			virtual void SetMinDistance(float distance) = 0;
			virtual float GetMinDistance() const = 0;

			virtual void SetMaxDistance(float distance) = 0;
			virtual float GetMaxDistance() const = 0;

			virtual void SetTolerance(float tolerance) = 0;
			virtual float GetTolerance() const = 0;

			virtual void SetStiffness(float stiffness) = 0;
			virtual float GetStiffness() const = 0;

			virtual void SetDamping(float damping) = 0;
			virtual float GetDamping() const = 0;

			virtual void SetDistanceJointFlags(Flags flags) = 0;
			virtual void SetDistanceJointFlags(Flag flag, bool isEnable) = 0;
			virtual Flags GetDistanceJointFlags() const = 0;
		};
		FLAGS_OPERATORS(IDistanceJoint::Flag, uint16_t);

		class IFixedJoint : public IJoint
		{
		public:
			IFixedJoint() = default;
			virtual ~IFixedJoint() = default;

		public:
			virtual void SetProjectionLinearTolerance(float tolerance) = 0;
			virtual float GetProjectionLinearTolerance() const = 0;

			virtual void SetProjectionAngularTolerance(float tolerance) = 0;
			virtual float GetProjectionAngularTolerance() const = 0;
		};

		class IPrismaticJoint : public IJoint
		{
		public:
			IPrismaticJoint() = default;
			virtual ~IPrismaticJoint() = default;

		public:
			enum Flag
			{
				eNone = 0,
				eLimitEnabled = 1 << 1,
			};
			using Flags = EnumFlags<Flag, uint16_t>;

		public:
			virtual float GetPosition() const = 0;
			virtual float GetVelocity() const = 0;

			virtual void SetLimit(const JointLinearLimitPair& limit) = 0;
			virtual JointLinearLimitPair GetLimit() const = 0;

			virtual void SetPrismaticJointFlags(Flags flags) = 0;
			virtual void SetPrismaticJointFlag(Flag flag, bool isEnable) = 0;
			virtual Flags GetPrismaticJointFlags() const = 0;

			virtual void SetProjectionLinearTolerance(float tolerance) = 0;
			virtual float GetProjectionLinearTolerance() const = 0;

			virtual void SetProjectionAngularTolerance(float tolerance) = 0;
			virtual float GetProjectionAngularTolerance() const = 0;
		};
		FLAGS_OPERATORS(IPrismaticJoint::Flag, uint16_t);

		class IRevoluteJoint : public IJoint
		{
		public:
			IRevoluteJoint() = default;
			virtual ~IRevoluteJoint() = default;

		public:
			enum Flag
			{
				eNone = 0,
				eLimitEnabled = 1 << 0,
				eDriveEnabled = 1 << 1,
				eDriveFreeSpin = 1 << 2,
			};
			using Flags = EnumFlags<Flag, uint16_t>;

		public:
			virtual float GetAngle() const = 0;
			virtual float GetVelocity() const = 0;

			virtual void SetLimit(const JointAngularLimitPair& limit) = 0;
			virtual JointAngularLimitPair GetLimit() const = 0;

			virtual void SetDriveVelocity(float velocity, bool isEnableAutoWake = true) = 0;
			virtual float GetDriveVelocity() const = 0;

			virtual void SetDriveForceLimit(float limit) = 0;
			virtual float GetDriveForceLimit() const = 0;

			virtual void SetDriveGearRatio(float ratio) = 0;
			virtual float GetDriveGearRatio() const = 0;

			virtual void SetRevoluteJointFlags(Flags flags) = 0;
			virtual void SetRevoluteJointFlag(Flag flag, bool isEnable) = 0;
			virtual Flags GetRevoluteJointFlags() const = 0;

			virtual void SetProjectionLinearTolerance(float tolerance) = 0;
			virtual float GetProjectionLinearTolerance() const = 0;

			virtual void SetProjectionAngularTolerance(float tolerance) = 0;
			virtual float GetProjectionAngularTolerance() const = 0;
		};
		FLAGS_OPERATORS(IRevoluteJoint::Flag, uint16_t);

		class ISphericalJoint : public IJoint
		{
		public:
			ISphericalJoint() = default;
			virtual ~ISphericalJoint() = default;

		public:
			enum Flag
			{
				eNone = 0,
				eLimitEnabled = 1 << 1,
			};
			using Flags = EnumFlags<Flag, uint16_t>;

		public:
			virtual JointLimitCone GetLimitCone() const = 0;
			virtual void SetLimitCone(const JointLimitCone& limit) = 0;

			virtual float GetSwingYAngle() const = 0;
			virtual float GetSwingZAngle() const = 0;

			virtual void SetSphericalJointFlags(Flags flags) = 0;
			virtual void SetSphericalJointFlag(Flag flag, bool isEnable) = 0;
			virtual Flags GetSphericalJointFlags() const = 0;

			virtual void SetProjectionLinearTolerance(float tolerance) = 0;
			virtual float GetProjectionLinearTolerance() const = 0;
		};
		FLAGS_OPERATORS(ISphericalJoint::Flag, uint16_t);

		class IBVHStructure
		{
		public:
			IBVHStructure() = default;
			virtual ~IBVHStructure() = default;

		public:
			virtual uint32_t Raycast(const math::float3& origin, const math::float3& unitDir, float maxDist, uint32_t maxHits, uint32_t* __restrict rayHits) const = 0;
			virtual uint32_t Sweep(const Bounds& aabb, const math::float3& unitDir, float maxDist, uint32_t maxHits, uint32_t* __restrict sweepHits) const = 0;
			virtual uint32_t Overlap(const Bounds& aabb, uint32_t maxHits, uint32_t* __restrict overlapHits) const = 0;

			virtual const Bounds* GetBounds() const = 0;
			virtual uint32_t GetNumBounds() const = 0;
		};

		class IAggregate
		{
		public:
			IAggregate() = default;
			virtual ~IAggregate() = default;

		public:
			virtual bool AddActor(IRigidActor* pActor, const IBVHStructure* IBVHStructure = nullptr) = 0;
			virtual bool RemoveActor(IRigidActor* pActor) = 0;

			virtual bool AddArticulation(IArticulationBase* pArticulation) = 0;
			virtual bool RemoveArticulation(IArticulationBase* pArticulation) = 0;

			virtual uint32_t GetNumActors() const = 0;
			virtual uint32_t GetMaxNumActors() const = 0;

			virtual uint32_t GetActors(IRigidActor** ppUserBuffer, uint32_t bufferSize, uint32_t startIndex = 0) const = 0;

			virtual bool GetSelfCollision() const = 0;
		};

		struct RigidActorProperty
		{
			struct Material
			{
				float dynamicFriction{ 0.f };	// 0.f ~ FLOAT_MAX
				float staticFriction{ 0.f };	// 0.f ~ FLOAT_MAX
				float restitution{ 0.f };		// 0.f ~ 1.f
				IMaterial::Flags flags;
				IMaterial::CombineMode frictionCombineMode{ IMaterial::eAverage };
				IMaterial::CombineMode restitionCombineMode{ IMaterial::eAverage };
			};

			struct Shape
			{
				FilterData simulationFilterData;
				FilterData queryFilterData;
				Transform localPose;
				float contactOffset{ 0.02f };
				float restOffset{ 0.f };
				IShape::Flags flags;
				std::unique_ptr<IGeometry> pGeometry;
				bool isExclusive{ true };

				void SetSphereGeometry(float radius);
				void SetPlaneGeometry(const math::Plane& plane);
				void SetCapsuleGeometry(float radius, float halfHeight);
				void SetBoxGeometry(const math::float3& halfExtents);
				void SetConvexMeshGeometry(const math::float3& scale, const math::Quaternion& rotation, const math::float3* pVertices, uint32_t numVertices, const uint32_t* pIndices, uint32_t numIndices, ConvexMeshGeometry::Flags geometryFlags);
				void SetTriangleMeshGeometry(const math::float3& scale, const math::Quaternion& rotation, const math::float3* pVertices, uint32_t numVertices, const uint32_t* pIndices, uint32_t numIndices, IGeometry::MeshFlags meshFlags);
				void SetHeightFieldGeometry(uint32_t numRows, uint32_t numColumns, const float* pHeightFieldPoints, float thickness, float convexEdgeThreshold, float heightScale, float rowScale, float columnScale, IGeometry::MeshFlag meshFlags, HeightFieldGeometry::Flags geometryFlags);
			};

			struct RigidActor
			{
				string::StringID name;

				IActor::Type type{ IActor::eRigidStatic };
				IActor::ActorFlags flags{ IActor::ActorFlag::eNone };
				IActor::DominanceGroup group{ 0 };

				Transform globalTransform;

				struct DynamicProperty
				{
					IRigidDynamic::Flags flags;
					IRigidDynamic::LockFlags lockFlags;

					Transform centerMassLocalPose;
					float mass{ 1.f };
					math::float3 massSpaceInertiaTensor{ math::float3::One };

					float linearDamping{ 0.f };
					float angularDamping{ 0.5f };
					float maxAngularVelocity{ 100.f };
					float sleepThreshold{ 5e-5f };
					float stabilizationThreshold{ 1e-5f };
					float contactReportThreshold{ std::numeric_limits<float>::max() };
				};
				DynamicProperty dynamicProperty;
			};

			Material material;
			Shape shape;
			RigidActor rigidAcotr;

			RigidActorProperty& operator=(const RigidActorProperty& source);
		};
		
		enum QueryHitType
		{
			eNone = 0,
			eTouch = 1,
			eBlock = 2,
		};

		struct QueryFilterData
		{
			enum Flag
			{
				eStatic = (1 << 0),	//!< Traverse static shapes
				eDynamic = (1 << 1),	//!< Traverse dynamic shapes
				ePrefilter = (1 << 2),	//!< Run the pre-intersection-test filter (see #PxQueryFilterCallback::preFilter())
				ePostFilter = (1 << 3),	//!< Run the post-intersection-test filter (see #PxQueryFilterCallback::postFilter())
				eAnyHit = (1 << 4),	//!< Abort traversal as soon as any hit is found and return it via callback.block.
										//!< Helps query performance. Both eTOUCH and eBLOCK hitTypes are considered hits with this flag.
				eNoBlock = (1 << 5),	//!< All hits are reported as touching. Overrides eBLOCK returned from user filters with eTOUCH.
										//!< This is also an optimization hint that may improve query performance.
				eReserved = (1 << 15)	//!< Reserved for internal use
			};
			using Flags = EnumFlags<Flag, uint16_t>;

			FilterData filterData;
			Flags flags{ Flag::eStatic | Flag::eDynamic };

			QueryFilterData() = default;
			QueryFilterData(const FilterData& filterData, Flags flags)
				: filterData(filterData)
				, flags(flags)
			{
			}

			QueryFilterData(Flags flags)
				: flags(flags)
			{
			}
		};
		FLAGS_OPERATORS(QueryFilterData::Flag, uint16_t);

		struct QueryCache
		{
			IShape* pShape{ nullptr };
			IRigidActor* pActor{ nullptr };
		};

		using HitActorShape = QueryCache;

		std::shared_ptr<IMaterial> CreateMaterial(float staticFriction, float dynamicFriction, float restitution);
		std::shared_ptr<IShape> CreateShape(const IGeometry* pGeometry, const std::shared_ptr<IMaterial>& pMaterial, bool isExclusive, IShape::Flags shapeFlags);

		std::unique_ptr<IRigidActor> CreateRigidActor(const RigidActorProperty& rigidActorProperty);
		std::unique_ptr<IRigidStatic> CreateRigidStatic(const Transform& transform);
		std::unique_ptr<IRigidDynamic> CreateRigidDynamic(const Transform& transform);

		std::unique_ptr<IArticulation> CreateArticulation();
		std::unique_ptr<IArticulationReducedCoordinate>	CreateArticulationReducedCoordinate();
		std::unique_ptr<IBVHStructure> CreateBVHStructure(Bounds* pBounds, uint32_t numBounds);
		std::unique_ptr<IAggregate> CreateAggregate(uint32_t maxSize, bool isSelfCollision);

		namespace scene
		{
			void AddActor(IRigidActor* pRigidActor);
			void RemoveActor(std::unique_ptr<IRigidActor> pRigidActor, bool isEnableWakeOnLostTouch = true);

			void AddArticulation(IArticulation* pArticulation);
			void RemoveArticulation(std::unique_ptr<IArticulation> pArticulation, bool isEnableWakeOnLostTouch = true);

			void AddAggregate(IAggregate* pAggregate);
			void RemoveAggregate(std::unique_ptr<IAggregate> pAggregate, bool isEnableWakeOnLostTouch = true);

			bool Raycast(const math::float3& origin, const math::float3& unitDir, const float distance, HitLocation* pHitLocation_out = nullptr, HitActorShape* pHitActorShape_out = nullptr, const QueryFilterData& queryFilterData = {}, const QueryCache& queryCache = {});
			bool Raycast(const math::float3& origin, const math::float3& unitDir, const float distance, const IShape* pShape, const Transform& pose, HitLocation* pHitLocation_out = nullptr);
		}
	}
}