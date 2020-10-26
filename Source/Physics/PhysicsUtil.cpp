#include "stdafx.h"
#include "PhysicsUtil.h"

#include "PhysicsInterface.h"
#include "RigidBody.h"
#include "Articulation.h"
#include "PhysicsShape.h"
#include "PhysicsMaterial.h"

namespace est
{
	namespace physics
	{
		namespace util
		{
			physx::PxRigidActor* GetInterface(const IRigidActor* pRigidActor)
			{
				if (pRigidActor == nullptr)
					return nullptr;

				switch (pRigidActor->GetType())
				{
				case IRigidActor::eRigidStatic:
				{
					const RigidStatic* pRigidStatic = static_cast<const RigidStatic*>(pRigidActor);
					return pRigidStatic->GetInterface();
				}
				break;
				case IRigidActor::eRigidDynamic:
				{
					const RigidDynamic* pRigidDynamic = static_cast<const RigidDynamic*>(pRigidActor);
					return pRigidDynamic->GetInterface();
				}
				break;
				case IRigidActor::eArticulationLink:
				{
					const ArticulationLink* pArticulationLink = static_cast<const ArticulationLink*>(pRigidActor);
					return pArticulationLink->GetInterface();
				}
				break;
				default:
					throw_line("unknown type");
					return nullptr;
				}
			}

			physx::PxArticulationBase* GetInterface(const IArticulationBase* pArticulationBase)
			{
				if (pArticulationBase == nullptr)
					return nullptr;

				switch (pArticulationBase->GetType())
				{
				case IArticulationBase::Type::eReducedCoordinate:
				{
					const ArticulationReducedCoordinate* pArticulationReducedCoordinate = static_cast<const ArticulationReducedCoordinate*>(pArticulationBase);
					return pArticulationReducedCoordinate->GetInterface();
				}
				break;
				case IArticulationBase::Type::eMaximumCoordinate:
				{
					const Articulation* pArticulationOrigin = static_cast<const Articulation*>(pArticulationBase);
					return pArticulationOrigin->GetInterface();
				}
				break;
				default:
					throw_line("unknown type");
					return nullptr;
				}
			}

			physx::PxShape* GetInterface(const IShape* pShape)
			{
				if (pShape == nullptr)
					return nullptr;
				
				return static_cast<const Shape*>(pShape)->GetInterface();
			}

			physx::PxMaterial* GetInterface(const IMaterial* pMaterial)
			{
				if (pMaterial == nullptr)
					return nullptr;

				return static_cast<const Material*>(pMaterial)->GetInterface();
			}

			physx::PxConvexMesh* CreateConvexMesh(const math::float3* pVertices, uint32_t numVertices, physx::PxConvexFlags flags)
			{
				physx::PxPhysics* pPhysics = GetPhysics();
				physx::PxCooking* pCooking = GetCooking();

				physx::PxConvexMeshDesc convexDesc;
				convexDesc.points.stride = sizeof(math::float3);
				convexDesc.points.data = pVertices;
				convexDesc.points.count = numVertices;
				convexDesc.flags = flags;

				physx::PxConvexMesh* pConvexMesh = nullptr;
				physx::PxDefaultMemoryOutputStream stream;
				if (pCooking->cookConvexMesh(convexDesc, stream) == true)
				{
					physx::PxDefaultMemoryInputData inputData(stream.getData(), stream.getSize());
					pConvexMesh = pPhysics->createConvexMesh(inputData);
				}

				return pConvexMesh;
			}

			physx::PxConvexMesh* CreateConvexMesh(const math::float3* pVertices, uint32_t numVertices, const uint32_t* pIndices, uint32_t numIndices, physx::PxConvexFlags flags)
			{
				physx::PxPhysics* pPhysics = GetPhysics();
				physx::PxCooking* pCooking = GetCooking();

				physx::PxConvexMeshDesc convexDesc;
				convexDesc.points.stride = sizeof(math::float3);
				convexDesc.points.data = pVertices;
				convexDesc.points.count = numVertices;
				convexDesc.indices.stride = sizeof(uint32_t);
				convexDesc.indices.data = pIndices;
				convexDesc.indices.count = numIndices;
				convexDesc.flags = flags;

				physx::PxConvexMesh* pConvexMesh = nullptr;
				physx::PxDefaultMemoryOutputStream stream;
				if (pCooking->cookConvexMesh(convexDesc, stream) == true)
				{
					physx::PxDefaultMemoryInputData inputData(stream.getData(), stream.getSize());
					pConvexMesh = pPhysics->createConvexMesh(inputData);
				}

				return pConvexMesh;
			}

			physx::PxTriangleMesh* CreateTriangleMesh(const math::float3* pVertices, uint32_t numVertices, const uint32_t* pIndices, uint32_t numIndices)
			{
				physx::PxPhysics* pPhysics = GetPhysics();
				physx::PxCooking* pCooking = GetCooking();

				physx::PxTriangleMeshDesc triangleMeshDesc;
				triangleMeshDesc.points.stride = sizeof(math::float3);
				triangleMeshDesc.points.data = pVertices;
				triangleMeshDesc.points.count = numVertices;
				triangleMeshDesc.triangles.stride = sizeof(uint32_t) * 3;
				triangleMeshDesc.triangles.data = pIndices;
				triangleMeshDesc.triangles.count = numIndices / 3;

				physx::PxTriangleMesh* pTriangleMesh = nullptr;
				physx::PxDefaultMemoryOutputStream stream;
				if (pCooking->cookTriangleMesh(triangleMeshDesc, stream) == true)
				{
					physx::PxDefaultMemoryInputData inputData(stream.getData(), stream.getSize());
					pTriangleMesh = pPhysics->createTriangleMesh(inputData);
				}

				return pTriangleMesh;
			}

			physx::PxHeightField* CreateHeightField(const HeightFieldGeometry* pHeightFieldGeometry)
			{
				physx::PxPhysics* pPhysics = GetPhysics();
				physx::PxCooking* pCooking = GetCooking();

				physx::PxHeightFieldDesc heightFieldDesc;
				heightFieldDesc.nbRows = pHeightFieldGeometry->numRows;
				heightFieldDesc.nbColumns = pHeightFieldGeometry->numColumns;
				heightFieldDesc.samples.stride = sizeof(float);
				heightFieldDesc.samples.data = pHeightFieldGeometry->pHeightFieldPoints;
				heightFieldDesc.convexEdgeThreshold = pHeightFieldGeometry->convexEdgeThreshold;
				heightFieldDesc.flags = Convert<const physx::PxHeightFieldFlags>(pHeightFieldGeometry->flags);

				physx::PxHeightField* pHeightField = nullptr;
				physx::PxDefaultMemoryOutputStream stream;
				if (pCooking->cookHeightField(heightFieldDesc, stream) == true)
				{
					physx::PxDefaultMemoryInputData inputData(stream.getData(), stream.getSize());
					pHeightField = pPhysics->createHeightField(inputData);
				}

				return pHeightField;
			}
		}
	}
}