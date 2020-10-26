#pragma once

#include "PhysicsDefine.h"

namespace physx
{
	class PxCooking;
	class PxPhysics;
	class PxScene;

	class PxActor;
	class PxArticulationBase;
	class PxShape;
	class PxMaterial;

	class PxConvexMesh;
	class PxTriangleMesh;
	class PxHeightField;
}

namespace est
{
	namespace physics
	{
		struct HeightFieldGeometry;
		class IRigidActor;
		class IArticulationBase;
		class IShape;
		class IMaterial;

		template <typename T_to, typename T_from>
		T_to& Convert(T_from& value)
		{
			static_assert(sizeof(T_to) == sizeof(T_from), "mismatch type convert");
			return *reinterpret_cast<T_to*>(&value);
		}

		physx::PxPhysics* GetPhysics();
		physx::PxCooking* GetCooking();
		physx::PxScene* GetScene();

		namespace util
		{
			physx::PxRigidActor* GetInterface(const IRigidActor* pRigidActor);
			physx::PxArticulationBase* GetInterface(const IArticulationBase* pArticulationBase);
			physx::PxShape* GetInterface(const IShape* pShape);
			physx::PxMaterial* GetInterface(const IMaterial* pMaterial);

			physx::PxConvexMesh* CreateConvexMesh(const math::float3* pVertices, uint32_t numVertices, physx::PxConvexFlags flags = physx::PxConvexFlag::eCOMPUTE_CONVEX);
			physx::PxConvexMesh* CreateConvexMesh(const math::float3* pVertices, uint32_t numVertices, const uint32_t* pIndices, uint32_t numIndices, physx::PxConvexFlags flags = physx::PxConvexFlag::eCOMPUTE_CONVEX);

			physx::PxTriangleMesh* CreateTriangleMesh(const math::float3* pVertices, uint32_t numVertices, const uint32_t* pIndices, uint32_t numIndices);
			physx::PxHeightField* CreateHeightField(const HeightFieldGeometry* pHeightGeometry);
		}
	}
}