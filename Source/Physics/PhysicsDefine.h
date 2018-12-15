#pragma once

#include <variant>

class btCollisionShape;

namespace eastengine
{
	namespace physics
	{
		const float Gravity = -9.80665f;

		enum ActiveStateType
		{
			eActiveTag = 1,
			eIslandSleeping,
			eWantsDeactivation,
			eDisableDeactivation,
			eDisableSimulation,
		};

		enum CollisionFlag
		{
			eStaticObject = 1,
			eKinematicObject = 1 << 1,
			eNoContactResponse = 1 << 2,
			eCustomMaterialCallback = 1 << 3, //this allows per-triangle material (friction/restitution)
			eCharacterObject = 1 << 4,
			eDisableVisualizeObject = 1 << 5, //disable debug drawing
			eDisableSPUCollisionProcessing = 1 << 6, //disable parallel/SPU processing
		};

		enum ShapeType
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

		struct Shape
		{
			struct Box
			{
				math::float3 f3Size = math::float3::One;
			};

			struct Sphere
			{
				float fRadius = 1.f;
			};

			struct Cylinder
			{
				math::float3 f3HalfExtents = math::float3::One;
			};

			struct Capsule
			{
				float fRadius = 1.f;
				float fHeight = 1.f;
			};

			struct Cone
			{
				float fRadius = 1.f;
				float fHeight = 1.f;
			};

			struct Hull
			{
				const math::float3* pVertices = nullptr;
				uint32_t nVertexCount = 0;
				const uint32_t* pIndices = nullptr;
				uint32_t nIndexCount = 0;
			};

			struct TriangleMesh
			{
				const math::float3* pVertices = nullptr;
				uint32_t nVertexCount = 0;
				uint32_t nStride = 0;
				const uint32_t* pIndices = nullptr;
				uint32_t nIndexCount = 0;
			};

			struct Terrain
			{
				math::int2 n2Size = math::int2::One;
				float fHeightScale = 1.f;
				float fHeightMax = 1.f;
				float fHeightMin = 1.f;
				const float* pHeightArray = nullptr;
				uint32_t nHeightArarySize = 0;
			};

			std::variant<Box,
				Sphere,
				Cylinder,
				Capsule,
				Cone,
				Hull,
				TriangleMesh,
				Terrain> element;

			ShapeType emShapeType = ShapeType::eEmpty;
			
			Shape()
			{
			}

			Shape& operator = (const Shape& source)
			{
				element = source.element;

				emShapeType = source.emShapeType;

				return *this;
			}

			void SetBox(const math::float3& f3Size)
			{
				Box& box = element.emplace<Box>();

				box.f3Size = f3Size;
				emShapeType = physics::ShapeType::eBox;
			}

			void SetSphere(float fRadius)
			{
				Sphere& sphere = element.emplace<Sphere>();

				sphere.fRadius = fRadius;
				emShapeType = physics::ShapeType::eSphere;
			}

			void SetCylinder(const math::float3& f3HalfExtents)
			{
				Cylinder& cylinder = element.emplace<Cylinder>();

				cylinder.f3HalfExtents = f3HalfExtents;
				emShapeType = physics::ShapeType::eCylinder;
			}

			void SetCylinderX(const math::float3& f3HalfExtents)
			{
				Cylinder& cylinder = element.emplace<Cylinder>();

				cylinder.f3HalfExtents = f3HalfExtents;
				emShapeType = physics::ShapeType::eCylinder_X;
			}

			void SetCylinderZ(const math::float3& f3HalfExtents)
			{
				Cylinder& cylinder = element.emplace<Cylinder>();

				cylinder.f3HalfExtents = f3HalfExtents;
				emShapeType = physics::ShapeType::eCylinder_Z;
			}

			void SetCapsule(float fRadius, float fHeight)
			{
				Capsule& capsule = element.emplace<Capsule>();

				capsule.fRadius = fRadius;
				capsule.fHeight = fHeight;
				emShapeType = physics::ShapeType::eCapsule;
			}

			void SetCapsuleX(float fRadius, float fHeight)
			{
				Capsule& capsule = element.emplace<Capsule>();

				capsule.fRadius = fRadius;
				capsule.fHeight = fHeight;
				emShapeType = physics::ShapeType::eCapsule_X;
			}

			void SetCapsuleZ(float fRadius, float fHeight)
			{
				Capsule& capsule = element.emplace<Capsule>();

				capsule.fRadius = fRadius;
				capsule.fHeight = fHeight;
				emShapeType = physics::ShapeType::eCapsule_Z;
			}

			void SetCone(float fRadius, float fHeight)
			{
				Cone& cone = element.emplace<Cone>();

				cone.fRadius = fRadius;
				cone.fHeight = fHeight;
				emShapeType = physics::ShapeType::eCone;
			}

			void SetConeX(float fRadius, float fHeight)
			{
				Cone& cone = element.emplace<Cone>();

				cone.fRadius = fRadius;
				cone.fHeight = fHeight;
				emShapeType = physics::ShapeType::eCone_X;
			}

			void SetConeZ(float fRadius, float fHeight)
			{
				Cone& cone = element.emplace<Cone>();

				cone.fRadius = fRadius;
				cone.fHeight = fHeight;
				emShapeType = physics::ShapeType::eCone_Z;
			}

			void SetHull()
			{
				element.emplace<Hull>();

				emShapeType = physics::ShapeType::eHull;
			}

			void SetHull(const math::float3* pVertices, uint32_t nVertexCount, const uint32_t* pIndices, uint32_t nIndexCount)
			{
				Hull& hull = element.emplace<Hull>();

				hull.pVertices = pVertices;
				hull.nVertexCount = nVertexCount;
				hull.pIndices = pIndices;
				hull.nIndexCount = nIndexCount;
				emShapeType = physics::ShapeType::eHull;
			}

			void SetTriangleMesh()
			{
				element.emplace<TriangleMesh>();

				emShapeType = physics::ShapeType::eTriangleMesh;
			}

			void SetTriangleMesh(const math::float3* pVertices, uint32_t nVertexCount)
			{
				TriangleMesh& triangleMesh = element.emplace<TriangleMesh>();

				triangleMesh.pVertices = pVertices;
				triangleMesh.nVertexCount = nVertexCount;
				triangleMesh.pIndices = nullptr;
				triangleMesh.nIndexCount = 0;
				emShapeType = physics::ShapeType::eTriangleMesh;
			}

			void SetTriangleMesh(const math::float3* pVertices, uint32_t nVertexCount, const uint32_t* pIndices, uint32_t nIndexCount)
			{
				TriangleMesh& triangleMesh = element.emplace<TriangleMesh>();

				triangleMesh.pVertices = pVertices;
				triangleMesh.nVertexCount = nVertexCount;
				triangleMesh.pIndices = pIndices;
				triangleMesh.nIndexCount = nIndexCount;
				emShapeType = physics::ShapeType::eTriangleMesh;
			}

			void SetTriangleMesh(const math::float3* pVertices, uint32_t nVertexCount, uint32_t nStride, const uint32_t* pIndices, uint32_t nIndexCount)
			{
				TriangleMesh& triangleMesh = element.emplace<TriangleMesh>();

				triangleMesh.pVertices = pVertices;
				triangleMesh.nVertexCount = nVertexCount;
				triangleMesh.nStride = nStride;
				triangleMesh.pIndices = pIndices;
				triangleMesh.nIndexCount = nIndexCount;
				emShapeType = physics::ShapeType::eTriangleMesh;
			}

			// 이거 누가.. 사용 방법 좀 연구해주셈;;
			// btHeightfieldTerrainShape
			//void SetTerrain(const math::int2& n2Size, const float fHeightScale, const float fHeightMax, const float fHeightMin, const float* pHeightArray, uint32_t nHeightArarySize)
			//{
			//	Terrain& terrain = element.emplace<Terrain>();
			//
			//	terrain.n2Size = n2Size;
			//	terrain.fHeightScale = fHeightScale;
			//	terrain.fHeightMax = fHeightMax;
			//	terrain.fHeightMin = fHeightMin;
			//	terrain.pHeightArray = pHeightArray;
			//	terrain.nHeightArarySize = nHeightArarySize;
			//	emShapeType = physics::ShapeType::eTerrain;
			//}
		};

		std::unique_ptr<btCollisionShape> CreateShape(const Shape& shape);
	}
}