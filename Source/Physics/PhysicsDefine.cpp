#include "stdafx.h"
#include "PhysicsDefine.h"

#include "MathConvertor.h"

#include "BulletCollision/CollisionShapes/btShapeHull.h"
#include "BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h"

namespace eastengine
{
	namespace physics
	{
		static_assert(ActiveStateType::eActiveTag == ACTIVE_TAG, "ActiveState Mismatch");
		static_assert(ActiveStateType::eIslandSleeping == ISLAND_SLEEPING, "ActiveState Mismatch");
		static_assert(ActiveStateType::eWantsDeactivation == WANTS_DEACTIVATION, "ActiveState Mismatch");
		static_assert(ActiveStateType::eDisableDeactivation == DISABLE_DEACTIVATION, "ActiveState Mismatch");
		static_assert(ActiveStateType::eDisableSimulation == DISABLE_SIMULATION, "ActiveState Mismatch");
		
		static_assert(CollisionFlag::eStaticObject == btCollisionObject::CF_STATIC_OBJECT, "CollisionFlags Mismatch");
		static_assert(CollisionFlag::eKinematicObject == btCollisionObject::CF_KINEMATIC_OBJECT, "CollisionFlags Mismatch");
		static_assert(CollisionFlag::eNoContactResponse == btCollisionObject::CF_NO_CONTACT_RESPONSE, "CollisionFlags Mismatch");
		static_assert(CollisionFlag::eCustomMaterialCallback == btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK, "CollisionFlags Mismatch");
		static_assert(CollisionFlag::eCharacterObject == btCollisionObject::CF_CHARACTER_OBJECT, "CollisionFlags Mismatch");
		static_assert(CollisionFlag::eDisableVisualizeObject == btCollisionObject::CF_DISABLE_VISUALIZE_OBJECT, "CollisionFlags Mismatch");
		static_assert(CollisionFlag::eDisableSPUCollisionProcessing == btCollisionObject::CF_DISABLE_SPU_COLLISION_PROCESSING, "CollisionFlags Mismatch");

		std::unique_ptr<btCollisionShape> CreateShape(const Shape& shape)
		{
			switch (shape.emShapeType)
			{
			case ShapeType::eEmpty:
				return nullptr;
			case ShapeType::eBox:
			{
				const Shape::Box* pBox = std::get_if<Shape::Box>(&shape.element);
				if (pBox == nullptr)
					return nullptr;

				return std::make_unique<btBoxShape>(math::ConvertToBt(pBox->f3Size));
			}
			case ShapeType::eSphere:
			{
				const Shape::Sphere* pSphere = std::get_if<Shape::Sphere>(&shape.element);
				if (pSphere == nullptr)
					return nullptr;

				return std::make_unique<btSphereShape>(pSphere->fRadius);
			}
			case ShapeType::eCylinder:
			{
				const Shape::Cylinder* pCylinder = std::get_if<Shape::Cylinder>(&shape.element);
				if (pCylinder == nullptr)
					return nullptr;

				return std::make_unique<btCylinderShape>(math::ConvertToBt(pCylinder->f3HalfExtents));
			}
			case ShapeType::eCylinder_X:
			{
				const Shape::Cylinder* pCylinder = std::get_if<Shape::Cylinder>(&shape.element);
				if (pCylinder == nullptr)
					return nullptr;

				return std::make_unique<btCylinderShapeX>(math::ConvertToBt(pCylinder->f3HalfExtents));
			}
			case ShapeType::eCylinder_Z:
			{
				const Shape::Cylinder* pCylinder = std::get_if<Shape::Cylinder>(&shape.element);
				if (pCylinder == nullptr)
					return nullptr;

				return std::make_unique<btCylinderShapeZ>(math::ConvertToBt(pCylinder->f3HalfExtents));
			}
			case ShapeType::eCapsule:
			{
				const Shape::Capsule* pCapsule = std::get_if<Shape::Capsule>(&shape.element);
				if (pCapsule == nullptr)
					return nullptr;

				return std::make_unique<btCapsuleShape>(pCapsule->fRadius, pCapsule->fHeight);
			}
			case ShapeType::eCapsule_X:
			{
				const Shape::Capsule* pCapsule = std::get_if<Shape::Capsule>(&shape.element);
				if (pCapsule == nullptr)
					return nullptr;

				return std::make_unique<btCapsuleShapeX>(pCapsule->fRadius, pCapsule->fHeight);
			}
			case ShapeType::eCapsule_Z:
			{
				const Shape::Capsule* pCapsule = std::get_if<Shape::Capsule>(&shape.element);
				if (pCapsule == nullptr)
					return nullptr;

				return std::make_unique<btCapsuleShapeZ>(pCapsule->fRadius, pCapsule->fHeight);
			}
			case ShapeType::eCone:
			{
				const Shape::Cone* pCone = std::get_if<Shape::Cone>(&shape.element);
				if (pCone == nullptr)
					return nullptr;

				return std::make_unique<btConeShape>(pCone->fRadius, pCone->fHeight);
			}
			case ShapeType::eCone_X:
			{
				const Shape::Cone* pCone = std::get_if<Shape::Cone>(&shape.element);
				if (pCone == nullptr)
					return nullptr;

				return std::make_unique<btConeShapeX>(pCone->fRadius, pCone->fHeight);
			}
			case ShapeType::eCone_Z:
			{
				const Shape::Cone* pCone = std::get_if<Shape::Cone>(&shape.element);
				if (pCone == nullptr)
					return nullptr;

				return std::make_unique<btConeShapeZ>(pCone->fRadius, pCone->fHeight);
			}
			case ShapeType::eHull:
			{
				const Shape::Hull* pShapeInfo = std::get_if<Shape::Hull>(&shape.element);
				if (pShapeInfo == nullptr)
					return nullptr;

				const math::float3* pVertices = pShapeInfo->pVertices;
				uint32_t nVertexCount = pShapeInfo->nVertexCount;
				const uint32_t* pIndices = pShapeInfo->pIndices;
				uint32_t nIndexCount = pShapeInfo->nIndexCount;

				if (pVertices == nullptr || nVertexCount == 0 || pIndices == nullptr || nIndexCount == 0)
					return nullptr;

				btIndexedMesh pIndexedMesh;
				pIndexedMesh.m_indexType = PHY_ScalarType::PHY_INTEGER;
				pIndexedMesh.m_numTriangles = nIndexCount / 3;
				pIndexedMesh.m_numVertices = nVertexCount;
				pIndexedMesh.m_triangleIndexStride = 3 * sizeof(uint32_t);
				pIndexedMesh.m_vertexStride = sizeof(float) * 3;
				pIndexedMesh.m_vertexType = PHY_ScalarType::PHY_FLOAT;
				pIndexedMesh.m_triangleIndexBase = reinterpret_cast<const unsigned char*>(pIndices);
				pIndexedMesh.m_vertexBase = reinterpret_cast<const unsigned char*>(pVertices);
				btTriangleIndexVertexArray* pMeshInterface = new btTriangleIndexVertexArray;
				pMeshInterface->addIndexedMesh(pIndexedMesh, PHY_ScalarType::PHY_INTEGER);
				btConvexShape* pMeshShape = new btConvexTriangleMeshShape(pMeshInterface);

				btShapeHull* pHullShape = new btShapeHull(pMeshShape);
				btScalar currentMargin = pMeshShape->getMargin();
				pHullShape->buildHull(currentMargin);
				std::unique_ptr<btConvexHullShape> pReducedPolygonStaticMesh = std::make_unique<btConvexHullShape>();

				const btVector3* pVertex = pHullShape->getVertexPointer();
				int nSize = pHullShape->numVertices();
				for (int i = 0; i < nSize; ++i)
				{
					pReducedPolygonStaticMesh->addPoint(pVertex[i], false);
				}

				pReducedPolygonStaticMesh->recalcLocalAabb();

				pReducedPolygonStaticMesh->optimizeConvexHull();
				pReducedPolygonStaticMesh->initializePolyhedralFeatures();

				SafeDelete(pHullShape);
				SafeDelete(pMeshInterface);
				SafeDelete(pMeshShape);

				return pReducedPolygonStaticMesh;
			}
			case ShapeType::eTriangleMesh:
			{
				const Shape::TriangleMesh* pShapeInfo = std::get_if<Shape::TriangleMesh>(&shape.element);
				if (pShapeInfo == nullptr)
					return nullptr;

				const math::float3* pVertices = pShapeInfo->pVertices;
				uint32_t nVertexCount = pShapeInfo->nVertexCount;

				const uint32_t* pIndices = pShapeInfo->pIndices;
				uint32_t nIndexCount = pShapeInfo->nIndexCount;

				if (pVertices == nullptr || nVertexCount == 0)
					return false;

				if (pIndices != nullptr && nIndexCount == 0)
					return false;

				btStridingMeshInterface* pMeshInterface = nullptr;

				if (pIndices != nullptr)
				{
					btTriangleIndexVertexArray* pTriangle = new btTriangleIndexVertexArray;

					btIndexedMesh pIndexedMesh;
					pIndexedMesh.m_indexType = PHY_ScalarType::PHY_INTEGER;
					pIndexedMesh.m_numTriangles = nIndexCount / 3;
					pIndexedMesh.m_numVertices = nVertexCount;
					pIndexedMesh.m_triangleIndexStride = 3 * sizeof(uint32_t);
					pIndexedMesh.m_vertexStride = pShapeInfo->nStride != 0 ? pShapeInfo->nStride : sizeof(float) * 3;
					pIndexedMesh.m_vertexType = PHY_ScalarType::PHY_FLOAT;
					pIndexedMesh.m_triangleIndexBase = reinterpret_cast<const unsigned char*>(pIndices);
					pIndexedMesh.m_vertexBase = reinterpret_cast<const unsigned char*>(pVertices);

					pTriangle->addIndexedMesh(pIndexedMesh, PHY_ScalarType::PHY_INTEGER);

					pMeshInterface = pTriangle;
				}
				else
				{
					btTriangleMesh* pTriangle = new btTriangleMesh;
					for (uint32_t i = 0; i < nVertexCount; i += 3)
					{
						const math::float3& v1 = pVertices[i];
						const math::float3& v2 = pVertices[i + 1];
						const math::float3& v3 = pVertices[i + 2];

						pTriangle->addTriangle(math::ConvertToBt(v1), math::ConvertToBt(v2), math::ConvertToBt(v3));
					}

					pMeshInterface = pTriangle;
				}

				return std::make_unique<btBvhTriangleMeshShape>(pMeshInterface, true);
			}
			case ShapeType::eTerrain:
			{
				// 이거 누가.. 사용 방법좀 연구해주셈;;
				const Shape::Terrain* pShapeInfo = std::get_if<Shape::Terrain>(&shape.element);
				if (pShapeInfo == nullptr)
					return nullptr;

				float heightScale = pShapeInfo->fHeightMax / 65535.f;

				std::unique_ptr<btHeightfieldTerrainShape> pHeightShape = std::make_unique<btHeightfieldTerrainShape>(pShapeInfo->n2Size.x, pShapeInfo->n2Size.y,
					pShapeInfo->pHeightArray, heightScale, pShapeInfo->fHeightMin, pShapeInfo->fHeightMax,
					1, PHY_FLOAT, true);

				pHeightShape->setUseDiamondSubdivision(true);

				return pHeightShape;
			}
			case ShapeType::eStaticPlane:
				return nullptr;
			}

			return nullptr;
		}
	}
}