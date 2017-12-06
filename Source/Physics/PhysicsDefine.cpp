#include "stdafx.h"
#include "PhysicsDefine.h"

#include "MathConvertor.h"

#include "BulletCollision/CollisionShapes/btShapeHull.h"
#include "BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h"

namespace EastEngine
{
	namespace Physics
	{
		btCollisionShape* CreateShape(const Shape& shape)
		{
			switch (shape.emPhysicsShapeType)
			{
			case EmPhysicsShape::eEmpty:
				return nullptr;
			case EmPhysicsShape::eBox:
			{
				const Shape::Box* pBox = std::get_if<Shape::Box>(&shape.element);
				if (pBox == nullptr)
					return nullptr;

				return new btBoxShape(Math::ConvertToBt(pBox->f3Size));
			}
			case EmPhysicsShape::eSphere:
			{
				const Shape::Sphere* pSphere = std::get_if<Shape::Sphere>(&shape.element);
				if (pSphere == nullptr)
					return nullptr;

				return new btSphereShape(pSphere->fRadius);
			}
			case EmPhysicsShape::eCylinder:
			{
				const Shape::Cylinder* pCylinder = std::get_if<Shape::Cylinder>(&shape.element);
				if (pCylinder == nullptr)
					return nullptr;

				return new btCylinderShape(Math::ConvertToBt(pCylinder->f3HalfExtents));
			}
			case EmPhysicsShape::eCylinder_X:
			{
				const Shape::Cylinder* pCylinder = std::get_if<Shape::Cylinder>(&shape.element);
				if (pCylinder == nullptr)
					return nullptr;

				return new btCylinderShapeX(Math::ConvertToBt(pCylinder->f3HalfExtents));
			}
			case EmPhysicsShape::eCylinder_Z:
			{
				const Shape::Cylinder* pCylinder = std::get_if<Shape::Cylinder>(&shape.element);
				if (pCylinder == nullptr)
					return nullptr;

				return new btCylinderShapeZ(Math::ConvertToBt(pCylinder->f3HalfExtents));
			}
			case EmPhysicsShape::eCapsule:
			{
				const Shape::Capsule* pCapsule = std::get_if<Shape::Capsule>(&shape.element);
				if (pCapsule == nullptr)
					return nullptr;

				return new btCapsuleShape(pCapsule->fRadius, pCapsule->fHeight);
			}
			case EmPhysicsShape::eCapsule_X:
			{
				const Shape::Capsule* pCapsule = std::get_if<Shape::Capsule>(&shape.element);
				if (pCapsule == nullptr)
					return nullptr;

				return new btCapsuleShapeX(pCapsule->fRadius, pCapsule->fHeight);
			}
			case EmPhysicsShape::eCapsule_Z:
			{
				const Shape::Capsule* pCapsule = std::get_if<Shape::Capsule>(&shape.element);
				if (pCapsule == nullptr)
					return nullptr;

				return new btCapsuleShapeZ(pCapsule->fRadius, pCapsule->fHeight);
			}
			case EmPhysicsShape::eCone:
			{
				const Shape::Cone* pCone = std::get_if<Shape::Cone>(&shape.element);
				if (pCone == nullptr)
					return nullptr;

				return new btConeShape(pCone->fRadius, pCone->fHeight);
			}
			case EmPhysicsShape::eCone_X:
			{
				const Shape::Cone* pCone = std::get_if<Shape::Cone>(&shape.element);
				if (pCone == nullptr)
					return nullptr;

				return new btConeShapeX(pCone->fRadius, pCone->fHeight);
			}
			case EmPhysicsShape::eCone_Z:
			{
				const Shape::Cone* pCone = std::get_if<Shape::Cone>(&shape.element);
				if (pCone == nullptr)
					return nullptr;

				return new btConeShapeZ(pCone->fRadius, pCone->fHeight);
			}
			case EmPhysicsShape::eHull:
			{
				const Shape::Hull* pShapeInfo = std::get_if<Shape::Hull>(&shape.element);
				if (pShapeInfo == nullptr)
					return nullptr;

				const Math::Vector3* pVertices = pShapeInfo->pVertices;
				uint32_t nVertexCount = pShapeInfo->nVertexCount;
				const uint32_t* pIndices = pShapeInfo->pIndices;
				uint32_t nIndexCount = pShapeInfo->nIndexCount;

				if (pVertices == nullptr || nVertexCount == 0 || pIndices == nullptr || nIndexCount == 0)
					return nullptr;

				btIndexedMesh pIndexedMesh;
				pIndexedMesh.m_indexType = PHY_ScalarType::PHY_INTEGER;
				pIndexedMesh.m_numTriangles = nIndexCount;
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
				btConvexHullShape* pReducedPolygonStaticMesh = new btConvexHullShape;

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
			case EmPhysicsShape::eTriangleMesh:
			{
				const Shape::TriangleMesh* pShapeInfo = std::get_if<Shape::TriangleMesh>(&shape.element);
				if (pShapeInfo == nullptr)
					return nullptr;

				const Math::Vector3* pVertices = pShapeInfo->pVertices;
				uint32_t nVertexCount = pShapeInfo->nVertexCount;

				const uint32_t* pIndices = pShapeInfo->pIndices;
				uint32_t nIndexCount = pShapeInfo->nIndexCount;

				if (pVertices == nullptr || nVertexCount == 0)
					return false;

				if (pIndices != nullptr && nIndexCount == 0)
					return false;

				btTriangleMesh* pTriangleMesh = new btTriangleMesh;

				if (pIndices != nullptr)
				{
					for (uint32_t i = 0; i < nIndexCount; i += 3)
					{
						const Math::Vector3& v1 = pVertices[pIndices[i]];
						const Math::Vector3& v2 = pVertices[pIndices[i + 1]];
						const Math::Vector3& v3 = pVertices[pIndices[i + 2]];

						pTriangleMesh->addTriangle(Math::ConvertToBt(v1), Math::ConvertToBt(v2), Math::ConvertToBt(v3));
					}
				}
				else
				{
					for (uint32_t i = 0; i < nVertexCount; i += 3)
					{
						const Math::Vector3& v1 = pVertices[i];
						const Math::Vector3& v2 = pVertices[i + 1];
						const Math::Vector3& v3 = pVertices[i + 2];

						pTriangleMesh->addTriangle(Math::ConvertToBt(v1), Math::ConvertToBt(v2), Math::ConvertToBt(v3));
					}
				}

				btBvhTriangleMeshShape* pTriShape = new btBvhTriangleMeshShape(pTriangleMesh, true);

				return pTriShape;
			}
			case EmPhysicsShape::eTerrain:
			{
				const Shape::Terrain* pShapeInfo = std::get_if<Shape::Terrain>(&shape.element);
				if (pShapeInfo == nullptr)
					return nullptr;

				float heightScale = pShapeInfo->fHeightMax / 65535.f;

				btHeightfieldTerrainShape* pHeightShape = new btHeightfieldTerrainShape(pShapeInfo->n2Size.x, pShapeInfo->n2Size.y,
					pShapeInfo->pHeightArray, heightScale, pShapeInfo->fHeightMin, pShapeInfo->fHeightMax,
					1, PHY_FLOAT, true);

				//btHeightfieldTerrainShape* pHeightShape = new btHeightfieldTerrainShape(pShapeInfo->n2Size.x, pShapeInfo->n2Size.y,
				//	pShapeInfo->pHeightArray, pShapeInfo->fHeightMax, 1, true, false);

				pHeightShape->setUseDiamondSubdivision(true);

				return pHeightShape;
			}
			case EmPhysicsShape::eStaticPlane:
				return nullptr;
			}

			return nullptr;
		}
	}
}