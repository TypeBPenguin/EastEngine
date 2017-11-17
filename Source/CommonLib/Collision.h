#pragma once

#include "Math.h"

namespace EastEngine
{
	namespace Collision
	{
		namespace EmContainment
		{
			enum Type
			{
				eDisjoint = 0,
				eIntersects = 1,
				eContains = 2,
			};
		}

		namespace EmPlaneIntersection
		{
			enum Type
			{
				eFront = 0,
				eIntersecting = 1,
				eBack = 2,
			};
		}

		// Returns 8 corners position of bounding frustum.
		//     Near    Far
		//    0----1  4----5
		//    |    |  |    |
		//    |    |  |    |
		//    3----2  7----6
		enum EmCorners
		{
			eNearLeftTop = 0,
			eNearRightTop,
			eNearRightBottom,
			eNearLeftBottom,
			eFarLeftTop,
			eFarRightTop,
			eFarRightBottom,
			eFarLeftBottom,
			CornerCount,
		};

		struct AABB;
		struct OBB;
		struct Frustum;
		
		//-------------------------------------------------------------------------------------
		// Bounding sphere
		//-------------------------------------------------------------------------------------
		struct Sphere
		{
			Math::Vector3 Center;		// Center of the sphere.
			float Radius;				// Radius of the sphere.

			// Creators
			Sphere();
			Sphere(_In_ const Math::Vector3& center, _In_ float radius);
			Sphere(_In_ const Sphere& sp);

			// Methods
			Sphere& operator=(_In_ const Sphere& sp) { Center = sp.Center; Radius = sp.Radius; return *this; }

			void __vectorcall Transform(_Out_ Sphere& Out, _In_ const Math::Matrix& matrix) const;
			void __vectorcall Transform(_Out_ Sphere& Out, _In_ float Scale, _In_ const Math::Quaternion& Rotation, _In_ const Math::Vector3& Translation) const;
			// Transform the sphere

			EmContainment::Type __vectorcall Contains(_In_ const Math::Vector3& Point) const;
			EmContainment::Type __vectorcall Contains(_In_ const Math::Vector3& V0, _In_ const Math::Vector3& V1, _In_ const Math::Vector3& V2) const;
			EmContainment::Type Contains(_In_ const Sphere& sh) const;
			EmContainment::Type Contains(_In_ const AABB& box) const;
			EmContainment::Type Contains(_In_ const OBB& box) const;
			EmContainment::Type Contains(_In_ const Frustum& fr) const;

			bool Intersects(_In_ const Sphere& sh) const;
			bool Intersects(_In_ const AABB& box) const;
			bool Intersects(_In_ const OBB& box) const;
			bool Intersects(_In_ const Frustum& fr) const;

			bool __vectorcall Intersects(_In_ const Math::Vector3& v0, _In_ const Math::Vector3& v1, _In_ const Math::Vector3& v2) const;
			// Triangle-sphere test

			EmPlaneIntersection::Type __vectorcall Intersects(_In_ Math::Plane& Plane) const;
			// Plane-sphere test

			bool __vectorcall Intersects(_In_ const Math::Vector3& Origin, _In_ const Math::Vector3& Direction, _Out_ float& Dist) const;
			// Ray-sphere test

			EmContainment::Type __vectorcall ContainedBy(_In_ const Math::Plane& Plane0, _In_ const Math::Plane& Plane1, _In_ const Math::Plane& Plane2,
				_In_ const Math::Plane& Plane3, _In_ const Math::Plane& Plane4, _In_ const Math::Plane& Plane5) const;
			// Test sphere against six planes (see Frustum::GetPlanes)

			// Static methods
			static void CreateMerged(_Out_ Sphere& Out, _In_ const Sphere& S1, _In_ const Sphere& S2);

			static void CreateFromAABB(_Out_ Sphere& Out, _In_ const AABB& box);
			static void CreateFromAABB(_Out_ Sphere& Out, _In_ const OBB& box);

			static void CreateFromPoints(_Out_ Sphere& Out, _In_ size_t Count,
				_In_reads_bytes_(sizeof(Math::Vector3) + Stride*(Count - 1)) const Math::Vector3* pPoints, _In_ size_t Stride);

			static void CreateFromFrustum(_Out_ Sphere& Out, _In_ const Frustum& fr);
		};

		//-------------------------------------------------------------------------------------
		// Axis-aligned bounding box
		//-------------------------------------------------------------------------------------
		struct AABB
		{
			static const size_t CORNER_COUNT = 8;

			Math::Vector3 Center;			// Center of the box.
			Math::Vector3 Extents;			// Distance from the center to each side.

			// Creators
			AABB();
			AABB(_In_ const Math::Vector3& center, _In_ const Math::Vector3& extents);
			AABB(_In_ const AABB& box);

			// Methods
			AABB& operator=(_In_ const AABB& box) { Center = box.Center; Extents = box.Extents; return *this; }

			void __vectorcall Transform(_Out_ AABB& Out, _In_ const Math::Matrix& matrix) const;
			void __vectorcall Transform(_Out_ AABB& Out, _In_ float Scale, _In_ const Math::Quaternion& Rotation, _In_ const Math::Vector3& Translation) const;

			void GetCorners(_Out_writes_(8) Math::Vector3* Corners) const;
			// Gets the 8 corners of the box

			EmContainment::Type __vectorcall Contains(_In_ const Math::Vector3& Point) const;
			EmContainment::Type __vectorcall Contains(_In_ const Math::Vector3& v0, _In_ const Math::Vector3& v1, _In_ const Math::Vector3& v2) const;
			EmContainment::Type Contains(_In_ const Sphere& sh) const;
			EmContainment::Type Contains(_In_ const AABB& box) const;
			EmContainment::Type Contains(_In_ const OBB& box) const;
			EmContainment::Type Contains(_In_ const Frustum& fr) const;

			bool Intersects(_In_ const Sphere& sh) const;
			bool Intersects(_In_ const AABB& box) const;
			bool Intersects(_In_ const OBB& box) const;
			bool Intersects(_In_ const Frustum& fr) const;

			bool __vectorcall Intersects(_In_ const Math::Vector3& V0, _In_ const Math::Vector3& V1, _In_ const Math::Vector3& V2) const;
			// Triangle-Box test

			EmPlaneIntersection::Type __vectorcall Intersects(_In_ const Math::Plane& Plane) const;
			// Plane-box test

			bool __vectorcall Intersects(_In_ const Math::Vector3& Origin, _In_ const Math::Vector3& Direction, _Out_ float& Dist) const;
			// Ray-Box test

			EmContainment::Type __vectorcall ContainedBy(_In_ const Math::Plane& Plane0, _In_ const Math::Plane& Plane1, _In_ const Math::Plane& Plane2,
				_In_ const Math::Plane& Plane3, _In_ const Math::Plane& Plane4, _In_ const Math::Plane& Plane5) const;
			// Test box against six planes (see Frustum::GetPlanes)

			// Static methods
			static void CreateMerged(_Out_ AABB& Out, _In_ const AABB& b1, _In_ const AABB& b2);

			static void CreateFromSphere(_Out_ AABB& Out, _In_ const Sphere& sh);

			static void __vectorcall CreateFromPoints(_Out_ AABB& Out, _In_ const Math::Vector3& pt1, _In_ const Math::Vector3& pt2);
			static void CreateFromPoints(_Out_ AABB& Out, _In_ size_t Count,
				_In_reads_bytes_(sizeof(Math::Vector3) + Stride*(Count - 1)) const Math::Vector3* pPoints, _In_ size_t Stride);
		};

		//-------------------------------------------------------------------------------------
		// Oriented bounding box
		//-------------------------------------------------------------------------------------
		struct OBB
		{
			static const size_t CORNER_COUNT = 8;

			Math::Vector3 Center;            // Center of the box.
			Math::Vector3 Extents;           // Distance from the center to each side.
			Math::Quaternion Orientation;       // Unit quaternion representing rotation (box -> world).

			// Creators
			OBB();
			OBB(_In_ const Math::Vector3& _Center, _In_ const Math::Vector3& _Extents, _In_ const Math::Quaternion& _Orientation);
			OBB(_In_ const OBB& box);

			// Methods
			OBB& operator=(_In_ const OBB& box) { Center = box.Center; Extents = box.Extents; Orientation = box.Orientation; return *this; }

			void __vectorcall Transform(_Out_ OBB& Out, _In_ const Math::Matrix& M) const;
			void __vectorcall Transform(_Out_ OBB& Out, _In_ float Scale, _In_ const Math::Quaternion& Rotation, _In_ const Math::Vector3& Translation) const;

			void GetCorners(_Out_writes_(8) Math::Vector3* Corners) const;
			// Gets the 8 corners of the box

			EmContainment::Type __vectorcall Contains(_In_ const Math::Vector3& Point) const;
			EmContainment::Type __vectorcall Contains(_In_ const Math::Vector3& V0, _In_ const Math::Vector3& V1, _In_ const Math::Vector3& V2) const;
			EmContainment::Type Contains(_In_ const Sphere& sh) const;
			EmContainment::Type Contains(_In_ const AABB& box) const;
			EmContainment::Type Contains(_In_ const OBB& box) const;
			EmContainment::Type Contains(_In_ const Frustum& fr) const;

			bool Intersects(_In_ const Sphere& sh) const;
			bool Intersects(_In_ const AABB& box) const;
			bool Intersects(_In_ const OBB& box) const;
			bool Intersects(_In_ const Frustum& fr) const;

			bool __vectorcall Intersects(_In_ const Math::Vector3& V0, _In_ const Math::Vector3& V1, _In_ const Math::Vector3& V2) const;
			// Triangle-OrientedBox test

			EmPlaneIntersection::Type __vectorcall Intersects(_In_ const Math::Plane& Plane) const;
			// Plane-OrientedBox test

			bool __vectorcall Intersects(_In_ const Math::Vector3& Origin, _In_ const Math::Vector3& Direction, _Out_ float& Dist) const;
			// Ray-OrientedBox test

			EmContainment::Type __vectorcall ContainedBy(_In_ const Math::Plane& Plane0, _In_ const Math::Plane& Plane1, _In_ const Math::Plane& Plane2,
				_In_ const Math::Plane& Plane3, _In_ const Math::Plane& Plane4, _In_ const Math::Plane& Plane5) const;
			// Test OrientedBox against six planes (see Frustum::GetPlanes)

			// Static methods
			static void CreateFromAABB(_Out_ OBB& Out, _In_ const AABB& box);

			static void CreateFromPoints(_Out_ OBB& Out, _In_ size_t Count,
				_In_reads_bytes_(sizeof(Math::Vector3) + Stride*(Count - 1)) const Math::Vector3* pPoints, _In_ size_t Stride);
		};

		//-------------------------------------------------------------------------------------
		// Frustum
		//-------------------------------------------------------------------------------------
		struct Frustum
		{
			static const size_t CORNER_COUNT = 8;

			Math::Vector3 Origin;            // Origin of the frustum (and projection).
			Math::Quaternion Orientation;       // Quaternion representing rotation.

			float RightSlope;           // Positive X slope (X/Z).
			float LeftSlope;            // Negative X slope.
			float TopSlope;             // Positive Y slope (Y/Z).
			float BottomSlope;          // Negative Y slope.
			float Near, Far;            // Z of the near plane and far plane.

			// Creators
			Frustum();
			Frustum(_In_ const Math::Vector3& _Origin, _In_ const Math::Quaternion& _Orientation,
				_In_ float _RightSlope, _In_ float _LeftSlope, _In_ float _TopSlope, _In_ float _BottomSlope,
				_In_ float _Near, _In_ float _Far);
			Frustum(_In_ const Frustum& fr);
			Frustum(_In_ const Math::Matrix& Projection);

			// Methods
			Frustum& operator=(_In_ const Frustum& fr) {
				Origin = fr.Origin; Orientation = fr.Orientation;
				RightSlope = fr.RightSlope; LeftSlope = fr.LeftSlope;
				TopSlope = fr.TopSlope; BottomSlope = fr.BottomSlope;
				Near = fr.Near; Far = fr.Far; return *this;
			}

			void __vectorcall Transform(_In_ const Math::Matrix& matrix);
			void __vectorcall Transform(_In_ float Scale, _In_ const Math::Quaternion& Rotation, _In_ const Math::Vector3& Translation);

			void GetCorners(_Out_writes_(8) Math::Vector3* Corners) const;
			// Gets the 8 corners of the frustum

			EmContainment::Type __vectorcall Contains(_In_ const Math::Vector3& Point) const;
			EmContainment::Type __vectorcall Contains(_In_ const Math::Vector3& V0, _In_ const Math::Vector3& V1, _In_ const Math::Vector3& V2) const;
			EmContainment::Type Contains(_In_ const Sphere& sh) const;
			EmContainment::Type Contains(_In_ const AABB& box) const;
			EmContainment::Type Contains(_In_ const OBB& box) const;
			EmContainment::Type Contains(_In_ const Frustum& fr) const;
			// Frustum-Frustum test

			bool Intersects(_In_ const Sphere& sh) const;
			bool Intersects(_In_ const AABB& box) const;
			bool Intersects(_In_ const OBB& box) const;
			bool Intersects(_In_ const Frustum& fr) const;

			bool __vectorcall Intersects(_In_ const Math::Vector3& V0, _In_ const Math::Vector3& V1, _In_ const Math::Vector3& V2) const;
			// Triangle-Frustum test

			EmPlaneIntersection::Type __vectorcall Intersects(_In_ const Math::Plane& Plane) const;
			// Plane-Frustum test

			bool __vectorcall Intersects(_In_ const Math::Vector3& rayOrigin, _In_ const Math::Vector3& Direction, _Out_ float& Dist) const;
			// Ray-Frustum test

			EmContainment::Type __vectorcall ContainedBy(_In_ const Math::Plane& Plane0, _In_ const Math::Plane& Plane1, _In_ const Math::Plane& Plane2,
				_In_ const Math::Plane& Plane3, _In_ const Math::Plane& Plane4, _In_ const Math::Plane& Plane5) const;
			// Test frustum against six planes (see Frustum::GetPlanes)

			void GetPlanes(_Out_opt_ Math::Plane* NearPlane, _Out_opt_ Math::Plane* FarPlane, _Out_opt_ Math::Plane* RightPlane,
				_Out_opt_ Math::Plane* LeftPlane, _Out_opt_ Math::Plane* TopPlane, _Out_opt_ Math::Plane* BottomPlane) const;
			// Create 6 Planes representation of Frustum

			// Static methods
			static void __vectorcall CreateFromMatrix(_Out_ Frustum& Out, _In_ const Math::Matrix& Projection);
		};

		//------------------------------------------------------------------------------
		// Ray
		class Ray
		{
		public:
			Math::Vector3 position;
			Math::Vector3 direction;

			Ray();
			Ray(const Math::Vector3& pos, const Math::Vector3& dir);

			// Comparison operators
			bool operator == (const Ray& r) const;
			bool operator != (const Ray& r) const;

			// Ray operations
			bool Intersects(const Sphere& sphere, _Out_ float& Dist) const;
			bool Intersects(const AABB& box, _Out_ float& Dist) const;
			bool Intersects(const Math::Vector3& tri0, const Math::Vector3& tri1, const Math::Vector3& tri2, _Out_ float& Dist) const;
			bool Intersects(const Math::Plane& plane, _Out_ float& Dist) const;
		};

		//-----------------------------------------------------------------------------
		// Triangle intersection testing routines.
		//-----------------------------------------------------------------------------
		namespace TriangleTests
		{
			bool __vectorcall Intersects(_In_ const Math::Vector3& Origin, _In_ const Math::Vector3& Direction, _In_ const Math::Vector3& V0, _In_ const Math::Vector3& V1, _In_ const Math::Vector3& V2, _Out_ float& Dist);
			// Ray-Triangle

			bool __vectorcall Intersects(_In_ const Math::Vector3& A0, _In_ const Math::Vector3& A1, _In_ const Math::Vector3& A2, _In_ const Math::Vector3& B0, _In_ const Math::Vector3& B1, _In_ const Math::Vector3& B2);
			// Triangle-Triangle

			EmPlaneIntersection::Type __vectorcall Intersects(_In_ const Math::Vector3& V0, _In_ const Math::Vector3& V1, _In_ const Math::Vector3& V2, _In_ const Math::Plane& Plane);
			// Plane-Triangle

			EmContainment::Type __vectorcall ContainedBy(_In_ const Math::Vector3& V0, _In_ const Math::Vector3& V1, _In_ const Math::Vector3& V2,
				_In_ const Math::Vector3& Plane0, _In_ const Math::Vector3& Plane1, _In_ const Math::Vector3& Plane2,
				_In_ const Math::Vector3& Plane3, _In_ const Math::Vector3& Plane4, _In_ const Math::Vector3& Plane5);
			// Test a triangle against six planes at once (see Frustum::GetPlanes)
		};
	};
}

namespace std
{
	template<> struct less<EastEngine::Collision::Ray>
	{
		bool operator()(const EastEngine::Collision::Ray& R1, const EastEngine::Collision::Ray& R2) const
		{
			if (R1.position.x != R2.position.x) return R1.position.x < R2.position.x;
			if (R1.position.y != R2.position.y) return R1.position.y < R2.position.y;
			if (R1.position.z != R2.position.z) return R1.position.z < R2.position.z;

			if (R1.direction.x != R2.direction.x) return R1.direction.x < R2.direction.x;
			if (R1.direction.y != R2.direction.y) return R1.direction.y < R2.direction.y;
			if (R1.direction.z != R2.direction.z) return R1.direction.z < R2.direction.z;

			return false;
		}
	};
}