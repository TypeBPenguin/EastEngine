#include "stdafx.h"
#include "PhysicsInterface.h"

#include "PhysicsUtil.h"
#include "PhysicsMaterial.h"
#include "PhysicsShape.h"
#include "Aggregate.h"
#include "Articulation.h"
#include "RigidBody.h"
#include "BVHStructure.h"

namespace est
{
	namespace physics
	{
		static_assert(IGeometry::eSphere == physx::PxGeometryType::eSPHERE, "mismatch enum");
		static_assert(IGeometry::ePlane == physx::PxGeometryType::ePLANE, "mismatch enum");
		static_assert(IGeometry::eCapsule == physx::PxGeometryType::eCAPSULE, "mismatch enum");
		static_assert(IGeometry::eBox == physx::PxGeometryType::eBOX, "mismatch enum");
		static_assert(IGeometry::eConvexMesh == physx::PxGeometryType::eCONVEXMESH, "mismatch enum");
		static_assert(IGeometry::eTriangleMesh == physx::PxGeometryType::eTRIANGLEMESH, "mismatch enum");
		static_assert(IGeometry::eHeightField == physx::PxGeometryType::eHEIGHTFIELD, "mismatch enum");

		static_assert(IGeometry::eDoubleSided == physx::PxMeshGeometryFlag::eDOUBLE_SIDED, "mismatch enum");

		static_assert(ConvexMeshGeometry::eTightBounds == physx::PxConvexMeshGeometryFlag::eTIGHT_BOUNDS, "mismatch enum");
		static_assert(TriangleMeshGeometry::eDoubleSided == physx::PxMeshGeometryFlag::eDOUBLE_SIDED, "mismatch enum");

		static_assert(IMaterial::Flag::eDisableFriction == physx::PxMaterialFlag::eDISABLE_FRICTION, "mismatch enum");
		static_assert(IMaterial::Flag::eDisableStrongFriction == physx::PxMaterialFlag::eDISABLE_STRONG_FRICTION, "mismatch enum");

		static_assert(IMaterial::CombineMode::eAverage == physx::PxCombineMode::eAVERAGE, "mismatch enum");
		static_assert(IMaterial::CombineMode::eMin == physx::PxCombineMode::eMIN, "mismatch enum");
		static_assert(IMaterial::CombineMode::eMulitply == physx::PxCombineMode::eMULTIPLY, "mismatch enum");
		static_assert(IMaterial::CombineMode::eMax == physx::PxCombineMode::eMAX, "mismatch enum");
		static_assert(IMaterial::CombineMode::eN_Values == physx::PxCombineMode::eN_VALUES, "mismatch enum");
		static_assert(IMaterial::CombineMode::ePAD_32 == physx::PxCombineMode::ePAD_32, "mismatch enum");

		static_assert(IShape::Flag::eSimulationShape == physx::PxShapeFlag::eSIMULATION_SHAPE, "mismatch enum");
		static_assert(IShape::Flag::eSceneQueryShape == physx::PxShapeFlag::eSCENE_QUERY_SHAPE, "mismatch enum");
		static_assert(IShape::Flag::eTriggerShape == physx::PxShapeFlag::eTRIGGER_SHAPE, "mismatch enum");
		static_assert(IShape::Flag::eVisualization == physx::PxShapeFlag::eVISUALIZATION, "mismatch enum");

		static_assert(IArticulationBase::Type::eReducedCoordinate == physx::PxArticulationBase::eReducedCoordinate, "mismatch enum");
		static_assert(IArticulationBase::Type::eMaximumCoordinate == physx::PxArticulationBase::eMaximumCoordinate, "mismatch enum");

		static_assert(IArticulationReducedCoordinate::Flag::eFixBase == physx::PxArticulationFlag::eFIX_BASE, "mismatch enum");

		static_assert(IArticulationJoint::eTarget == physx::PxArticulationJointDriveType::eTARGET, "mismatch enum");
		static_assert(IArticulationJoint::eError == physx::PxArticulationJointDriveType::eERROR, "mismatch enum");

		static_assert(IArticulationJointReducedCoordinate::eTwist == physx::PxArticulationAxis::eTWIST, "mismatch enum");
		static_assert(IArticulationJointReducedCoordinate::eSwing1 == physx::PxArticulationAxis::eSWING1, "mismatch enum");
		static_assert(IArticulationJointReducedCoordinate::eSwing2 == physx::PxArticulationAxis::eSWING2, "mismatch enum");
		static_assert(IArticulationJointReducedCoordinate::eX == physx::PxArticulationAxis::eX, "mismatch enum");
		static_assert(IArticulationJointReducedCoordinate::eY == physx::PxArticulationAxis::eY, "mismatch enum");
		static_assert(IArticulationJointReducedCoordinate::eZ == physx::PxArticulationAxis::eZ, "mismatch enum");

		static_assert(IArticulationJointReducedCoordinate::eLocked == physx::PxArticulationMotion::eLOCKED, "mismatch enum");
		static_assert(IArticulationJointReducedCoordinate::eLimited == physx::PxArticulationMotion::eLIMITED, "mismatch enum");
		static_assert(IArticulationJointReducedCoordinate::eFree == physx::PxArticulationMotion::eFREE, "mismatch enum");

		static_assert(IArticulationJointReducedCoordinate::ePrismatic == physx::PxArticulationJointType::ePRISMATIC, "mismatch enum");
		static_assert(IArticulationJointReducedCoordinate::eRevolute == physx::PxArticulationJointType::eREVOLUTE, "mismatch enum");
		static_assert(IArticulationJointReducedCoordinate::eSpherical == physx::PxArticulationJointType::eSPHERICAL, "mismatch enum");
		static_assert(IArticulationJointReducedCoordinate::eFix == physx::PxArticulationJointType::eFIX, "mismatch enum");
		static_assert(IArticulationJointReducedCoordinate::eUndefined == physx::PxArticulationJointType::eUNDEFINED, "mismatch enum");

		static_assert(IActor::Type::eRigidStatic == physx::PxActorType::eRIGID_STATIC, "mismatch enum");
		static_assert(IActor::Type::eRigidDynamic == physx::PxActorType::eRIGID_DYNAMIC, "mismatch enum");
		static_assert(IActor::Type::eArticulationLink == physx::PxActorType::eARTICULATION_LINK, "mismatch enum");

		static_assert(IActor::ActorFlag::eVisualization == physx::PxActorFlag::eVISUALIZATION, "mismatch enum");
		static_assert(IActor::ActorFlag::eDisableGravity == physx::PxActorFlag::eDISABLE_GRAVITY, "mismatch enum");
		static_assert(IActor::ActorFlag::eSendSleepNotifies == physx::PxActorFlag::eSEND_SLEEP_NOTIFIES, "mismatch enum");
		static_assert(IActor::ActorFlag::eDisableSimulation == physx::PxActorFlag::eDISABLE_SIMULATION, "mismatch enum");

		static_assert(IRigidBody::Flag::eKinematic == physx::PxRigidBodyFlag::eKINEMATIC, "mismatch enum");
		static_assert(IRigidBody::Flag::eUseKinematicTargetForSceneQueries == physx::PxRigidBodyFlag::eUSE_KINEMATIC_TARGET_FOR_SCENE_QUERIES, "mismatch enum");
		static_assert(IRigidBody::Flag::eEnableCCD == physx::PxRigidBodyFlag::eENABLE_CCD, "mismatch enum");
		static_assert(IRigidBody::Flag::eEnableCCD_Friction == physx::PxRigidBodyFlag::eENABLE_CCD_FRICTION, "mismatch enum");
		static_assert(IRigidBody::Flag::eEnablePoseIntegrationPreview == physx::PxRigidBodyFlag::eENABLE_POSE_INTEGRATION_PREVIEW, "mismatch enum");
		static_assert(IRigidBody::Flag::eEnableSpeculativeCCD == physx::PxRigidBodyFlag::eENABLE_SPECULATIVE_CCD, "mismatch enum");
		static_assert(IRigidBody::Flag::eEnableCCD_MaxContactImpulse == physx::PxRigidBodyFlag::eENABLE_CCD_MAX_CONTACT_IMPULSE, "mismatch enum");

		static_assert(IRigidDynamic::LockFlag::eLockLinear_X == physx::PxRigidDynamicLockFlag::eLOCK_LINEAR_X, "mismatch enum");
		static_assert(IRigidDynamic::LockFlag::eLockLinear_Y == physx::PxRigidDynamicLockFlag::eLOCK_LINEAR_Y, "mismatch enum");
		static_assert(IRigidDynamic::LockFlag::eLockLinear_Z == physx::PxRigidDynamicLockFlag::eLOCK_LINEAR_Z, "mismatch enum");
		static_assert(IRigidDynamic::LockFlag::eLockAngular_X == physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_X, "mismatch enum");
		static_assert(IRigidDynamic::LockFlag::eLockAngular_Y == physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_Y, "mismatch enum");
		static_assert(IRigidDynamic::LockFlag::eLockAngular_Z == physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_Z, "mismatch enum");

		static_assert(IConstraint::Flag::eBorken == physx::PxConstraintFlag::eBROKEN, "mismatch enum");
		static_assert(IConstraint::Flag::eProjectToActor0 == physx::PxConstraintFlag::ePROJECT_TO_ACTOR0, "mismatch enum");
		static_assert(IConstraint::Flag::eProjectToActor1 == physx::PxConstraintFlag::ePROJECT_TO_ACTOR1, "mismatch enum");
		static_assert(IConstraint::Flag::eProjection == physx::PxConstraintFlag::ePROJECTION, "mismatch enum");
		static_assert(IConstraint::Flag::eCollisionEnabled == physx::PxConstraintFlag::eCOLLISION_ENABLED, "mismatch enum");
		static_assert(IConstraint::Flag::eVisualization == physx::PxConstraintFlag::eVISUALIZATION, "mismatch enum");
		static_assert(IConstraint::Flag::eDriveLimitsAreForces == physx::PxConstraintFlag::eDRIVE_LIMITS_ARE_FORCES, "mismatch enum");
		static_assert(IConstraint::Flag::eImprovedSlerp == physx::PxConstraintFlag::eIMPROVED_SLERP, "mismatch enum");
		static_assert(IConstraint::Flag::eDisablePreprocessing == physx::PxConstraintFlag::eDISABLE_PREPROCESSING, "mismatch enum");
		static_assert(IConstraint::Flag::eEnableExtendedLimits == physx::PxConstraintFlag::eENABLE_EXTENDED_LIMITS, "mismatch enum");
		static_assert(IConstraint::Flag::eGpuCompatible == physx::PxConstraintFlag::eGPU_COMPATIBLE, "mismatch enum");

		static_assert(IJoint::ActorIndex::eActor0 == physx::PxJointActorIndex::eACTOR0, "mismatch enum");
		static_assert(IJoint::ActorIndex::eActor1 == physx::PxJointActorIndex::eACTOR1, "mismatch enum");

		static_assert(ID6Joint::Axis::eAxis_X == physx::PxD6Axis::eX, "mismatch enum");
		static_assert(ID6Joint::Axis::eAxis_Y == physx::PxD6Axis::eY, "mismatch enum");
		static_assert(ID6Joint::Axis::eAxis_Z == physx::PxD6Axis::eZ, "mismatch enum");
		static_assert(ID6Joint::Axis::eAxis_Twist == physx::PxD6Axis::eTWIST, "mismatch enum");
		static_assert(ID6Joint::Axis::eAxis_Swing1 == physx::PxD6Axis::eSWING1, "mismatch enum");
		static_assert(ID6Joint::Axis::eAxis_Swing2 == physx::PxD6Axis::eSWING2, "mismatch enum");

		static_assert(ID6Joint::Motion::eLocked == physx::PxD6Motion::eLOCKED, "mismatch enum");
		static_assert(ID6Joint::Motion::eLimited == physx::PxD6Motion::eLIMITED, "mismatch enum");
		static_assert(ID6Joint::Motion::eFree == physx::PxD6Motion::eFREE, "mismatch enum");

		static_assert(ID6Joint::Drive::eDrive_X == physx::PxD6Drive::eX, "mismatch enum");
		static_assert(ID6Joint::Drive::eDrive_Y == physx::PxD6Drive::eY, "mismatch enum");
		static_assert(ID6Joint::Drive::eDrive_Z == physx::PxD6Drive::eZ, "mismatch enum");
		static_assert(ID6Joint::Drive::eDrive_Swing == physx::PxD6Drive::eSWING, "mismatch enum");
		static_assert(ID6Joint::Drive::eDrive_Twist == physx::PxD6Drive::eTWIST, "mismatch enum");
		static_assert(ID6Joint::Drive::eDrive_Slerp == physx::PxD6Drive::eSLERP, "mismatch enum");

		static_assert(ID6Joint::DriveFlag::eAcceleration == physx::PxD6JointDriveFlag::eACCELERATION, "mismatch enum");

		static_assert(IDistanceJoint::Flag::eMaxDistanceEnabled == physx::PxDistanceJointFlag::eMAX_DISTANCE_ENABLED, "mismatch enum");
		static_assert(IDistanceJoint::Flag::eMinDistanceEnabled == physx::PxDistanceJointFlag::eMIN_DISTANCE_ENABLED, "mismatch enum");
		static_assert(IDistanceJoint::Flag::eSpringEnabled == physx::PxDistanceJointFlag::eSPRING_ENABLED, "mismatch enum");

		static_assert(IPrismaticJoint::Flag::eLimitEnabled == physx::PxPrismaticJointFlag::eLIMIT_ENABLED, "mismatch enum");

		static_assert(IRevoluteJoint::Flag::eLimitEnabled == physx::PxRevoluteJointFlag::eLIMIT_ENABLED, "mismatch enum");
		static_assert(IRevoluteJoint::Flag::eDriveEnabled == physx::PxRevoluteJointFlag::eDRIVE_ENABLED, "mismatch enum");
		static_assert(IRevoluteJoint::Flag::eDriveFreeSpin == physx::PxRevoluteJointFlag::eDRIVE_FREESPIN, "mismatch enum");

		static_assert(ISphericalJoint::Flag::eLimitEnabled == physx::PxSphericalJointFlag::eLIMIT_ENABLED, "mismatch enum");

		static_assert(QueryFilterData::eStatic == physx::PxQueryFlag::eSTATIC, "mismatch enum");
		static_assert(QueryFilterData::eDynamic == physx::PxQueryFlag::eDYNAMIC, "mismatch enum");
		static_assert(QueryFilterData::ePrefilter == physx::PxQueryFlag::ePREFILTER, "mismatch enum");
		static_assert(QueryFilterData::ePostFilter == physx::PxQueryFlag::ePOSTFILTER, "mismatch enum");
		static_assert(QueryFilterData::eAnyHit == physx::PxQueryFlag::eANY_HIT, "mismatch enum");
		static_assert(QueryFilterData::eNoBlock == physx::PxQueryFlag::eNO_BLOCK, "mismatch enum");
		static_assert(QueryFilterData::eReserved == physx::PxQueryFlag::eRESERVED, "mismatch enum");

		static_assert(QueryHitType::eNone == physx::PxQueryHitType::eNONE, "mismatch enum");
		static_assert(QueryHitType::eTouch == physx::PxQueryHitType::eTOUCH, "mismatch enum");
		static_assert(QueryHitType::eBlock == physx::PxQueryHitType::eBLOCK, "mismatch enum");

		RigidActorProperty& RigidActorProperty::operator=(const RigidActorProperty& source)
		{
			material = source.material;

			shape.simulationFilterData = source.shape.simulationFilterData;
			shape.queryFilterData = source.shape.queryFilterData;
			shape.localPose = source.shape.localPose;
			shape.contactOffset = source.shape.contactOffset;
			shape.restOffset = source.shape.restOffset;
			shape.flags = source.shape.flags;
			shape.isExclusive = source.shape.isExclusive;
			if (source.shape.pGeometry != nullptr)
			{
				switch (source.shape.pGeometry->GetType())
				{
				case IGeometry::eSphere:
				{
					const SphereGeometry* pSphereGeometry = static_cast<const SphereGeometry*>(source.shape.pGeometry.get());
					shape.SetSphereGeometry(pSphereGeometry->radius);
				}
				break;
				case IGeometry::ePlane:
				{
					const PlaneGeometry* pPlaneGeometry = static_cast<const PlaneGeometry*>(source.shape.pGeometry.get());
					shape.SetPlaneGeometry(pPlaneGeometry->plane);
				}
				break;
				case IGeometry::eCapsule:
				{
					const CapsuleGeometry* pCapsuleGeometry = static_cast<const CapsuleGeometry*>(source.shape.pGeometry.get());
					shape.SetCapsuleGeometry(pCapsuleGeometry->radius, pCapsuleGeometry->halfHeight);
				}
				break;
				case IGeometry::eBox:
				{
					const BoxGeometry* pBoxGeometry = static_cast<const BoxGeometry*>(source.shape.pGeometry.get());
					shape.SetBoxGeometry(pBoxGeometry->halfExtents);
				}
				break;
				case IGeometry::eConvexMesh:
				{
					const ConvexMeshGeometry* pConvexMeshGeometry = static_cast<const ConvexMeshGeometry*>(source.shape.pGeometry.get());
					shape.SetConvexMeshGeometry(pConvexMeshGeometry->scale, pConvexMeshGeometry->rotation, pConvexMeshGeometry->pVertices, pConvexMeshGeometry->numVertices, pConvexMeshGeometry->pIndices, pConvexMeshGeometry->numIndices, pConvexMeshGeometry->flags);
				}
				break;
				case IGeometry::eTriangleMesh:
				{
					const TriangleMeshGeometry* pTriangleMeshGeometry = static_cast<const TriangleMeshGeometry*>(source.shape.pGeometry.get());
					shape.SetTriangleMeshGeometry(pTriangleMeshGeometry->scale, pTriangleMeshGeometry->rotation, pTriangleMeshGeometry->pVertices, pTriangleMeshGeometry->numVertices, pTriangleMeshGeometry->pIndices, pTriangleMeshGeometry->numIndices, pTriangleMeshGeometry->meshFlags);
				}
				break;
				case IGeometry::eHeightField:
				{
					const TriangleMeshGeometry* pTriangleMeshGeometry = static_cast<const TriangleMeshGeometry*>(source.shape.pGeometry.get());
					shape.SetTriangleMeshGeometry(pTriangleMeshGeometry->scale, pTriangleMeshGeometry->rotation, pTriangleMeshGeometry->pVertices, pTriangleMeshGeometry->numVertices, pTriangleMeshGeometry->pIndices, pTriangleMeshGeometry->numIndices, pTriangleMeshGeometry->meshFlags);
				}
				break;
				}
			}

			rigidAcotr = source.rigidAcotr;

			return *this;
		}

		void RigidActorProperty::Shape::SetSphereGeometry(float radius)
		{
			pGeometry = std::make_unique<SphereGeometry>();
			SphereGeometry* pSphereGeometry = static_cast<SphereGeometry*>(pGeometry.get());
			pSphereGeometry->radius = radius;
		}

		void RigidActorProperty::Shape::SetPlaneGeometry(const math::Plane& plane)
		{
			pGeometry = std::make_unique<PlaneGeometry>();
			PlaneGeometry* pPlaneGeometry = static_cast<PlaneGeometry*>(pGeometry.get());
			pPlaneGeometry->plane = plane;
		}

		void RigidActorProperty::Shape::SetCapsuleGeometry(float radius, float halfHeight)
		{
			pGeometry = std::make_unique<CapsuleGeometry>();
			CapsuleGeometry* pCapsuleGeometry = static_cast<CapsuleGeometry*>(pGeometry.get());
			pCapsuleGeometry->radius = radius;

			const math::float3 p0(0.f, halfHeight, 0.f);
			const math::float3 p1(0.f, -halfHeight, 0.f);
			const physx::PxTransform transform; physx::PxTransformFromSegment(Convert<const physx::PxVec3>(p0), Convert<const physx::PxVec3>(p1), &pCapsuleGeometry->halfHeight);
			localPose = Convert<const Transform>(transform);
		}

		void RigidActorProperty::Shape::SetBoxGeometry(const math::float3& halfExtents)
		{
			pGeometry = std::make_unique<BoxGeometry>();
			BoxGeometry* pBoxGeometry = static_cast<BoxGeometry*>(pGeometry.get());
			pBoxGeometry->halfExtents = halfExtents;
		}

		void RigidActorProperty::Shape::SetConvexMeshGeometry(const math::float3& scale, const math::Quaternion& rotation, const math::float3* pVertices, uint32_t numVertices, const uint32_t* pIndices, uint32_t numIndices, ConvexMeshGeometry::Flags geometryFlags)
		{
			pGeometry = std::make_unique<ConvexMeshGeometry>();
			ConvexMeshGeometry* pConvexMeshGeometry = static_cast<ConvexMeshGeometry*>(pGeometry.get());
			pConvexMeshGeometry->scale = scale;
			pConvexMeshGeometry->rotation = rotation;
			pConvexMeshGeometry->pVertices = pVertices;
			pConvexMeshGeometry->numVertices = numVertices;
			pConvexMeshGeometry->pIndices = pIndices;
			pConvexMeshGeometry->numIndices = numIndices;
			pConvexMeshGeometry->flags = geometryFlags;
		}

		void RigidActorProperty::Shape::SetTriangleMeshGeometry(const math::float3& scale, const math::Quaternion& rotation, const math::float3* pVertices, uint32_t numVertices, const uint32_t* pIndices, uint32_t numIndices, IGeometry::MeshFlags meshFlags)
		{
			pGeometry = std::make_unique<TriangleMeshGeometry>();
			TriangleMeshGeometry* pTriangleMeshGeometry = static_cast<TriangleMeshGeometry*>(pGeometry.get());
			pTriangleMeshGeometry->scale = scale;
			pTriangleMeshGeometry->rotation = rotation;
			pTriangleMeshGeometry->pVertices = pVertices;
			pTriangleMeshGeometry->numVertices = numVertices;
			pTriangleMeshGeometry->pIndices = pIndices;
			pTriangleMeshGeometry->numIndices = numIndices;
			pTriangleMeshGeometry->meshFlags = meshFlags;
		}

		void RigidActorProperty::Shape::SetHeightFieldGeometry(uint32_t numRows, uint32_t numColumns, const float* pHeightFieldPoints, float thickness, float convexEdgeThreshold, float heightScale, float rowScale, float columnScale, IGeometry::MeshFlag meshFlags, HeightFieldGeometry::Flags geometryFlags)
		{
			pGeometry = std::make_unique<HeightFieldGeometry>();
			HeightFieldGeometry* pHeightFieldGeometry = static_cast<HeightFieldGeometry*>(pGeometry.get());
			pHeightFieldGeometry->numRows = numRows;
			pHeightFieldGeometry->numColumns = numColumns;
			pHeightFieldGeometry->pHeightFieldPoints = pHeightFieldPoints;
			pHeightFieldGeometry->thickness = thickness;
			pHeightFieldGeometry->convexEdgeThreshold = convexEdgeThreshold;
			pHeightFieldGeometry->heightScale = heightScale;
			pHeightFieldGeometry->rowScale = rowScale;
			pHeightFieldGeometry->columnScale = columnScale;
			pHeightFieldGeometry->meshFlags = meshFlags;
			pHeightFieldGeometry->flags = geometryFlags;
		}

		std::shared_ptr<IMaterial> CreateMaterial(float staticFriction, float dynamicFriction, float restitution)
		{
			physx::PxMaterial* pPxMaterial = GetPhysics()->createMaterial(staticFriction, dynamicFriction, restitution);
			return std::make_unique<Material>(pPxMaterial);
		}

		std::shared_ptr<IShape> CreateShape(const IGeometry* pGeometry, const std::shared_ptr<IMaterial>& pMaterial, bool isExclusive, IShape::Flags shapeFlags)
		{
			physx::PxMaterial* pPxMaterials[] = 
			{
				util::GetInterface(pMaterial.get()),
			};

			physx::PxShape* pPxShape = nullptr;

			switch (pGeometry->GetType())
			{
			case IGeometry::eSphere:
			{
				const SphereGeometry* pSphereGeometry = static_cast<const SphereGeometry*>(pGeometry);
				const physx::PxSphereGeometry geometry(pSphereGeometry->radius);
				pPxShape = GetPhysics()->createShape(geometry, pPxMaterials, _countof(pPxMaterials), isExclusive, Convert<physx::PxShapeFlags>(shapeFlags));
			}
			break;
			case IGeometry::ePlane:
			{
				pPxShape = GetPhysics()->createShape(physx::PxPlaneGeometry(), pPxMaterials, _countof(pPxMaterials), isExclusive, Convert<physx::PxShapeFlags>(shapeFlags));
			}
			break;
			case IGeometry::eCapsule:
			{
				const CapsuleGeometry* pCapsuleGeometry = static_cast<const CapsuleGeometry*>(pGeometry);
				const physx::PxCapsuleGeometry geometry(pCapsuleGeometry->radius, pCapsuleGeometry->halfHeight);
				pPxShape = GetPhysics()->createShape(geometry, pPxMaterials, _countof(pPxMaterials), isExclusive, Convert<physx::PxShapeFlags>(shapeFlags));
			}
			break;
			case IGeometry::eBox:
			{
				const BoxGeometry* pBoxGeometry = static_cast<const BoxGeometry*>(pGeometry);
				const physx::PxBoxGeometry geometry(Convert<const physx::PxVec3>(pBoxGeometry->halfExtents));
				pPxShape = GetPhysics()->createShape(geometry, pPxMaterials, _countof(pPxMaterials), isExclusive, Convert<physx::PxShapeFlags>(shapeFlags));
			}
			break;
			case IGeometry::eConvexMesh:
			{
				const ConvexMeshGeometry* pConvexMeshGeometry = static_cast<const ConvexMeshGeometry*>(pGeometry);

				physx::PxConvexMesh* pConvexMesh = nullptr;
				if (pConvexMeshGeometry->pIndices == nullptr || pConvexMeshGeometry->numIndices == 0)
				{
					pConvexMesh = util::CreateConvexMesh(pConvexMeshGeometry->pVertices, pConvexMeshGeometry->numVertices);
				}
				else
				{
					pConvexMesh = util::CreateConvexMesh(pConvexMeshGeometry->pVertices, pConvexMeshGeometry->numVertices, pConvexMeshGeometry->pIndices, pConvexMeshGeometry->numIndices);
				}

				const physx::PxMeshScale meshScale(Convert<const physx::PxVec3>(pConvexMeshGeometry->scale), Convert<const physx::PxQuat>(pConvexMeshGeometry->rotation));
				const physx::PxConvexMeshGeometry geometry(pConvexMesh, meshScale, Convert<const physx::PxConvexMeshGeometryFlags>(pConvexMeshGeometry->flags));
				pPxShape = GetPhysics()->createShape(geometry, pPxMaterials, _countof(pPxMaterials), isExclusive, Convert<physx::PxShapeFlags>(shapeFlags));
			}
			break;
			case IGeometry::eTriangleMesh:
			{
				const TriangleMeshGeometry* pTriangleMeshGeometry = static_cast<const TriangleMeshGeometry*>(pGeometry);
				physx::PxTriangleMesh* pTriangleMesh = util::CreateTriangleMesh(pTriangleMeshGeometry->pVertices, pTriangleMeshGeometry->numVertices, pTriangleMeshGeometry->pIndices, pTriangleMeshGeometry->numIndices);

				const physx::PxMeshScale meshScale(Convert<const physx::PxVec3>(pTriangleMeshGeometry->scale), Convert<const physx::PxQuat>(pTriangleMeshGeometry->rotation));
				physx::PxTriangleMeshGeometry geometry(pTriangleMesh, meshScale, Convert<const physx::PxMeshGeometryFlags>(pTriangleMeshGeometry->meshFlags));
				pPxShape = GetPhysics()->createShape(geometry, pPxMaterials, _countof(pPxMaterials), isExclusive, Convert<physx::PxShapeFlags>(shapeFlags));
			}
			break;
			case IGeometry::eHeightField:
			{
				const HeightFieldGeometry* pHeightFieldGeometry = static_cast<const HeightFieldGeometry*>(pGeometry);
				physx::PxHeightField* pHeightField = util::CreateHeightField(pHeightFieldGeometry);
				physx::PxHeightFieldGeometry geometry(pHeightField, Convert<const physx::PxMeshGeometryFlags>(pHeightFieldGeometry->meshFlags), pHeightFieldGeometry->heightScale, pHeightFieldGeometry->rowScale, pHeightFieldGeometry->columnScale);
				pPxShape = GetPhysics()->createShape(geometry, pPxMaterials, _countof(pPxMaterials), isExclusive, Convert<physx::PxShapeFlags>(shapeFlags));
			}
			break;
			default:
				throw_line("unknown geometry type");
				return nullptr;
			}

			return std::make_unique<Shape>(pPxShape, &pMaterial, 1);
		}

		std::unique_ptr<IRigidActor> CreateRigidActor(const RigidActorProperty& rigidActorProperty)
		{
			std::shared_ptr<IMaterial> pMaterial = CreateMaterial(rigidActorProperty.material.staticFriction, rigidActorProperty.material.dynamicFriction, rigidActorProperty.material.restitution);
			pMaterial->SetFlags(rigidActorProperty.material.flags);
			pMaterial->SetFrictionCombineMode(rigidActorProperty.material.frictionCombineMode);
			pMaterial->SetRestitutionCombineMode(rigidActorProperty.material.restitionCombineMode);

			std::shared_ptr<IShape> pShape = CreateShape(rigidActorProperty.shape.pGeometry.get(), pMaterial, rigidActorProperty.shape.isExclusive, rigidActorProperty.shape.flags);
			pShape->SetSimulationFilterData(rigidActorProperty.shape.simulationFilterData);
			pShape->SetQueryFilterData(rigidActorProperty.shape.queryFilterData);
			pShape->SetLocalPose(rigidActorProperty.shape.localPose);
			pShape->SetContactOffset(rigidActorProperty.shape.contactOffset);
			pShape->SetRestOffset(rigidActorProperty.shape.restOffset);

			std::unique_ptr<IRigidActor> pRigidActor;
			switch (rigidActorProperty.rigidAcotr.type)
			{
			case IActor::Type::eRigidStatic:
				pRigidActor = CreateRigidStatic(rigidActorProperty.rigidAcotr.globalTransform);
				break;
			case IActor::Type::eRigidDynamic:
			{
				pRigidActor = CreateRigidDynamic(rigidActorProperty.rigidAcotr.globalTransform);

				const RigidActorProperty::RigidActor::DynamicProperty& dynamicProperty = rigidActorProperty.rigidAcotr.dynamicProperty;

				IRigidDynamic* pRigidDynamic = static_cast<IRigidDynamic*>(pRigidActor.get());
				pRigidDynamic->SetFlags(dynamicProperty.flags);
				pRigidDynamic->SetLockFlags(dynamicProperty.lockFlags);

				pRigidDynamic->SetCenterMassLocalPose(dynamicProperty.centerMassLocalPose);
				pRigidDynamic->SetMass(dynamicProperty.mass);
				pRigidDynamic->SetMassSpaceInertiaTensor(dynamicProperty.massSpaceInertiaTensor);

				pRigidDynamic->SetLinearDamping(dynamicProperty.linearDamping);
				pRigidDynamic->SetAngularDamping(dynamicProperty.angularDamping);
				pRigidDynamic->SetMaxAngularVelocity(dynamicProperty.maxAngularVelocity);
				pRigidDynamic->SetSleepThreshold(dynamicProperty.sleepThreshold);
				pRigidDynamic->SetStabilizationThreshold(dynamicProperty.stabilizationThreshold);
				pRigidDynamic->SetContactReportThreshold(dynamicProperty.contactReportThreshold);
			}
			break;
			default:
				throw_line("unknown type");
				break;
			}
			pRigidActor->SetActorFlags(rigidActorProperty.rigidAcotr.flags);
			pRigidActor->SetDominanceGroup(rigidActorProperty.rigidAcotr.group);

			pRigidActor->AttachShape(pShape);
			return pRigidActor;
		}

		std::unique_ptr<IRigidStatic> CreateRigidStatic(const Transform& transform)
		{
			physx::PxRigidStatic* pRigidStatic = GetPhysics()->createRigidStatic(Convert<const physx::PxTransform>(transform));
			return std::make_unique<RigidStatic>(pRigidStatic);
		}

		std::unique_ptr<IRigidDynamic> CreateRigidDynamic(const Transform& transform)
		{
			physx::PxRigidDynamic* pRigidDynamic = GetPhysics()->createRigidDynamic(Convert<const physx::PxTransform>(transform));
			return std::make_unique<RigidDynamic>(pRigidDynamic);
		}

		std::unique_ptr<IArticulation> CreateArticulation()
		{
			physx::PxArticulation* pPxArticulation = GetPhysics()->createArticulation();
			return std::make_unique<Articulation>(pPxArticulation);
		}

		std::unique_ptr<IArticulationReducedCoordinate>	CreateArticulationReducedCoordinate()
		{
			physx::PxArticulationReducedCoordinate* pPxArticulationReducedCoordinate = GetPhysics()->createArticulationReducedCoordinate();
			return std::make_unique<ArticulationReducedCoordinate>(pPxArticulationReducedCoordinate);
		}

		std::unique_ptr<IBVHStructure> CreateBVHStructure(Bounds* pBounds, uint32_t numBounds)
		{
			physx::PxPhysics* pPhysics = GetPhysics();
			physx::PxCooking* pCooking = GetCooking();

			physx::PxBVHStructureDesc bvhDesc;
			bvhDesc.bounds.count = numBounds;
			bvhDesc.bounds.data = pBounds;
			bvhDesc.bounds.stride = sizeof(physx::PxBounds3);
			physx::PxBVHStructure* pBVHStructure = pCooking->createBVHStructure(bvhDesc, pPhysics->getPhysicsInsertionCallback());
			return std::make_unique<BVHStructure>(pBVHStructure);
		}

		std::unique_ptr<IAggregate> CreateAggregate(uint32_t maxSize, bool isSelfCollision)
		{
			physx::PxAggregate* pPxAggregate = GetPhysics()->createAggregate(maxSize, isSelfCollision);
			return std::make_unique<Aggregate>(pPxAggregate);
		}

		namespace scene
		{
			void AddActor(IRigidActor* pRigidActor)
			{
				GetScene()->addActor(*util::GetInterface(pRigidActor));
			}

			void RemoveActor(std::unique_ptr<IRigidActor> pRigidActor, bool isEnableWakeOnLostTouch)
			{
				switch (pRigidActor->GetType())
				{
				case IRigidActor::eRigidStatic:
				{
					RigidStatic* pRigidStatic = static_cast<RigidStatic*>(pRigidActor.get());
					pRigidStatic->Remove(GetScene(), isEnableWakeOnLostTouch);
				}
				break;
				case IRigidActor::eRigidDynamic:
				{
					RigidDynamic* pRigidDynamic = static_cast<RigidDynamic*>(pRigidActor.get());
					pRigidDynamic->Remove(GetScene(), isEnableWakeOnLostTouch);
				}
				break;
				case IRigidActor::eArticulationLink:
				{
					ArticulationLink* pArticulationLink = static_cast<ArticulationLink*>(pRigidActor.get());
					pArticulationLink->Remove(GetScene(), isEnableWakeOnLostTouch);
				}
				break;
				default:
					throw_line("unknown type");
					break;
				}
			}

			void AddArticulation(IArticulation* pArticulation)
			{
				GetScene()->addArticulation(*util::GetInterface(pArticulation));
			}

			void RemoveArticulation(std::unique_ptr<IArticulation> pArticulation, bool isEnableWakeOnLostTouch)
			{
				Articulation* pOrigin = static_cast<Articulation*>(pArticulation.get());
				pOrigin->Remove(GetScene(), isEnableWakeOnLostTouch);
			}

			void AddAggregate(IAggregate* pAggregate)
			{
				Aggregate* pOrigin = static_cast<Aggregate*>(pAggregate);
				GetScene()->addAggregate(*pOrigin->GetInterface());
			}

			void RemoveAggregate(std::unique_ptr<IAggregate> pAggregate, bool isEnableWakeOnLostTouch)
			{
				Aggregate* pOrigin = static_cast<Aggregate*>(pAggregate.get());
				pOrigin->Remove(GetScene(), isEnableWakeOnLostTouch);
			}

			bool Raycast(const math::float3& origin, const math::float3& unitDir, const float distance, HitLocation* pHitLocation_out, HitActorShape* pHitActorShape_out, const QueryFilterData& queryFilterData, const QueryCache& queryCache)
			{
				const physx::PxHitFlags hitFlags = physx::PxHitFlag::ePOSITION | physx::PxHitFlag::eNORMAL;

				physx::PxQueryFilterData filterData;
				filterData.data = Convert<const physx::PxFilterData>(queryFilterData.filterData);
				filterData.flags = Convert<const physx::PxQueryFlags>(queryFilterData.flags);

				physx::PxQueryFilterCallback* pFilterCallback = nullptr;

				physx::PxQueryCache physxQueryCache;

				const bool isValidQueryCache = queryCache.pShape != nullptr && queryCache.pActor != nullptr;
				if (isValidQueryCache == true)
				{
					physxQueryCache.shape = util::GetInterface(queryCache.pShape);
					physxQueryCache.actor = util::GetInterface(queryCache.pActor);
				}

				physx::PxRaycastBuffer raycastBuffer;
				GetScene()->raycast(Convert<const physx::PxVec3>(origin), Convert<const physx::PxVec3>(unitDir), distance, raycastBuffer, hitFlags, filterData, pFilterCallback, isValidQueryCache == true ? &physxQueryCache : nullptr);

				if (raycastBuffer.hasBlock == true)
				{
					if (pHitLocation_out != nullptr)
					{
						pHitLocation_out->position = Convert<const math::float3>(raycastBuffer.block.position);
						pHitLocation_out->normal = Convert<const math::float3>(raycastBuffer.block.normal);
						pHitLocation_out->distance = raycastBuffer.block.distance;
					}

					if (pHitActorShape_out != nullptr)
					{
						pHitActorShape_out->pShape = static_cast<IShape*>(raycastBuffer.block.shape->userData);
						pHitActorShape_out->pActor = static_cast<IRigidActor*>(raycastBuffer.block.actor->userData);
					}
				}
				return raycastBuffer.hasBlock;
			}

			bool Raycast(const math::float3& origin, const math::float3& unitDir, const float distance, const IShape* pShape, const Transform& pose, HitLocation* pHitLocation_out)
			{
				const physx::PxHitFlags hitFlags = physx::PxHitFlag::ePOSITION | physx::PxHitFlag::eNORMAL;

				const physx::PxGeometry& geometry = util::GetInterface(pShape)->getGeometry().any();

				physx::PxRaycastHit raycastHit;

				const uint32_t hitCount = physx::PxGeometryQuery::raycast(Convert<const physx::PxVec3>(origin), Convert<const physx::PxVec3>(unitDir), geometry, Convert<const physx::PxTransform>(pose), distance, hitFlags, 1, &raycastHit);
				if (hitCount > 0)
				{
					if (pHitLocation_out != nullptr)
					{
						pHitLocation_out->position = Convert<const math::float3>(raycastHit.position);
						pHitLocation_out->normal = Convert<const math::float3>(raycastHit.normal);
						pHitLocation_out->distance = raycastHit.distance;
					}

					return true;
				}

				return false;
			}
		}
	}
}