#pragma once

class btCollisionShape;

namespace EastEngine
{
	namespace Physics
	{
		const float Gravity = -9.80665f;

		namespace EmActiveState
		{
			enum Type
			{
				eActiveTag = 1,
				eIslandSleeping,
				eWantsDeactivation,
				eDisableDeactivation,
				eDisableSimulation,
			};
		}

		namespace EmCollision
		{
			enum Flag
			{
				eStaticObject = 1,
				eKinematicObject = 1 << 1,
				eNoContactResponse = 1 << 2,
				eCustomMaterialCallback = 1 << 3, //this allows per-triangle material (friction/restitution)
				eCharacterObject = 1 << 4,
				eDisableVisualizeObject = 1 << 5, //disable debug drawing
				eDisableSPUCollisionProcessing = 1 << 6, //disable parallel/SPU processing
			};
		}

		namespace EmPhysicsShape
		{
			enum Type
			{
				eEmpty = 0,
				eBox,
				eSphere,
				eCylinder,
				eCylinder_X,
				eCylinder_Z,
				eCapsule,
				eCapsule_X,
				eCapsule_Z,
				eCone,
				eCone_X,
				eCone_Z,
				eHull,
				eTriangleMesh,
				eTerrain,
				eStaticPlane,

				eCount,
			};
		}

		struct Shape
		{
			struct Box
			{
				Math::Vector3 f3Size;
			};

			struct Sphere
			{
				float fRadius;
			};

			struct Cylinder
			{
				Math::Vector3 f3HalfExtents;
			};

			struct Capsule
			{
				float fRadius;
				float fHeight;
			};

			struct Cone
			{
				float fRadius;
				float fHeight;
			};

			struct Hull
			{
				const Math::Vector3* pVertices;
				uint32_t nVertexCount;
				const uint32_t* pIndices;
				uint32_t nIndexCount;
			};

			struct TriangleMesh
			{
				const Math::Vector3* pVertices;
				uint32_t nVertexCount;
				const uint32_t* pIndices;
				uint32_t nIndexCount;
			};

			struct Terrain
			{
				Math::Int2 n2Size;
				float fHeightScale;
				float fHeightMax;
				float fHeightMin;
				const float* pHeightArray;
				uint32_t nHeightArarySize;
			};

			std::variant<Box,
				Sphere,
				Cylinder,
				Capsule,
				Cone,
				Hull,
				TriangleMesh,
				Terrain> element;

			EmPhysicsShape::Type emPhysicsShapeType = EmPhysicsShape::eEmpty;
			
			Shape()
			{
			}

			Shape& operator = (const Shape& source)
			{
				element = source.element;

				emPhysicsShapeType = source.emPhysicsShapeType;

				return *this;
			}

			void SetBox(const Math::Vector3& f3Size)
			{
				Box& box = element.emplace<Box>();

				box.f3Size = f3Size;
				emPhysicsShapeType = Physics::EmPhysicsShape::eBox;
			}

			void SetSphere(float fRadius)
			{
				Sphere& sphere = element.emplace<Sphere>();

				sphere.fRadius = fRadius;
				emPhysicsShapeType = Physics::EmPhysicsShape::eSphere;
			}

			void SetCylinder(const Math::Vector3& f3HalfExtents)
			{
				Cylinder& cylinder = element.emplace<Cylinder>();

				cylinder.f3HalfExtents = f3HalfExtents;
				emPhysicsShapeType = Physics::EmPhysicsShape::eCylinder;
			}

			void SetCylinderX(const Math::Vector3& f3HalfExtents)
			{
				Cylinder& cylinder = element.emplace<Cylinder>();

				cylinder.f3HalfExtents = f3HalfExtents;
				emPhysicsShapeType = Physics::EmPhysicsShape::eCylinder_X;
			}

			void SetCylinderZ(const Math::Vector3& f3HalfExtents)
			{
				Cylinder& cylinder = element.emplace<Cylinder>();

				cylinder.f3HalfExtents = f3HalfExtents;
				emPhysicsShapeType = Physics::EmPhysicsShape::eCylinder_Z;
			}

			void SetCapsule(float fRadius, float fHeight)
			{
				Capsule& capsule = element.emplace<Capsule>();

				capsule.fRadius = fRadius;
				capsule.fHeight = fHeight;
				emPhysicsShapeType = Physics::EmPhysicsShape::eCapsule;
			}

			void SetCapsuleX(float fRadius, float fHeight)
			{
				Capsule& capsule = element.emplace<Capsule>();

				capsule.fRadius = fRadius;
				capsule.fHeight = fHeight;
				emPhysicsShapeType = Physics::EmPhysicsShape::eCapsule_X;
			}

			void SetCapsuleZ(float fRadius, float fHeight)
			{
				Capsule& capsule = element.emplace<Capsule>();

				capsule.fRadius = fRadius;
				capsule.fHeight = fHeight;
				emPhysicsShapeType = Physics::EmPhysicsShape::eCapsule_Z;
			}

			void SetCone(float fRadius, float fHeight)
			{
				Cone& cone = element.emplace<Cone>();

				cone.fRadius = fRadius;
				cone.fHeight = fHeight;
				emPhysicsShapeType = Physics::EmPhysicsShape::eCone;
			}

			void SetConeX(float fRadius, float fHeight)
			{
				Cone& cone = element.emplace<Cone>();

				cone.fRadius = fRadius;
				cone.fHeight = fHeight;
				emPhysicsShapeType = Physics::EmPhysicsShape::eCone_X;
			}

			void SetConeZ(float fRadius, float fHeight)
			{
				Cone& cone = element.emplace<Cone>();

				cone.fRadius = fRadius;
				cone.fHeight = fHeight;
				emPhysicsShapeType = Physics::EmPhysicsShape::eCone_Z;
			}

			void SetHull()
			{
				emPhysicsShapeType = Physics::EmPhysicsShape::eHull;
			}

			void SetHull(const Math::Vector3* pVertices, uint32_t nVertexCount, const uint32_t* pIndices, uint32_t nIndexCount)
			{
				Hull& hull = element.emplace<Hull>();

				hull.pVertices = pVertices;
				hull.nVertexCount = nVertexCount;
				hull.pIndices = pIndices;
				hull.nIndexCount = nIndexCount;
				emPhysicsShapeType = Physics::EmPhysicsShape::eHull;
			}

			void SetTriangleMesh()
			{
				emPhysicsShapeType = Physics::EmPhysicsShape::eTriangleMesh;
			}

			void SetTriangleMesh(const Math::Vector3* pVertices, uint32_t nVertexCount)
			{
				TriangleMesh& triangleMesh = element.emplace<TriangleMesh>();

				triangleMesh.pVertices = pVertices;
				triangleMesh.nVertexCount = nVertexCount;
				triangleMesh.pIndices = nullptr;
				triangleMesh.nIndexCount = 0;
				emPhysicsShapeType = Physics::EmPhysicsShape::eTriangleMesh;
			}

			void SetTriangleMesh(const Math::Vector3* pVertices, uint32_t nVertexCount, const uint32_t* pIndices, uint32_t nIndexCount)
			{
				TriangleMesh& triangleMesh = element.emplace<TriangleMesh>();

				triangleMesh.pVertices = pVertices;
				triangleMesh.nVertexCount = nVertexCount;
				triangleMesh.pIndices = pIndices;
				triangleMesh.nIndexCount = nIndexCount;
				emPhysicsShapeType = Physics::EmPhysicsShape::eTriangleMesh;
			}

			// 이거 누가.. 사용 방법 좀 연구해주셈;;
			// btHeightfieldTerrainShape
			//void SetTerrain(const Math::Int2& n2Size, const float fHeightScale, const float fHeightMax, const float fHeightMin, const float* pHeightArray, uint32_t nHeightArarySize)
			//{
			//	Terrain& terrain = element.emplace<Terrain>();
			//
			//	terrain.n2Size = n2Size;
			//	terrain.fHeightScale = fHeightScale;
			//	terrain.fHeightMax = fHeightMax;
			//	terrain.fHeightMin = fHeightMin;
			//	terrain.pHeightArray = pHeightArray;
			//	terrain.nHeightArarySize = nHeightArarySize;
			//	emPhysicsShapeType = Physics::EmPhysicsShape::eTerrain;
			//}
		};

		btCollisionShape* CreateShape(const Shape& shape);
	}
}