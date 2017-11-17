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
				return new btBoxShape(Math::ConvertToBt(shape.box.f3Size));
			case EmPhysicsShape::eSphere:
				return new btSphereShape(shape.sphere.fRadius);
			case EmPhysicsShape::eCylinder:
				return new btCylinderShape(Math::ConvertToBt(shape.cylinder.f3HalfExtents));
			case EmPhysicsShape::eCylinder_X:
				return new btCylinderShapeX(Math::ConvertToBt(shape.cylinder.f3HalfExtents));
			case EmPhysicsShape::eCylinder_Z:
				return new btCylinderShapeZ(Math::ConvertToBt(shape.cylinder.f3HalfExtents));
			case EmPhysicsShape::eCapsule:
				return new btCapsuleShape(shape.capsule.fRadius, shape.capsule.fHeight);
			case EmPhysicsShape::eCapsule_X:
				return new btCapsuleShapeX(shape.capsule.fRadius, shape.capsule.fHeight);
			case EmPhysicsShape::eCapsule_Z:
				return new btCapsuleShapeZ(shape.capsule.fRadius, shape.capsule.fHeight);
			case EmPhysicsShape::eCone:
				return new btConeShape(shape.cone.fRadius, shape.cone.fHeight);
			case EmPhysicsShape::eCone_X:
				return new btConeShapeX(shape.cone.fRadius, shape.cone.fHeight);
			case EmPhysicsShape::eCone_Z:
				return new btConeShapeZ(shape.cone.fRadius, shape.cone.fHeight);
			case EmPhysicsShape::eHull:
			{
				const Math::Vector3* pVertices = shape.hull.pVertices;
				uint32_t nVertexCount = shape.hull.nVertexCount;
				const uint32_t* pIndices = shape.hull.pIndices;
				uint32_t nIndexCount = shape.hull.nIndexCount;

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

				/*Graphics::CVertexCollector<Graphics::VertexPos> vecNewVertex;
				vecNewVertex.reserve(pHullShape->numVertices());

				Graphics::IndexCollector<uint32_t> vecNewIndex;
				vecNewIndex.reserve(pHullShape->numIndices());*/

				const btVector3* pVertex = pHullShape->getVertexPointer();
				int nSize = pHullShape->numVertices();
				for (int i = 0; i < nSize; ++i)
				{
					pReducedPolygonStaticMesh->addPoint(pVertex[i], false);
					//vecNewVertex.push_back(Math::Convert(pVertex[i]));
				}

				/*const uint32_t* pIndex = pHullShape->getIndexPointer();
				nSize = pHullShape->numIndices();
				for (int i = 0; i < nSize; ++i)
				{
				vecNewIndex.push_back(pIndex[i]);
				}*/

				pReducedPolygonStaticMesh->recalcLocalAabb();

				pReducedPolygonStaticMesh->optimizeConvexHull();
				pReducedPolygonStaticMesh->initializePolyhedralFeatures();

				SafeDelete(pHullShape);
				SafeDelete(pMeshInterface);
				SafeDelete(pMeshShape);

				return pReducedPolygonStaticMesh;

				//TriMeshData trimeshshape;
				//btBvhTriangleMeshShape *triangleMeshShape = CreateBvhTriangleMeshShape(pMesh, &trimeshshape);

				//btTriangleMesh* triangleMesh = new btTriangleMesh();
				//for (DWORD i = 0; i < pMesh->GetNumFaces(); i++)
				//{
				//	int index0 = trimeshshape.indices[i * 3];
				//	int index1 = trimeshshape.indices[i * 3 + 1];
				//	int index2 = trimeshshape.indices[i * 3 + 2];

				//	btVector3 vertex0(trimeshshape.vertices[index0 * 3], trimeshshape.vertices[index0 * 3 + 1], trimeshshape.vertices[index0 * 3 + 2]);
				//	btVector3 vertex1(trimeshshape.vertices[index1 * 3], trimeshshape.vertices[index1 * 3 + 1], trimeshshape.vertices[index1 * 3 + 2]);
				//	btVector3 vertex2(trimeshshape.vertices[index2 * 3], trimeshshape.vertices[index2 * 3 + 1], trimeshshape.vertices[index2 * 3 + 2]);

				//	triangleMesh->addTriangle(vertex0, vertex1, vertex2);
				//}

				//btConvexShape* tmpConvexShape = new btConvexTriangleMeshShape(triangleMesh);

				//// Create a hull approximation
				//btShapeHull* hull = new btShapeHull(tmpConvexShape);
				//btScalar margin = tmpConvexShape->getMargin();
				//hull->buildHull(margin);

				//btConvexHullShape* simplifiedConvexShape = new btConvexHullShape();
				//for (int i = 0; i < hull->numVertices(); i++)
				//{
				//	simplifiedConvexShape->addPoint(hull->getVertexPointer()[i]);
				//}

				//delete triangleMeshShape;
				//delete triangleMesh;
				//delete tmpConvexShape;
				//delete hull;

				//return simplifiedConvexShape;
			}
			case EmPhysicsShape::eTriangleMesh:
			{
				const Math::Vector3* pVertices = shape.triangleMesh.pVertices;
				uint32_t nVertexCount = shape.triangleMesh.nVertexCount;
				const uint32_t* pIndices = shape.triangleMesh.pIndices;
				uint32_t nIndexCount = shape.triangleMesh.nIndexCount;

				if (pVertices == nullptr || nVertexCount == 0 || pIndices == nullptr || nIndexCount == 0)
					return false;

				btTriangleMesh* pTriangleMesh = new btTriangleMesh;

				for (uint32_t i = 0; i < nIndexCount; i += 3)
				{
					const Math::Vector3& v1 = pVertices[pIndices[i]];
					const Math::Vector3& v2 = pVertices[pIndices[i + 1]];
					const Math::Vector3& v3 = pVertices[pIndices[i + 2]];

					pTriangleMesh->addTriangle(Math::ConvertToBt(v1), Math::ConvertToBt(v2), Math::ConvertToBt(v3));
				}

				btBvhTriangleMeshShape* pTriShape = new btBvhTriangleMeshShape(pTriangleMesh, true);

				return pTriShape;

				//DWORD numVertices = pMesh->GetNumVertices();
				//DWORD numFaces = pMesh->GetNumFaces();

				//Vertex *v = 0;
				//pMesh->LockVertexBuffer(0, (void**)&v);

				//// Extract vertices
				//pData->vertices = new btScalar[numVertices * 3];

				//for (DWORD i = 0; i < numVertices; i++)
				//{
				//	pData->vertices[i * 3 + 0] = v[i].position.x;
				//	pData->vertices[i * 3 + 1] = v[i].position.y;
				//	pData->vertices[i * 3 + 2] = v[i].position.z;
				//}

				//pMesh->UnlockVertexBuffer();

				//// Extract indices
				//pData->indices = new int[numFaces * 3];
				//WORD* ind = 0;
				//pMesh->LockIndexBuffer(0, (void**)&ind);

				////memcpy( &indices, &ind, sizeof(ind));	
				//for (DWORD i = 0; i < numFaces; i++)
				//{
				//	pData->indices[i * 3 + 0] = ind[i * 3 + 0];
				//	pData->indices[i * 3 + 1] = ind[i * 3 + 1];
				//	pData->indices[i * 3 + 2] = ind[i * 3 + 2];
				//}

				//pMesh->UnlockIndexBuffer();

				//int indexStride = 3 * sizeof(int);
				//int vertStride = sizeof(btVector3);

				//pData->indexVertexArrays = new btTriangleIndexVertexArray(numFaces, pData->indices, indexStride,
				//	numVertices, (btScalar*)&pData->vertices[0], sizeof(btScalar) * 3);

				//bool useQuantizedAabbCompression = true;
				//btBvhTriangleMeshShape *shape = new btBvhTriangleMeshShape(pData->indexVertexArrays, true);

				//// The indices, vertices, and array needs to be manually deleted else Bullet will get a access violation
				////delete indices;
				////delete vertices;	
				////delete indexVertexArrays;	

				//return shape;
			}
			case EmPhysicsShape::eStaticPlane:
				return nullptr;
			}

			return nullptr;
		}
	}
}