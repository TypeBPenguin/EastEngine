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
				Math::Vector2 f2Size;
				Math::Vector2 f2Height;
				const float* pHeightArray;
				uint32_t nHeightArarySize;
			};

			union
			{
				Box box;
				Sphere sphere;
				Cylinder cylinder;
				Capsule capsule;
				Cone cone;
				Hull hull;
				TriangleMesh triangleMesh;
				Terrain terrain;
			};

			EmPhysicsShape::Type emPhysicsShapeType = EmPhysicsShape::eEmpty;
			
			Shape()
			{
			}

			Shape& operator = (const Shape& source)
			{
				box = source.box;
				sphere = source.sphere;
				cylinder = source.cylinder;
				capsule = source.capsule;
				cone = source.cone;

				emPhysicsShapeType = source.emPhysicsShapeType;

				return *this;
			}

			void SetBox(const Math::Vector3& f3Size)
			{
				box.f3Size = f3Size;
				emPhysicsShapeType = Physics::EmPhysicsShape::eBox;
			}

			void SetSphere(float fRadius)
			{
				sphere.fRadius = fRadius;
				emPhysicsShapeType = Physics::EmPhysicsShape::eSphere;
			}

			void SetCylinder(const Math::Vector3& f3HalfExtents)
			{
				cylinder.f3HalfExtents = f3HalfExtents;
				emPhysicsShapeType = Physics::EmPhysicsShape::eCylinder;
			}

			void SetCylinderX(const Math::Vector3& f3HalfExtents)
			{
				cylinder.f3HalfExtents = f3HalfExtents;
				emPhysicsShapeType = Physics::EmPhysicsShape::eCylinder_X;
			}

			void SetCylinderZ(const Math::Vector3& f3HalfExtents)
			{
				cylinder.f3HalfExtents = f3HalfExtents;
				emPhysicsShapeType = Physics::EmPhysicsShape::eCylinder_Z;
			}

			void SetCapsule(float fRadius, float fHeight)
			{
				capsule.fRadius = fRadius;
				capsule.fHeight = fHeight;
				emPhysicsShapeType = Physics::EmPhysicsShape::eCapsule;
			}

			void SetCapsuleX(float fRadius, float fHeight)
			{
				capsule.fRadius = fRadius;
				capsule.fHeight = fHeight;
				emPhysicsShapeType = Physics::EmPhysicsShape::eCapsule_X;
			}

			void SetCapsuleZ(float fRadius, float fHeight)
			{
				capsule.fRadius = fRadius;
				capsule.fHeight = fHeight;
				emPhysicsShapeType = Physics::EmPhysicsShape::eCapsule_Z;
			}

			void SetCone(float fRadius, float fHeight)
			{
				cone.fRadius = fRadius;
				cone.fHeight = fHeight;
				emPhysicsShapeType = Physics::EmPhysicsShape::eCone;
			}

			void SetConeX(float fRadius, float fHeight)
			{
				cone.fRadius = fRadius;
				cone.fHeight = fHeight;
				emPhysicsShapeType = Physics::EmPhysicsShape::eCone_X;
			}

			void SetConeZ(float fRadius, float fHeight)
			{
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

			void SetTriangleMesh(const Math::Vector3* pVertices, uint32_t nVertexCount, const uint32_t* pIndices, uint32_t nIndexCount)
			{
				triangleMesh.pVertices = pVertices;
				triangleMesh.nVertexCount = nVertexCount;
				triangleMesh.pIndices = pIndices;
				triangleMesh.nIndexCount = nIndexCount;
				emPhysicsShapeType = Physics::EmPhysicsShape::eTriangleMesh;
			}

			void SetTerrain(const Math::Vector2& f2Size, const Math::Vector2& f2Height, const float* pHeightArray, uint32_t nHeightArarySize)
			{
				terrain.f2Size = f2Size;
				terrain.f2Height = f2Height;
				terrain.pHeightArray = pHeightArray;
				terrain.nHeightArarySize = nHeightArarySize;
				emPhysicsShapeType = Physics::EmPhysicsShape::eTriangleMesh;
			}
		};

		btCollisionShape* CreateShape(const Shape& shape);
	}
}