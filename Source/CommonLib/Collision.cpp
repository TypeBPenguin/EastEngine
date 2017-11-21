#include "stdafx.h"
#include "Collision.h"

#include <DirectXMath.h>
#include <DirectXCollision.h>

namespace EastEngine
{
	namespace Collision
	{
		Sphere::Sphere() {}
		Sphere::Sphere(_In_ const Math::Vector3& center, _In_ float radius)
			: Center(center), Radius(radius) {}
		Sphere::Sphere(_In_ const Sphere& sp)
			: Center(sp.Center), Radius(sp.Radius) {}

		void __vectorcall Sphere::Transform(_Out_ Sphere& Out, _In_ const Math::Matrix& matrix) const
		{
			using namespace DirectX;

			// Load the center of the sphere.
			XMVECTOR vCenter = Center;

			XMMATRIX M = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(&matrix));

			// Transform the center of the sphere.
			XMVECTOR C = XMVector3Transform(vCenter, M);

			XMVECTOR dX = XMVector3Dot(M.r[0], M.r[0]);
			XMVECTOR dY = XMVector3Dot(M.r[1], M.r[1]);
			XMVECTOR dZ = XMVector3Dot(M.r[2], M.r[2]);

			XMVECTOR d = XMVectorMax(dX, XMVectorMax(dY, dZ));

			// Store the center sphere.
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&Out.Center), C);

			// Scale the radius of the pshere.
			float Scale = sqrtf(XMVectorGetX(d));
			Out.Radius = Radius * Scale;
		}

		void __vectorcall Sphere::Transform(_Out_ Sphere& Out, _In_ float Scale, _In_ const Math::Quaternion& Rotation, _In_ const Math::Vector3& Translation) const
		{
			using namespace DirectX;

			XMVECTOR R = Rotation;
			XMVECTOR T = Translation;

			// Load the center of the sphere.
			XMVECTOR vCenter = Center;

			// Transform the center of the sphere.
			vCenter = XMVector3Rotate(vCenter * XMVectorReplicate(Scale), R) + T;

			// Store the center sphere.
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&Out.Center), vCenter);

			// Scale the radius of the pshere.
			Out.Radius = Radius * Scale;
		}

		EmContainment::Type __vectorcall Sphere::Contains(_In_ const Math::Vector3& Point) const
		{
			using namespace DirectX;

			XMVECTOR P = Point;

			XMVECTOR vCenter = Center;
			XMVECTOR vRadius = XMVectorReplicatePtr(&Radius);

			XMVECTOR DistanceSquared = XMVector3LengthSq(P - vCenter);
			XMVECTOR RadiusSquared = XMVectorMultiply(vRadius, vRadius);

			return XMVector3LessOrEqual(DistanceSquared, RadiusSquared) ? EmContainment::eContains : EmContainment::eDisjoint;
		}

		EmContainment::Type __vectorcall Sphere::Contains(_In_ const Math::Vector3& V0, _In_ const Math::Vector3& V1, _In_ const Math::Vector3& V2) const
		{
			using namespace DirectX;

			if (Intersects(V0, V1, V2) == false)
				return EmContainment::eDisjoint;

			XMVECTOR v0 = V0;
			XMVECTOR v1 = V1;
			XMVECTOR v2 = V2;

			XMVECTOR vCenter = Center;
			XMVECTOR vRadius = XMVectorReplicatePtr(&Radius);
			XMVECTOR RadiusSquared = XMVectorMultiply(vRadius, vRadius);

			XMVECTOR DistanceSquared = XMVector3LengthSq(v0 - vCenter);
			XMVECTOR Inside = XMVectorLessOrEqual(DistanceSquared, RadiusSquared);

			DistanceSquared = XMVector3LengthSq(v1 - vCenter);
			Inside = XMVectorAndInt(Inside, XMVectorLessOrEqual(DistanceSquared, RadiusSquared));

			DistanceSquared = XMVector3LengthSq(v2 - vCenter);
			Inside = XMVectorAndInt(Inside, XMVectorLessOrEqual(DistanceSquared, RadiusSquared));

			return (XMVector3EqualInt(Inside, XMVectorTrueInt())) ? EmContainment::eContains : EmContainment::eIntersects;
		}

		EmContainment::Type Sphere::Contains(_In_ const Sphere& sh) const
		{
			using namespace DirectX;

			XMVECTOR Center1 = Center;
			float r1 = Radius;

			XMVECTOR Center2 = sh.Center;
			float r2 = sh.Radius;

			XMVECTOR V = XMVectorSubtract(Center2, Center1);

			XMVECTOR Dist = XMVector3Length(V);

			float d = XMVectorGetX(Dist);

			return (r1 + r2 >= d) ? ((r1 - r2 >= d) ? EmContainment::eContains : EmContainment::eIntersects) : EmContainment::eDisjoint;
		}

		EmContainment::Type Sphere::Contains(_In_ const AABB& box) const
		{
			using namespace DirectX;

			if (box.Intersects(*this) == false)
				return EmContainment::eDisjoint;

			XMVECTOR vCenter = Center;
			XMVECTOR vRadius = XMVectorReplicatePtr(&Radius);
			XMVECTOR RadiusSq = vRadius * vRadius;

			XMVECTOR boxCenter = box.Center;
			XMVECTOR boxExtents = box.Extents;

			XMVECTOR InsideAll = XMVectorTrueInt();

			XMVECTOR offset = boxCenter - vCenter;

			for (size_t i = 0; i < BoundingBox::CORNER_COUNT; ++i)
			{
				XMVECTOR C = XMVectorMultiplyAdd(boxExtents, g_BoxOffset[i], offset);
				XMVECTOR d = XMVector3LengthSq(C);
				InsideAll = XMVectorAndInt(InsideAll, XMVectorLessOrEqual(d, RadiusSq));
			}

			return (XMVector3EqualInt(InsideAll, XMVectorTrueInt())) ? EmContainment::eContains : EmContainment::eIntersects;
		}

		EmContainment::Type Sphere::Contains(_In_ const OBB& box) const
		{
			using namespace DirectX;

			if (box.Intersects(*this) == false)
				return EmContainment::eDisjoint;

			XMVECTOR vCenter = Center;
			XMVECTOR vRadius = XMVectorReplicatePtr(&Radius);
			XMVECTOR RadiusSq = vRadius * vRadius;

			XMVECTOR boxCenter = box.Center;
			XMVECTOR boxExtents = box.Extents;
			XMVECTOR boxOrientation = box.Orientation;

			assert(DirectX::Internal::XMQuaternionIsUnit(boxOrientation));

			XMVECTOR InsideAll = XMVectorTrueInt();

			for (size_t i = 0; i < BoundingOrientedBox::CORNER_COUNT; ++i)
			{
				XMVECTOR C = XMVector3Rotate(boxExtents * g_BoxOffset[i], boxOrientation) + boxCenter;
				XMVECTOR d = XMVector3LengthSq(XMVectorSubtract(vCenter, C));
				InsideAll = XMVectorAndInt(InsideAll, XMVectorLessOrEqual(d, RadiusSq));
			}

			return (XMVector3EqualInt(InsideAll, XMVectorTrueInt())) ? EmContainment::eContains : EmContainment::eIntersects;
		}

		EmContainment::Type Sphere::Contains(_In_ const Frustum& fr) const
		{
			using namespace DirectX;

			if (fr.Intersects(*this) == false)
				return EmContainment::eDisjoint;

			XMVECTOR vCenter = Center;
			XMVECTOR vRadius = XMVectorReplicatePtr(&Radius);
			XMVECTOR RadiusSq = vRadius * vRadius;

			XMVECTOR vOrigin = fr.Origin;
			XMVECTOR vOrientation = fr.Orientation;

			assert(DirectX::Internal::XMQuaternionIsUnit(vOrientation));

			// Build the corners of the frustum.
			XMVECTOR vRightTop = XMVectorSet(fr.RightSlope, fr.TopSlope, 1.0f, 0.0f);
			XMVECTOR vRightBottom = XMVectorSet(fr.RightSlope, fr.BottomSlope, 1.0f, 0.0f);
			XMVECTOR vLeftTop = XMVectorSet(fr.LeftSlope, fr.TopSlope, 1.0f, 0.0f);
			XMVECTOR vLeftBottom = XMVectorSet(fr.LeftSlope, fr.BottomSlope, 1.0f, 0.0f);
			XMVECTOR vNear = XMVectorReplicatePtr(&fr.Near);
			XMVECTOR vFar = XMVectorReplicatePtr(&fr.Far);

			XMVECTOR Corners[BoundingFrustum::CORNER_COUNT];
			Corners[0] = vRightTop * vNear;
			Corners[1] = vRightBottom * vNear;
			Corners[2] = vLeftTop * vNear;
			Corners[3] = vLeftBottom * vNear;
			Corners[4] = vRightTop * vFar;
			Corners[5] = vRightBottom * vFar;
			Corners[6] = vLeftTop * vFar;
			Corners[7] = vLeftBottom * vFar;

			XMVECTOR InsideAll = XMVectorTrueInt();
			for (size_t i = 0; i < BoundingFrustum::CORNER_COUNT; ++i)
			{
				XMVECTOR C = XMVector3Rotate(Corners[i], vOrientation) + vOrigin;
				XMVECTOR d = XMVector3LengthSq(XMVectorSubtract(vCenter, C));
				InsideAll = XMVectorAndInt(InsideAll, XMVectorLessOrEqual(d, RadiusSq));
			}

			return (XMVector3EqualInt(InsideAll, XMVectorTrueInt())) ? EmContainment::eContains : EmContainment::eIntersects;
		}

		bool Sphere::Intersects(_In_ const Sphere& sh) const
		{
			using namespace DirectX;

			// Load A.
			XMVECTOR vCenterA = Center;
			XMVECTOR vRadiusA = XMVectorReplicatePtr(&Radius);

			// Load B.
			XMVECTOR vCenterB = sh.Center;
			XMVECTOR vRadiusB = XMVectorReplicatePtr(&sh.Radius);

			// Distance squared between centers.    
			XMVECTOR Delta = vCenterB - vCenterA;
			XMVECTOR DistanceSquared = XMVector3LengthSq(Delta);

			// Sum of the radii squared.
			XMVECTOR RadiusSquared = XMVectorAdd(vRadiusA, vRadiusB);
			RadiusSquared = XMVectorMultiply(RadiusSquared, RadiusSquared);

			return XMVector3LessOrEqual(DistanceSquared, RadiusSquared);
		}

		bool Sphere::Intersects(_In_ const AABB& box) const { return box.Intersects(*this); }
		bool Sphere::Intersects(_In_ const OBB& box) const { return box.Intersects(*this); }
		bool Sphere::Intersects(_In_ const Frustum& fr) const { return fr.Intersects(*this); }

		bool __vectorcall Sphere::Intersects(_In_ const Math::Vector3& v0, _In_ const Math::Vector3& v1, _In_ const Math::Vector3& v2) const
		{
			using namespace DirectX;

			XMVECTOR V0 = v0;
			XMVECTOR V1 = v1;
			XMVECTOR V2 = v2;

			// Load the sphere.    
			XMVECTOR vCenter = Center;
			XMVECTOR vRadius = XMVectorReplicatePtr(&Radius);

			// Compute the plane of the triangle (has to be normalized).
			XMVECTOR N = XMVector3Normalize(XMVector3Cross(V1 - V0, V2 - V0));

			// Assert that the triangle is not degenerate.
			assert(!XMVector3Equal(N, XMVectorZero()));

			// Find the nearest feature on the triangle to the sphere.
			XMVECTOR Dist = XMVector3Dot(vCenter - V0, N);

			// If the center of the sphere is farther from the plane of the triangle than
			// the radius of the sphere, then there cannot be an intersection.
			XMVECTOR NoIntersection = XMVectorLess(Dist, -vRadius);
			NoIntersection = XMVectorOrInt(NoIntersection, XMVectorGreater(Dist, vRadius));

			// Project the center of the sphere onto the plane of the triangle.
			XMVECTOR Point = vCenter - (N * Dist);

			// Is it inside all the edges? If so we intersect because the distance 
			// to the plane is less than the radius.
			XMVECTOR Intersection = DirectX::Internal::PointOnPlaneInsideTriangle(Point, V0, V1, V2);

			// Find the nearest point on each edge.
			XMVECTOR RadiusSq = vRadius * vRadius;

			// Edge 0,1
			Point = DirectX::Internal::PointOnLineSegmentNearestPoint(V0, V1, vCenter);

			// If the distance to the center of the sphere to the point is less than 
			// the radius of the sphere then it must intersect.
			Intersection = XMVectorOrInt(Intersection, XMVectorLessOrEqual(XMVector3LengthSq(vCenter - Point), RadiusSq));

			// Edge 1,2
			Point = DirectX::Internal::PointOnLineSegmentNearestPoint(V1, V2, vCenter);

			// If the distance to the center of the sphere to the point is less than 
			// the radius of the sphere then it must intersect.
			Intersection = XMVectorOrInt(Intersection, XMVectorLessOrEqual(XMVector3LengthSq(vCenter - Point), RadiusSq));

			// Edge 2,0
			Point = DirectX::Internal::PointOnLineSegmentNearestPoint(V2, V0, vCenter);

			// If the distance to the center of the sphere to the point is less than 
			// the radius of the sphere then it must intersect.
			Intersection = XMVectorOrInt(Intersection, XMVectorLessOrEqual(XMVector3LengthSq(vCenter - Point), RadiusSq));

			return XMVector4EqualInt(XMVectorAndCInt(Intersection, NoIntersection), XMVectorTrueInt());
		}

		EmPlaneIntersection::Type __vectorcall Sphere::Intersects(_In_ Math::Plane& Plane) const
		{
			using namespace DirectX;

			XMVECTOR P = Plane;

			assert(DirectX::Internal::XMPlaneIsUnit(P));

			// Load the sphere.
			XMVECTOR vCenter = Center;
			XMVECTOR vRadius = XMVectorReplicatePtr(&Radius);

			// Set w of the center to one so we can dot4 with a plane.
			vCenter = XMVectorInsert<0, 0, 0, 0, 1>(vCenter, XMVectorSplatOne());

			XMVECTOR Outside, Inside;
			DirectX::Internal::FastIntersectSpherePlane(vCenter, vRadius, P, Outside, Inside);

			// If the sphere is outside any plane it is outside.
			if (XMVector4EqualInt(Outside, XMVectorTrueInt()))
				return EmPlaneIntersection::eFront;

			// If the sphere is inside all planes it is inside.
			if (XMVector4EqualInt(Inside, XMVectorTrueInt()))
				return EmPlaneIntersection::eBack;

			// The sphere is not inside all planes or outside a plane it intersects.
			return EmPlaneIntersection::eIntersecting;
		}

		bool __vectorcall Sphere::Intersects(_In_ const Math::Vector3& Origin, _In_ const Math::Vector3& Direction, _Out_ float& Dist) const
		{
			using namespace DirectX;

			XMVECTOR vOrigin = Origin;
			XMVECTOR vDirection = Direction;

			assert(DirectX::Internal::XMVector3IsUnit(vDirection));

			XMVECTOR vCenter = Center;
			XMVECTOR vRadius = XMVectorReplicatePtr(&Radius);

			// l is the vector from the ray origin to the center of the sphere.
			XMVECTOR l = vCenter - vOrigin;

			// s is the projection of the l onto the ray direction.
			XMVECTOR s = XMVector3Dot(l, vDirection);

			XMVECTOR l2 = XMVector3Dot(l, l);

			XMVECTOR r2 = vRadius * vRadius;

			// m2 is squared distance from the center of the sphere to the projection.
			XMVECTOR m2 = l2 - s * s;

			XMVECTOR NoIntersection;

			// If the ray origin is outside the sphere and the center of the sphere is 
			// behind the ray origin there is no intersection.
			NoIntersection = XMVectorAndInt(XMVectorLess(s, XMVectorZero()), XMVectorGreater(l2, r2));

			// If the squared distance from the center of the sphere to the projection
			// is greater than the radius squared the ray will miss the sphere.
			NoIntersection = XMVectorOrInt(NoIntersection, XMVectorGreater(m2, r2));

			// The ray hits the sphere, compute the nearest intersection point.
			XMVECTOR q = XMVectorSqrt(r2 - m2);
			XMVECTOR t1 = s - q;
			XMVECTOR t2 = s + q;

			XMVECTOR OriginInside = XMVectorLessOrEqual(l2, r2);
			XMVECTOR t = XMVectorSelect(t1, t2, OriginInside);

			if (XMVector4NotEqualInt(NoIntersection, XMVectorTrueInt()))
			{
				// Store the x-component to *pDist.
				XMStoreFloat(&Dist, t);
				return true;
			}

			Dist = 0.f;
			return false;
		}

		EmContainment::Type __vectorcall Sphere::ContainedBy(_In_ const Math::Plane& Plane0, _In_ const Math::Plane& Plane1, _In_ const Math::Plane& Plane2,
			_In_ const Math::Plane& Plane3, _In_ const Math::Plane& Plane4, _In_ const Math::Plane& Plane5) const
		{
			using namespace DirectX;

			XMVECTOR P0 = Plane0;
			XMVECTOR P1 = Plane1;
			XMVECTOR P2 = Plane2;
			XMVECTOR P3 = Plane3;
			XMVECTOR P4 = Plane4;
			XMVECTOR P5 = Plane5;

			// Load the sphere.
			XMVECTOR vCenter = Center;
			XMVECTOR vRadius = XMVectorReplicatePtr(&Radius);

			// Set w of the center to one so we can dot4 with a plane.
			vCenter = XMVectorInsert<0, 0, 0, 0, 1>(vCenter, XMVectorSplatOne());

			XMVECTOR Outside, Inside;

			// Test against each plane.
			DirectX::Internal::FastIntersectSpherePlane(vCenter, vRadius, P0, Outside, Inside);

			XMVECTOR AnyOutside = Outside;
			XMVECTOR AllInside = Inside;

			DirectX::Internal::FastIntersectSpherePlane(vCenter, vRadius, P1, Outside, Inside);
			AnyOutside = XMVectorOrInt(AnyOutside, Outside);
			AllInside = XMVectorAndInt(AllInside, Inside);

			DirectX::Internal::FastIntersectSpherePlane(vCenter, vRadius, P2, Outside, Inside);
			AnyOutside = XMVectorOrInt(AnyOutside, Outside);
			AllInside = XMVectorAndInt(AllInside, Inside);

			DirectX::Internal::FastIntersectSpherePlane(vCenter, vRadius, P3, Outside, Inside);
			AnyOutside = XMVectorOrInt(AnyOutside, Outside);
			AllInside = XMVectorAndInt(AllInside, Inside);

			DirectX::Internal::FastIntersectSpherePlane(vCenter, vRadius, P4, Outside, Inside);
			AnyOutside = XMVectorOrInt(AnyOutside, Outside);
			AllInside = XMVectorAndInt(AllInside, Inside);

			DirectX::Internal::FastIntersectSpherePlane(vCenter, vRadius, P5, Outside, Inside);
			AnyOutside = XMVectorOrInt(AnyOutside, Outside);
			AllInside = XMVectorAndInt(AllInside, Inside);

			// If the sphere is outside any plane it is outside.
			if (XMVector4EqualInt(AnyOutside, XMVectorTrueInt()))
				return EmContainment::eDisjoint;

			// If the sphere is inside all planes it is inside.
			if (XMVector4EqualInt(AllInside, XMVectorTrueInt()))
				return EmContainment::eContains;

			// The sphere is not inside all planes or outside a plane, it may intersect.
			return EmContainment::eIntersects;
		}
		// Test sphere against six planes (see Frustum::GetPlanes)

		// Static methods
		void Sphere::CreateMerged(_Out_ Sphere& Out, _In_ const Sphere& S1, _In_ const Sphere& S2)
		{
			using namespace DirectX;

			XMVECTOR Center1 = S1.Center;
			float r1 = S1.Radius;

			XMVECTOR Center2 = S2.Center;
			float r2 = S2.Radius;

			XMVECTOR V = XMVectorSubtract(Center2, Center1);

			XMVECTOR Dist = XMVector3Length(V);

			float d = XMVectorGetX(Dist);

			if (r1 + r2 >= d)
			{
				if (r1 - r2 >= d)
				{
					Out = S1;
					return;
				}
				else if (r2 - r1 >= d)
				{
					Out = S2;
					return;
				}
			}

			XMVECTOR N = XMVectorDivide(V, Dist);

			float t1 = XMMin(-r1, d - r2);
			float t2 = XMMax(r1, d + r2);
			float t_5 = (t2 - t1) * 0.5f;

			XMVECTOR NCenter = XMVectorAdd(Center1, XMVectorMultiply(N, XMVectorReplicate(t_5 + t1)));

			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&Out.Center), NCenter);
			Out.Radius = t_5;
		}

		void Sphere::CreateFromAABB(_Out_ Sphere& Out, _In_ const AABB& box)
		{
			using namespace DirectX;

			Out.Center = box.Center;
			XMVECTOR vExtents = box.Extents;
			Out.Radius = XMVectorGetX(XMVector3Length(vExtents));
		}

		void Sphere::CreateFromAABB(_Out_ Sphere& Out, _In_ const OBB& box)
		{
			using namespace DirectX;

			// Bounding box orientation is irrelevant because a sphere is rotationally invariant
			Out.Center = box.Center;
			XMVECTOR vExtents = box.Extents;
			Out.Radius = XMVectorGetX(XMVector3Length(vExtents));
		}

		void Sphere::CreateFromPoints(_Out_ Sphere& Out, _In_ size_t Count,
			_In_reads_bytes_(sizeof(Math::Vector3) + Stride*(Count - 1)) const Math::Vector3* pPoints, _In_ size_t Stride)
		{
			using namespace DirectX;

			assert(Count > 0);
			assert(pPoints);

			// Find the points with minimum and maximum x, y, and z
			XMVECTOR MinX, MaxX, MinY, MaxY, MinZ, MaxZ;

			MinX = MaxX = MinY = MaxY = MinZ = MaxZ = XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(pPoints));

			for (size_t i = 1; i < Count; ++i)
			{
				XMVECTOR Point = XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(reinterpret_cast<const uint8_t*>(pPoints) + i * Stride));

				float px = XMVectorGetX(Point);
				float py = XMVectorGetY(Point);
				float pz = XMVectorGetZ(Point);

				if (px < XMVectorGetX(MinX))
					MinX = Point;

				if (px > XMVectorGetX(MaxX))
					MaxX = Point;

				if (py < XMVectorGetY(MinY))
					MinY = Point;

				if (py > XMVectorGetY(MaxY))
					MaxY = Point;

				if (pz < XMVectorGetZ(MinZ))
					MinZ = Point;

				if (pz > XMVectorGetZ(MaxZ))
					MaxZ = Point;
			}

			// Use the min/max pair that are farthest apart to form the initial sphere.
			XMVECTOR DeltaX = MaxX - MinX;
			XMVECTOR DistX = XMVector3Length(DeltaX);

			XMVECTOR DeltaY = MaxY - MinY;
			XMVECTOR DistY = XMVector3Length(DeltaY);

			XMVECTOR DeltaZ = MaxZ - MinZ;
			XMVECTOR DistZ = XMVector3Length(DeltaZ);

			XMVECTOR vCenter;
			XMVECTOR vRadius;

			if (XMVector3Greater(DistX, DistY))
			{
				if (XMVector3Greater(DistX, DistZ))
				{
					// Use min/max x.
					vCenter = XMVectorLerp(MaxX, MinX, 0.5f);
					vRadius = DistX * 0.5f;
				}
				else
				{
					// Use min/max z.
					vCenter = XMVectorLerp(MaxZ, MinZ, 0.5f);
					vRadius = DistZ * 0.5f;
				}
			}
			else // Y >= X
			{
				if (XMVector3Greater(DistY, DistZ))
				{
					// Use min/max y.
					vCenter = XMVectorLerp(MaxY, MinY, 0.5f);
					vRadius = DistY * 0.5f;
				}
				else
				{
					// Use min/max z.
					vCenter = XMVectorLerp(MaxZ, MinZ, 0.5f);
					vRadius = DistZ * 0.5f;
				}
			}

			// Add any points not inside the sphere.
			for (size_t i = 0; i < Count; ++i)
			{
				XMVECTOR Point = XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(reinterpret_cast<const uint8_t*>(pPoints) + i * Stride));

				XMVECTOR Delta = Point - vCenter;

				XMVECTOR Dist = XMVector3Length(Delta);

				if (XMVector3Greater(Dist, vRadius))
				{
					// Adjust sphere to include the new point.
					vRadius = (vRadius + Dist) * 0.5f;
					vCenter += (XMVectorReplicate(1.0f) - XMVectorDivide(vRadius, Dist)) * Delta;
				}
			}

			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&Out.Center), vCenter);
			XMStoreFloat(&Out.Radius, vRadius);
		}

		void Sphere::CreateFromFrustum(_Out_ Sphere& Out, _In_ const Frustum& fr)
		{
			using namespace DirectX;

			Math::Vector3 Corners[Frustum::CORNER_COUNT];
			fr.GetCorners(Corners);

			CreateFromPoints(Out, Frustum::CORNER_COUNT, Corners, sizeof(Math::Vector3));
		}

		AABB::AABB() {}
		AABB::AABB(_In_ const Math::Vector3& center, _In_ const Math::Vector3& extents)
			: Center(center), Extents(extents) {}
		AABB::AABB(_In_ const AABB& box)
			: Center(box.Center), Extents(box.Extents) {}

		void __vectorcall AABB::Transform(_Out_ AABB& Out, _In_ const Math::Matrix& matrix) const
		{
			using namespace DirectX;

			XMMATRIX M = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(&matrix));

			// Load center and extents.
			XMVECTOR vCenter = Center;
			XMVECTOR vExtents = Extents;

			// Compute and transform the corners and find new min/max bounds.
			XMVECTOR Corner = XMVectorMultiplyAdd(vExtents, g_BoxOffset[0], vCenter);
			Corner = XMVector3Transform(Corner, M);

			XMVECTOR Min, Max;
			Min = Max = Corner;

			for (size_t i = 1; i < CORNER_COUNT; ++i)
			{
				Corner = XMVectorMultiplyAdd(vExtents, g_BoxOffset[i], vCenter);
				Corner = XMVector3Transform(Corner, M);

				Min = XMVectorMin(Min, Corner);
				Max = XMVectorMax(Max, Corner);
			}

			// Store center and extents.
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&Out.Center), (Min + Max) * 0.5f);
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&Out.Extents), (Max - Min) * 0.5f);
		}

		void __vectorcall AABB::Transform(_Out_ AABB& Out, _In_ float Scale, _In_ const Math::Quaternion& Rotation, _In_ const Math::Vector3& Translation) const
		{
			using namespace DirectX;

			XMVECTOR R = Rotation;
			XMVECTOR T = Translation;

			assert(DirectX::Internal::XMQuaternionIsUnit(R));

			// Load center and extents.
			XMVECTOR vCenter = Center;
			XMVECTOR vExtents = Extents;

			XMVECTOR VectorScale = XMVectorReplicate(Scale);

			// Compute and transform the corners and find new min/max bounds.
			XMVECTOR Corner = XMVectorMultiplyAdd(vExtents, g_BoxOffset[0], vCenter);
			Corner = XMVector3Rotate(Corner * VectorScale, R) + T;

			XMVECTOR Min, Max;
			Min = Max = Corner;

			for (size_t i = 1; i < CORNER_COUNT; ++i)
			{
				Corner = XMVectorMultiplyAdd(vExtents, g_BoxOffset[i], vCenter);
				Corner = XMVector3Rotate(Corner * VectorScale, R) + T;

				Min = XMVectorMin(Min, Corner);
				Max = XMVectorMax(Max, Corner);
			}

			// Store center and extents.
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&Out.Center), (Min + Max) * 0.5f);
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&Out.Extents), (Max - Min) * 0.5f);
		}

		void AABB::GetCorners(_Out_writes_(8) Math::Vector3* Corners) const
		{
			using namespace DirectX;

			assert(Corners != nullptr);

			// Load the box
			XMVECTOR vCenter = Center;
			XMVECTOR vExtents = Extents;

			for (size_t i = 0; i < CORNER_COUNT; ++i)
			{
				XMVECTOR C = XMVectorMultiplyAdd(vExtents, g_BoxOffset[i], vCenter);
				XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&Corners[i]), C);
			}
		}

		EmContainment::Type __vectorcall AABB::Contains(_In_ const Math::Vector3& Point) const
		{
			using namespace DirectX;

			XMVECTOR P = Point;
			XMVECTOR vCenter = Center;
			XMVECTOR vExtents = Extents;

			return XMVector3InBounds(P - vCenter, vExtents) ? EmContainment::eContains : EmContainment::eDisjoint;
		}

		EmContainment::Type __vectorcall AABB::Contains(_In_ const Math::Vector3& v0, _In_ const Math::Vector3& v1, _In_ const Math::Vector3& v2) const
		{
			using namespace DirectX;

			if (Intersects(v0, v1, v2) == false)
				return EmContainment::eDisjoint;

			XMVECTOR V0 = v0;
			XMVECTOR V1 = v1;
			XMVECTOR V2 = v2;

			XMVECTOR vCenter = Center;
			XMVECTOR vExtents = Extents;

			XMVECTOR d = XMVectorAbs(V0 - vCenter);
			XMVECTOR Inside = XMVectorLessOrEqual(d, vExtents);

			d = XMVectorAbs(V1 - vCenter);
			Inside = XMVectorAndInt(Inside, XMVectorLessOrEqual(d, vExtents));

			d = XMVectorAbs(V2 - vCenter);
			Inside = XMVectorAndInt(Inside, XMVectorLessOrEqual(d, vExtents));

			return (XMVector3EqualInt(Inside, XMVectorTrueInt())) ? EmContainment::eContains : EmContainment::eIntersects;
		}

		EmContainment::Type AABB::Contains(_In_ const Sphere& sh) const
		{
			using namespace DirectX;

			XMVECTOR SphereCenter = sh.Center;
			XMVECTOR SphereRadius = XMVectorReplicatePtr(&sh.Radius);

			XMVECTOR BoxCenter = Center;
			XMVECTOR BoxExtents = Extents;

			XMVECTOR BoxMin = BoxCenter - BoxExtents;
			XMVECTOR BoxMax = BoxCenter + BoxExtents;

			// Find the distance to the nearest point on the box.
			// for each i in (x, y, z)
			// if (SphereCenter(i) < BoxMin(i)) d2 += (SphereCenter(i) - BoxMin(i)) ^ 2
			// else if (SphereCenter(i) > BoxMax(i)) d2 += (SphereCenter(i) - BoxMax(i)) ^ 2

			XMVECTOR d = XMVectorZero();

			// Compute d for each dimension.
			XMVECTOR LessThanMin = XMVectorLess(SphereCenter, BoxMin);
			XMVECTOR GreaterThanMax = XMVectorGreater(SphereCenter, BoxMax);

			XMVECTOR MinDelta = SphereCenter - BoxMin;
			XMVECTOR MaxDelta = SphereCenter - BoxMax;

			// Choose value for each dimension based on the comparison.
			d = XMVectorSelect(d, MinDelta, LessThanMin);
			d = XMVectorSelect(d, MaxDelta, GreaterThanMax);

			// Use a dot-product to square them and sum them together.
			XMVECTOR d2 = XMVector3Dot(d, d);

			if (XMVector3Greater(d2, XMVectorMultiply(SphereRadius, SphereRadius)))
				return EmContainment::eDisjoint;

			XMVECTOR InsideAll = XMVectorLessOrEqual(BoxMin + SphereRadius, SphereCenter);
			InsideAll = XMVectorAndInt(InsideAll, XMVectorLessOrEqual(SphereCenter, BoxMax - SphereRadius));
			InsideAll = XMVectorAndInt(InsideAll, XMVectorGreater(BoxMax - BoxMin, SphereRadius));

			return (XMVector3EqualInt(InsideAll, XMVectorTrueInt())) ? EmContainment::eContains : EmContainment::eIntersects;
		}

		EmContainment::Type AABB::Contains(_In_ const AABB& box) const
		{
			using namespace DirectX;

			XMVECTOR CenterA = Center;
			XMVECTOR ExtentsA = Extents;

			XMVECTOR CenterB = box.Center;
			XMVECTOR ExtentsB = box.Extents;

			XMVECTOR MinA = CenterA - ExtentsA;
			XMVECTOR MaxA = CenterA + ExtentsA;

			XMVECTOR MinB = CenterB - ExtentsB;
			XMVECTOR MaxB = CenterB + ExtentsB;

			// for each i in (x, y, z) if a_min(i) > b_max(i) or b_min(i) > a_max(i) then return false
			XMVECTOR Disjoint = XMVectorOrInt(XMVectorGreater(MinA, MaxB), XMVectorGreater(MinB, MaxA));

			if (DirectX::Internal::XMVector3AnyTrue(Disjoint))
				return EmContainment::eDisjoint;

			// for each i in (x, y, z) if a_min(i) <= b_min(i) and b_max(i) <= a_max(i) then A contains B
			XMVECTOR Inside = XMVectorAndInt(XMVectorLessOrEqual(MinA, MinB), XMVectorLessOrEqual(MaxB, MaxA));

			return DirectX::Internal::XMVector3AllTrue(Inside) ? EmContainment::eContains : EmContainment::eIntersects;
		}

		EmContainment::Type AABB::Contains(_In_ const OBB& box) const
		{
			using namespace DirectX;

			if (box.Intersects(*this) == false)
				return EmContainment::eDisjoint;

			XMVECTOR vCenter = Center;
			XMVECTOR vExtents = Extents;

			// Subtract off the AABB center to remove a subtract below
			XMVECTOR oCenter = box.Center;
			oCenter -= vCenter;

			XMVECTOR oExtents = box.Extents;
			XMVECTOR oOrientation = box.Orientation;

			assert(DirectX::Internal::XMQuaternionIsUnit(oOrientation));

			XMVECTOR Inside = XMVectorTrueInt();

			for (size_t i = 0; i < BoundingOrientedBox::CORNER_COUNT; ++i)
			{
				XMVECTOR C = XMVector3Rotate(oExtents * g_BoxOffset[i], oOrientation) + oCenter;
				XMVECTOR d = XMVectorAbs(C);
				Inside = XMVectorAndInt(Inside, XMVectorLessOrEqual(d, vExtents));
			}

			return (XMVector3EqualInt(Inside, XMVectorTrueInt())) ? EmContainment::eContains : EmContainment::eIntersects;
		}

		EmContainment::Type AABB::Contains(_In_ const Frustum& fr) const
		{
			using namespace DirectX;

			if (fr.Intersects(*this) == false)
				return EmContainment::eDisjoint;

			Math::Vector3 Corners[Frustum::CORNER_COUNT];
			fr.GetCorners(Corners);

			XMVECTOR vCenter = Center;
			XMVECTOR vExtents = Extents;

			XMVECTOR Inside = XMVectorTrueInt();

			for (size_t i = 0; i < Frustum::CORNER_COUNT; ++i)
			{
				XMVECTOR Point = Corners[i];
				XMVECTOR d = XMVectorAbs(Point - vCenter);
				Inside = XMVectorAndInt(Inside, XMVectorLessOrEqual(d, vExtents));
			}

			return (XMVector3EqualInt(Inside, XMVectorTrueInt())) ? EmContainment::eContains : EmContainment::eIntersects;
		}

		bool AABB::Intersects(_In_ const Sphere& sh) const
		{
			using namespace DirectX;

			XMVECTOR SphereCenter = sh.Center;
			XMVECTOR SphereRadius = XMVectorReplicatePtr(&sh.Radius);

			XMVECTOR BoxCenter = Center;
			XMVECTOR BoxExtents = Extents;

			XMVECTOR BoxMin = BoxCenter - BoxExtents;
			XMVECTOR BoxMax = BoxCenter + BoxExtents;

			// Find the distance to the nearest point on the box.
			// for each i in (x, y, z)
			// if (SphereCenter(i) < BoxMin(i)) d2 += (SphereCenter(i) - BoxMin(i)) ^ 2
			// else if (SphereCenter(i) > BoxMax(i)) d2 += (SphereCenter(i) - BoxMax(i)) ^ 2

			XMVECTOR d = XMVectorZero();

			// Compute d for each dimension.
			XMVECTOR LessThanMin = XMVectorLess(SphereCenter, BoxMin);
			XMVECTOR GreaterThanMax = XMVectorGreater(SphereCenter, BoxMax);

			XMVECTOR MinDelta = SphereCenter - BoxMin;
			XMVECTOR MaxDelta = SphereCenter - BoxMax;

			// Choose value for each dimension based on the comparison.
			d = XMVectorSelect(d, MinDelta, LessThanMin);
			d = XMVectorSelect(d, MaxDelta, GreaterThanMax);

			// Use a dot-product to square them and sum them together.
			XMVECTOR d2 = XMVector3Dot(d, d);

			return XMVector3LessOrEqual(d2, XMVectorMultiply(SphereRadius, SphereRadius));
		}

		bool AABB::Intersects(_In_ const AABB& box) const
		{
			using namespace DirectX;

			XMVECTOR CenterA = Center;
			XMVECTOR ExtentsA = Extents;

			XMVECTOR CenterB = box.Center;
			XMVECTOR ExtentsB = box.Extents;

			XMVECTOR MinA = CenterA - ExtentsA;
			XMVECTOR MaxA = CenterA + ExtentsA;

			XMVECTOR MinB = CenterB - ExtentsB;
			XMVECTOR MaxB = CenterB + ExtentsB;

			// for each i in (x, y, z) if a_min(i) > b_max(i) or b_min(i) > a_max(i) then return false
			XMVECTOR Disjoint = XMVectorOrInt(XMVectorGreater(MinA, MaxB), XMVectorGreater(MinB, MaxA));

			return !DirectX::Internal::XMVector3AnyTrue(Disjoint);
		}

		bool AABB::Intersects(_In_ const OBB& box) const { return box.Intersects(*this); }
		bool AABB::Intersects(_In_ const Frustum& fr) const { return fr.Intersects(*this); }

		bool __vectorcall AABB::Intersects(_In_ const Math::Vector3& v0, _In_ const Math::Vector3& v1, _In_ const Math::Vector3& v2) const
		{
			using namespace DirectX;

			XMVECTOR V0 = v0;
			XMVECTOR V1 = v1;
			XMVECTOR V2 = v2;

			XMVECTOR Zero = XMVectorZero();

			// Load the box.
			XMVECTOR vCenter = Center;
			XMVECTOR vExtents = Extents;

			XMVECTOR BoxMin = vCenter - vExtents;
			XMVECTOR BoxMax = vCenter + vExtents;

			// Test the axes of the box (in effect test the AAB against the minimal AAB 
			// around the triangle).
			XMVECTOR TriMin = XMVectorMin(XMVectorMin(V0, V1), V2);
			XMVECTOR TriMax = XMVectorMax(XMVectorMax(V0, V1), V2);

			// for each i in (x, y, z) if a_min(i) > b_max(i) or b_min(i) > a_max(i) then disjoint
			XMVECTOR Disjoint = XMVectorOrInt(XMVectorGreater(TriMin, BoxMax), XMVectorGreater(BoxMin, TriMax));
			if (DirectX::Internal::XMVector3AnyTrue(Disjoint))
				return false;

			// Test the plane of the triangle.
			XMVECTOR Normal = XMVector3Cross(V1 - V0, V2 - V0);
			XMVECTOR Dist = XMVector3Dot(Normal, V0);

			// Assert that the triangle is not degenerate.
			assert(!XMVector3Equal(Normal, Zero));

			// for each i in (x, y, z) if n(i) >= 0 then v_min(i)=b_min(i), v_max(i)=b_max(i)
			// else v_min(i)=b_max(i), v_max(i)=b_min(i)
			XMVECTOR NormalSelect = XMVectorGreater(Normal, Zero);
			XMVECTOR V_Min = XMVectorSelect(BoxMax, BoxMin, NormalSelect);
			XMVECTOR V_Max = XMVectorSelect(BoxMin, BoxMax, NormalSelect);

			// if n dot v_min + d > 0 || n dot v_max + d < 0 then disjoint
			XMVECTOR MinDist = XMVector3Dot(V_Min, Normal);
			XMVECTOR MaxDist = XMVector3Dot(V_Max, Normal);

			XMVECTOR NoIntersection = XMVectorGreater(MinDist, Dist);
			NoIntersection = XMVectorOrInt(NoIntersection, XMVectorLess(MaxDist, Dist));

			// Move the box center to zero to simplify the following tests.
			XMVECTOR TV0 = V0 - vCenter;
			XMVECTOR TV1 = V1 - vCenter;
			XMVECTOR TV2 = V2 - vCenter;

			// Test the edge/edge axes (3*3).
			XMVECTOR e0 = TV1 - TV0;
			XMVECTOR e1 = TV2 - TV1;
			XMVECTOR e2 = TV0 - TV2;

			// Make w zero.
			e0 = XMVectorInsert<0, 0, 0, 0, 1>(e0, Zero);
			e1 = XMVectorInsert<0, 0, 0, 0, 1>(e1, Zero);
			e2 = XMVectorInsert<0, 0, 0, 0, 1>(e2, Zero);

			XMVECTOR Axis;
			XMVECTOR p0, p1, p2;
			XMVECTOR Min, Max;
			XMVECTOR Radius;

			// Axis == (1,0,0) x e0 = (0, -e0.z, e0.y)
			Axis = XMVectorPermute<XM_PERMUTE_0W, XM_PERMUTE_1Z, XM_PERMUTE_0Y, XM_PERMUTE_0X>(e0, -e0);
			p0 = XMVector3Dot(TV0, Axis);
			// p1 = XMVector3Dot( V1, Axis ); // p1 = p0;
			p2 = XMVector3Dot(TV2, Axis);
			Min = XMVectorMin(p0, p2);
			Max = XMVectorMax(p0, p2);
			Radius = XMVector3Dot(vExtents, XMVectorAbs(Axis));
			NoIntersection = XMVectorOrInt(NoIntersection, XMVectorGreater(Min, Radius));
			NoIntersection = XMVectorOrInt(NoIntersection, XMVectorLess(Max, -Radius));

			// Axis == (1,0,0) x e1 = (0, -e1.z, e1.y)
			Axis = XMVectorPermute<XM_PERMUTE_0W, XM_PERMUTE_1Z, XM_PERMUTE_0Y, XM_PERMUTE_0X>(e1, -e1);
			p0 = XMVector3Dot(TV0, Axis);
			p1 = XMVector3Dot(TV1, Axis);
			// p2 = XMVector3Dot( V2, Axis ); // p2 = p1;
			Min = XMVectorMin(p0, p1);
			Max = XMVectorMax(p0, p1);
			Radius = XMVector3Dot(vExtents, XMVectorAbs(Axis));
			NoIntersection = XMVectorOrInt(NoIntersection, XMVectorGreater(Min, Radius));
			NoIntersection = XMVectorOrInt(NoIntersection, XMVectorLess(Max, -Radius));

			// Axis == (1,0,0) x e2 = (0, -e2.z, e2.y)
			Axis = XMVectorPermute<XM_PERMUTE_0W, XM_PERMUTE_1Z, XM_PERMUTE_0Y, XM_PERMUTE_0X>(e2, -e2);
			p0 = XMVector3Dot(TV0, Axis);
			p1 = XMVector3Dot(TV1, Axis);
			// p2 = XMVector3Dot( V2, Axis ); // p2 = p0;
			Min = XMVectorMin(p0, p1);
			Max = XMVectorMax(p0, p1);
			Radius = XMVector3Dot(vExtents, XMVectorAbs(Axis));
			NoIntersection = XMVectorOrInt(NoIntersection, XMVectorGreater(Min, Radius));
			NoIntersection = XMVectorOrInt(NoIntersection, XMVectorLess(Max, -Radius));

			// Axis == (0,1,0) x e0 = (e0.z, 0, -e0.x)
			Axis = XMVectorPermute<XM_PERMUTE_0Z, XM_PERMUTE_0W, XM_PERMUTE_1X, XM_PERMUTE_0Y>(e0, -e0);
			p0 = XMVector3Dot(TV0, Axis);
			// p1 = XMVector3Dot( V1, Axis ); // p1 = p0;
			p2 = XMVector3Dot(TV2, Axis);
			Min = XMVectorMin(p0, p2);
			Max = XMVectorMax(p0, p2);
			Radius = XMVector3Dot(vExtents, XMVectorAbs(Axis));
			NoIntersection = XMVectorOrInt(NoIntersection, XMVectorGreater(Min, Radius));
			NoIntersection = XMVectorOrInt(NoIntersection, XMVectorLess(Max, -Radius));

			// Axis == (0,1,0) x e1 = (e1.z, 0, -e1.x)
			Axis = XMVectorPermute<XM_PERMUTE_0Z, XM_PERMUTE_0W, XM_PERMUTE_1X, XM_PERMUTE_0Y>(e1, -e1);
			p0 = XMVector3Dot(TV0, Axis);
			p1 = XMVector3Dot(TV1, Axis);
			// p2 = XMVector3Dot( V2, Axis ); // p2 = p1;
			Min = XMVectorMin(p0, p1);
			Max = XMVectorMax(p0, p1);
			Radius = XMVector3Dot(vExtents, XMVectorAbs(Axis));
			NoIntersection = XMVectorOrInt(NoIntersection, XMVectorGreater(Min, Radius));
			NoIntersection = XMVectorOrInt(NoIntersection, XMVectorLess(Max, -Radius));

			// Axis == (0,0,1) x e2 = (e2.z, 0, -e2.x)
			Axis = XMVectorPermute<XM_PERMUTE_0Z, XM_PERMUTE_0W, XM_PERMUTE_1X, XM_PERMUTE_0Y>(e2, -e2);
			p0 = XMVector3Dot(TV0, Axis);
			p1 = XMVector3Dot(TV1, Axis);
			// p2 = XMVector3Dot( V2, Axis ); // p2 = p0;
			Min = XMVectorMin(p0, p1);
			Max = XMVectorMax(p0, p1);
			Radius = XMVector3Dot(vExtents, XMVectorAbs(Axis));
			NoIntersection = XMVectorOrInt(NoIntersection, XMVectorGreater(Min, Radius));
			NoIntersection = XMVectorOrInt(NoIntersection, XMVectorLess(Max, -Radius));

			// Axis == (0,0,1) x e0 = (-e0.y, e0.x, 0)
			Axis = XMVectorPermute<XM_PERMUTE_1Y, XM_PERMUTE_0X, XM_PERMUTE_0W, XM_PERMUTE_0Z>(e0, -e0);
			p0 = XMVector3Dot(TV0, Axis);
			// p1 = XMVector3Dot( V1, Axis ); // p1 = p0;
			p2 = XMVector3Dot(TV2, Axis);
			Min = XMVectorMin(p0, p2);
			Max = XMVectorMax(p0, p2);
			Radius = XMVector3Dot(vExtents, XMVectorAbs(Axis));
			NoIntersection = XMVectorOrInt(NoIntersection, XMVectorGreater(Min, Radius));
			NoIntersection = XMVectorOrInt(NoIntersection, XMVectorLess(Max, -Radius));

			// Axis == (0,0,1) x e1 = (-e1.y, e1.x, 0)
			Axis = XMVectorPermute<XM_PERMUTE_1Y, XM_PERMUTE_0X, XM_PERMUTE_0W, XM_PERMUTE_0Z>(e1, -e1);
			p0 = XMVector3Dot(TV0, Axis);
			p1 = XMVector3Dot(TV1, Axis);
			// p2 = XMVector3Dot( V2, Axis ); // p2 = p1;
			Min = XMVectorMin(p0, p1);
			Max = XMVectorMax(p0, p1);
			Radius = XMVector3Dot(vExtents, XMVectorAbs(Axis));
			NoIntersection = XMVectorOrInt(NoIntersection, XMVectorGreater(Min, Radius));
			NoIntersection = XMVectorOrInt(NoIntersection, XMVectorLess(Max, -Radius));

			// Axis == (0,0,1) x e2 = (-e2.y, e2.x, 0)
			Axis = XMVectorPermute<XM_PERMUTE_1Y, XM_PERMUTE_0X, XM_PERMUTE_0W, XM_PERMUTE_0Z>(e2, -e2);
			p0 = XMVector3Dot(TV0, Axis);
			p1 = XMVector3Dot(TV1, Axis);
			// p2 = XMVector3Dot( V2, Axis ); // p2 = p0;
			Min = XMVectorMin(p0, p1);
			Max = XMVectorMax(p0, p1);
			Radius = XMVector3Dot(vExtents, XMVectorAbs(Axis));
			NoIntersection = XMVectorOrInt(NoIntersection, XMVectorGreater(Min, Radius));
			NoIntersection = XMVectorOrInt(NoIntersection, XMVectorLess(Max, -Radius));

			return XMVector4NotEqualInt(NoIntersection, XMVectorTrueInt());
		}

		EmPlaneIntersection::Type __vectorcall AABB::Intersects(_In_ const Math::Plane& Plane) const
		{
			using namespace DirectX;

			XMVECTOR P = Plane;
			assert(DirectX::Internal::XMPlaneIsUnit(P));

			// Load the box.
			XMVECTOR vCenter = Center;
			XMVECTOR vExtents = Extents;

			// Set w of the center to one so we can dot4 with a plane.
			vCenter = XMVectorInsert<0, 0, 0, 0, 1>(vCenter, XMVectorSplatOne());

			XMVECTOR Outside, Inside;
			DirectX::Internal::FastIntersectAxisAlignedBoxPlane(vCenter, vExtents, P, Outside, Inside);

			// If the box is outside any plane it is outside.
			if (XMVector4EqualInt(Outside, XMVectorTrueInt()))
				return EmPlaneIntersection::eFront;

			// If the box is inside all planes it is inside.
			if (XMVector4EqualInt(Inside, XMVectorTrueInt()))
				return EmPlaneIntersection::eBack;

			// The box is not inside all planes or outside a plane it intersects.
			return EmPlaneIntersection::eIntersecting;
		}

		bool __vectorcall AABB::Intersects(_In_ const Math::Vector3& Origin, _In_ const Math::Vector3& Direction, _Out_ float& Dist) const
		{
			using namespace DirectX;

			XMVECTOR vOrigin = Origin;
			XMVECTOR vDirection = Direction;
			assert(DirectX::Internal::XMVector3IsUnit(vDirection));

			// Load the box.
			XMVECTOR vCenter = Center;
			XMVECTOR vExtents = Extents;

			// Adjust ray origin to be relative to center of the box.
			XMVECTOR TOrigin = vCenter - vOrigin;

			// Compute the dot product againt each axis of the box.
			// Since the axii are (1,0,0), (0,1,0), (0,0,1) no computation is necessary.
			XMVECTOR AxisDotOrigin = TOrigin;
			XMVECTOR AxisDotDirection = vDirection;

			// if (fabs(AxisDotDirection) <= Epsilon) the ray is nearly parallel to the slab.
			XMVECTOR IsParallel = XMVectorLessOrEqual(XMVectorAbs(AxisDotDirection), g_RayEpsilon);

			// Test against all three axii simultaneously.
			XMVECTOR InverseAxisDotDirection = XMVectorReciprocal(AxisDotDirection);
			XMVECTOR t1 = (AxisDotOrigin - vExtents) * InverseAxisDotDirection;
			XMVECTOR t2 = (AxisDotOrigin + vExtents) * InverseAxisDotDirection;

			// Compute the max of min(t1,t2) and the min of max(t1,t2) ensuring we don't
			// use the results from any directions parallel to the slab.
			XMVECTOR t_min = XMVectorSelect(XMVectorMin(t1, t2), g_FltMin, IsParallel);
			XMVECTOR t_max = XMVectorSelect(XMVectorMax(t1, t2), g_FltMax, IsParallel);

			// t_min.x = maximum( t_min.x, t_min.y, t_min.z );
			// t_max.x = minimum( t_max.x, t_max.y, t_max.z );
			t_min = XMVectorMax(t_min, XMVectorSplatY(t_min));  // x = max(x,y)
			t_min = XMVectorMax(t_min, XMVectorSplatZ(t_min));  // x = max(max(x,y),z)
			t_max = XMVectorMin(t_max, XMVectorSplatY(t_max));  // x = min(x,y)
			t_max = XMVectorMin(t_max, XMVectorSplatZ(t_max));  // x = min(min(x,y),z)

																// if ( t_min > t_max ) return false;
			XMVECTOR NoIntersection = XMVectorGreater(XMVectorSplatX(t_min), XMVectorSplatX(t_max));

			// if ( t_max < 0.0f ) return false;
			NoIntersection = XMVectorOrInt(NoIntersection, XMVectorLess(XMVectorSplatX(t_max), XMVectorZero()));

			// if (IsParallel && (-Extents > AxisDotOrigin || Extents < AxisDotOrigin)) return false;
			XMVECTOR ParallelOverlap = XMVectorInBounds(AxisDotOrigin, vExtents);
			NoIntersection = XMVectorOrInt(NoIntersection, XMVectorAndCInt(IsParallel, ParallelOverlap));

			if (!DirectX::Internal::XMVector3AnyTrue(NoIntersection))
			{
				// Store the x-component to *pDist
				XMStoreFloat(&Dist, t_min);
				return true;
			}

			Dist = 0.f;
			return false;
		}

		EmContainment::Type __vectorcall AABB::ContainedBy(_In_ const Math::Plane& Plane0, _In_ const Math::Plane& Plane1, _In_ const Math::Plane& Plane2,
			_In_ const Math::Plane& Plane3, _In_ const Math::Plane& Plane4, _In_ const Math::Plane& Plane5) const
		{
			using namespace DirectX;

			XMVECTOR P0 = Plane0;
			XMVECTOR P1 = Plane1;
			XMVECTOR P2 = Plane2;
			XMVECTOR P3 = Plane3;
			XMVECTOR P4 = Plane4;
			XMVECTOR P5 = Plane5;

			// Load the box.
			XMVECTOR vCenter = Center;
			XMVECTOR vExtents = Extents;

			// Set w of the center to one so we can dot4 with a plane.
			vCenter = XMVectorInsert<0, 0, 0, 0, 1>(vCenter, XMVectorSplatOne());

			XMVECTOR Outside, Inside;

			// Test against each plane.
			DirectX::Internal::FastIntersectAxisAlignedBoxPlane(vCenter, vExtents, P0, Outside, Inside);

			XMVECTOR AnyOutside = Outside;
			XMVECTOR AllInside = Inside;

			DirectX::Internal::FastIntersectAxisAlignedBoxPlane(vCenter, vExtents, P1, Outside, Inside);
			AnyOutside = XMVectorOrInt(AnyOutside, Outside);
			AllInside = XMVectorAndInt(AllInside, Inside);

			DirectX::Internal::FastIntersectAxisAlignedBoxPlane(vCenter, vExtents, P2, Outside, Inside);
			AnyOutside = XMVectorOrInt(AnyOutside, Outside);
			AllInside = XMVectorAndInt(AllInside, Inside);

			DirectX::Internal::FastIntersectAxisAlignedBoxPlane(vCenter, vExtents, P3, Outside, Inside);
			AnyOutside = XMVectorOrInt(AnyOutside, Outside);
			AllInside = XMVectorAndInt(AllInside, Inside);

			DirectX::Internal::FastIntersectAxisAlignedBoxPlane(vCenter, vExtents, P4, Outside, Inside);
			AnyOutside = XMVectorOrInt(AnyOutside, Outside);
			AllInside = XMVectorAndInt(AllInside, Inside);

			DirectX::Internal::FastIntersectAxisAlignedBoxPlane(vCenter, vExtents, P5, Outside, Inside);
			AnyOutside = XMVectorOrInt(AnyOutside, Outside);
			AllInside = XMVectorAndInt(AllInside, Inside);

			// If the box is outside any plane it is outside.
			if (XMVector4EqualInt(AnyOutside, XMVectorTrueInt()))
				return EmContainment::eDisjoint;

			// If the box is inside all planes it is inside.
			if (XMVector4EqualInt(AllInside, XMVectorTrueInt()))
				return EmContainment::eContains;

			// The box is not inside all planes or outside a plane, it may intersect.
			return EmContainment::eIntersects;
		}

		// Static methods
		void AABB::CreateMerged(_Out_ AABB& Out, _In_ const AABB& b1, _In_ const AABB& b2)
		{
			using namespace DirectX;

			XMVECTOR b1Center = b1.Center;
			XMVECTOR b1Extents = b1.Extents;

			XMVECTOR b2Center = b2.Center;
			XMVECTOR b2Extents = b2.Extents;

			XMVECTOR Min = XMVectorSubtract(b1Center, b1Extents);
			Min = XMVectorMin(Min, XMVectorSubtract(b2Center, b2Extents));

			XMVECTOR Max = XMVectorAdd(b1Center, b1Extents);
			Max = XMVectorMax(Max, XMVectorAdd(b2Center, b2Extents));

			assert(XMVector3LessOrEqual(Min, Max));

			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&Out.Center), (Min + Max) * 0.5f);
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&Out.Extents), (Max - Min) * 0.5f);
		}

		void AABB::CreateFromSphere(_Out_ AABB& Out, _In_ const Sphere& sh)
		{
			using namespace DirectX;

			XMVECTOR spCenter = sh.Center;
			XMVECTOR shRadius = XMVectorReplicatePtr(&sh.Radius);

			XMVECTOR Min = XMVectorSubtract(spCenter, shRadius);
			XMVECTOR Max = XMVectorAdd(spCenter, shRadius);

			assert(XMVector3LessOrEqual(Min, Max));
			
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&Out.Center), (Min + Max) * 0.5f);
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&Out.Extents), (Max - Min) * 0.5f);
		}

		void __vectorcall AABB::CreateFromPoints(_Out_ AABB& Out, _In_ const Math::Vector3& pt1, _In_ const Math::Vector3& pt2)
		{
			using namespace DirectX;

			XMVECTOR t1 = pt1;
			XMVECTOR t2 = pt2;

			XMVECTOR Min = XMVectorMin(t1, t2);
			XMVECTOR Max = XMVectorMax(t1, t2);

			// Store center and extents.
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&Out.Center), (Min + Max) * 0.5f);
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&Out.Extents), (Max - Min) * 0.5f);
		}

		void AABB::CreateFromPoints(_Out_ AABB& Out, _In_ size_t Count,
			_In_reads_bytes_(sizeof(Math::Vector3) + Stride*(Count - 1)) const Math::Vector3* pPoints, _In_ size_t Stride)
		{
			using namespace DirectX;

			assert(Count > 0);
			assert(pPoints);

			// Find the minimum and maximum x, y, and z
			XMVECTOR vMin, vMax;

			vMin = vMax = XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(pPoints));

			for (size_t i = 1; i < Count; ++i)
			{
				XMVECTOR Point = XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(reinterpret_cast<const uint8_t*>(pPoints) + i * Stride));

				vMin = XMVectorMin(vMin, Point);
				vMax = XMVectorMax(vMax, Point);
			}

			// Store center and extents.
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&Out.Center), (vMin + vMax) * 0.5f);
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&Out.Extents), (vMax - vMin) * 0.5f);
		}

		OBB::OBB() {}
		OBB::OBB(_In_ const Math::Vector3& Center, _In_ const Math::Vector3& Extents, _In_ const Math::Quaternion& Orientation)
			: Center(Center), Extents(Extents), Orientation(Orientation) {}
		OBB::OBB(_In_ const OBB& box)
			: Center(box.Center), Extents(box.Extents), Orientation(box.Orientation) {}

		void __vectorcall OBB::Transform(_Out_ OBB& Out, _In_ const Math::Matrix& matrix) const
		{
			using namespace DirectX;

			XMMATRIX M = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(&matrix));

			// Load the box.
			XMVECTOR vCenter = Center;
			XMVECTOR vExtents = Extents;
			XMVECTOR vOrientation = Orientation;

			assert(DirectX::Internal::XMQuaternionIsUnit(vOrientation));

			// Composite the box rotation and the transform rotation.
			XMMATRIX nM;
			nM.r[0] = XMVector3Normalize(M.r[0]);
			nM.r[1] = XMVector3Normalize(M.r[1]);
			nM.r[2] = XMVector3Normalize(M.r[2]);
			nM.r[3] = g_XMIdentityR3;
			XMVECTOR Rotation = XMQuaternionRotationMatrix(nM);
			vOrientation = XMQuaternionMultiply(vOrientation, Rotation);

			// Transform the center.
			vCenter = XMVector3Transform(vCenter, M);

			// Scale the box extents.
			XMVECTOR dX = XMVector3Length(M.r[0]);
			XMVECTOR dY = XMVector3Length(M.r[1]);
			XMVECTOR dZ = XMVector3Length(M.r[2]);

			XMVECTOR VectorScale = XMVectorSelect(dX, dY, g_XMSelect1000);
			VectorScale = XMVectorSelect(VectorScale, dZ, g_XMSelect1100);
			vExtents = vExtents * VectorScale;

			// Store the box.
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&Out.Center), vCenter);
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&Out.Extents), vExtents);
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&Out.Orientation), vOrientation);
		}

		void __vectorcall OBB::Transform(_Out_ OBB& Out, _In_ float Scale, _In_ const Math::Quaternion& Rotation, _In_ const Math::Vector3& Translation) const
		{
			using namespace DirectX;

			// Load the box.
			XMVECTOR R = Rotation;
			XMVECTOR T = Translation;

			assert(DirectX::Internal::XMQuaternionIsUnit(R));

			// Load the box.
			XMVECTOR vCenter = Center;
			XMVECTOR vExtents = Extents;
			XMVECTOR vOrientation = Orientation;

			assert(DirectX::Internal::XMQuaternionIsUnit(vOrientation));

			// Composite the box rotation and the transform rotation.
			vOrientation = XMQuaternionMultiply(vOrientation, R);

			// Transform the center.
			XMVECTOR VectorScale = XMVectorReplicate(Scale);
			vCenter = XMVector3Rotate(vCenter * VectorScale, R) + T;

			// Scale the box extents.
			vExtents = vExtents * VectorScale;

			// Store the box.
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&Out.Center), vCenter);
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&Out.Extents), vExtents);
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&Out.Orientation), vOrientation);
		}

		void OBB::GetCorners(_Out_writes_(8) Math::Vector3* Corners) const
		{
			using namespace DirectX;

			assert(Corners != 0);

			// Load the box
			XMVECTOR vCenter = Center;
			XMVECTOR vExtents = Extents;
			XMVECTOR vOrientation = Orientation;

			assert(DirectX::Internal::XMQuaternionIsUnit(vOrientation));

			for (size_t i = 0; i < CORNER_COUNT; ++i)
			{
				XMVECTOR C = XMVector3Rotate(vExtents * g_BoxOffset[i], vOrientation) + vCenter;
				XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&Corners[i]), C);
			}
		}

		EmContainment::Type __vectorcall OBB::Contains(_In_ const Math::Vector3& Point) const
		{
			using namespace DirectX;

			XMVECTOR vPoint = Point;
			XMVECTOR vCenter = Center;
			XMVECTOR vExtents = Extents;
			XMVECTOR vOrientation = Orientation;

			// Transform the point to be local to the box.
			XMVECTOR TPoint = XMVector3InverseRotate(vPoint - vCenter, vOrientation);

			return XMVector3InBounds(TPoint, vExtents) ? EmContainment::eContains : EmContainment::eDisjoint;
		}

		EmContainment::Type __vectorcall OBB::Contains(_In_ const Math::Vector3& v0, _In_ const Math::Vector3& v1, _In_ const Math::Vector3& v2) const
		{
			using namespace DirectX;

			XMVECTOR V0 = v0;
			XMVECTOR V1 = v1;
			XMVECTOR V2 = v2;

			// Load the box center & orientation.
			XMVECTOR vCenter = Center;
			XMVECTOR vOrientation = Orientation;

			// Transform the triangle vertices into the space of the box.
			XMVECTOR TV0 = XMVector3InverseRotate(V0 - vCenter, vOrientation);
			XMVECTOR TV1 = XMVector3InverseRotate(V1 - vCenter, vOrientation);
			XMVECTOR TV2 = XMVector3InverseRotate(V2 - vCenter, vOrientation);

			AABB box;
			box.Center = Math::Vector3(0.0f, 0.0f, 0.0f);
			box.Extents = Extents;

			// Use the triangle vs axis aligned box intersection routine.
			return box.Contains(TV0, TV1, TV2);
		}

		EmContainment::Type OBB::Contains(_In_ const Sphere& sh) const
		{
			using namespace DirectX;

			XMVECTOR SphereCenter = sh.Center;
			XMVECTOR SphereRadius = XMVectorReplicatePtr(&sh.Radius);

			XMVECTOR BoxCenter = Center;
			XMVECTOR BoxExtents = Extents;
			XMVECTOR BoxOrientation = Orientation;

			assert(DirectX::Internal::XMQuaternionIsUnit(BoxOrientation));

			// Transform the center of the sphere to be local to the box.
			// BoxMin = -BoxExtents
			// BoxMax = +BoxExtents
			SphereCenter = XMVector3InverseRotate(SphereCenter - BoxCenter, BoxOrientation);

			// Find the distance to the nearest point on the box.
			// for each i in (x, y, z)
			// if (SphereCenter(i) < BoxMin(i)) d2 += (SphereCenter(i) - BoxMin(i)) ^ 2
			// else if (SphereCenter(i) > BoxMax(i)) d2 += (SphereCenter(i) - BoxMax(i)) ^ 2

			XMVECTOR d = XMVectorZero();

			// Compute d for each dimension.
			XMVECTOR LessThanMin = XMVectorLess(SphereCenter, -BoxExtents);
			XMVECTOR GreaterThanMax = XMVectorGreater(SphereCenter, BoxExtents);

			XMVECTOR MinDelta = SphereCenter + BoxExtents;
			XMVECTOR MaxDelta = SphereCenter - BoxExtents;

			// Choose value for each dimension based on the comparison.
			d = XMVectorSelect(d, MinDelta, LessThanMin);
			d = XMVectorSelect(d, MaxDelta, GreaterThanMax);

			// Use a dot-product to square them and sum them together.
			XMVECTOR d2 = XMVector3Dot(d, d);
			XMVECTOR SphereRadiusSq = XMVectorMultiply(SphereRadius, SphereRadius);

			if (XMVector4Greater(d2, SphereRadiusSq))
				return EmContainment::eDisjoint;

			// See if we are completely inside the box
			XMVECTOR SMin = SphereCenter - SphereRadius;
			XMVECTOR SMax = SphereCenter + SphereRadius;

			return (XMVector3InBounds(SMin, BoxExtents) && XMVector3InBounds(SMax, BoxExtents)) ? EmContainment::eContains : EmContainment::eIntersects;
		}

		EmContainment::Type OBB::Contains(_In_ const AABB& box) const
		{
			// Make the axis aligned box oriented and do an OBB vs OBB test.
			OBB obox(box.Center, box.Extents, Math::Quaternion::Identity);
			return Contains(obox);
		}

		EmContainment::Type OBB::Contains(_In_ const OBB& box) const
		{
			using namespace DirectX;

			if (Intersects(box) == false)
				return EmContainment::eDisjoint;

			// Load the boxes
			XMVECTOR aCenter = Center;
			XMVECTOR aExtents = Extents;
			XMVECTOR aOrientation = Orientation;

			assert(DirectX::Internal::XMQuaternionIsUnit(aOrientation));

			XMVECTOR bCenter = box.Center;
			XMVECTOR bExtents = box.Extents;
			XMVECTOR bOrientation = box.Orientation;

			assert(DirectX::Internal::XMQuaternionIsUnit(bOrientation));

			XMVECTOR offset = bCenter - aCenter;

			for (size_t i = 0; i < CORNER_COUNT; ++i)
			{
				// Cb = rotate( bExtents * corneroffset[i], bOrientation ) + bcenter
				// Ca = invrotate( Cb - aCenter, aOrientation )

				XMVECTOR C = XMVector3Rotate(bExtents * g_BoxOffset[i], bOrientation) + offset;
				C = XMVector3InverseRotate(C, aOrientation);

				if (XMVector3InBounds(C, aExtents) == false)
					return EmContainment::eIntersects;
			}

			return EmContainment::eContains;
		}

		EmContainment::Type OBB::Contains(_In_ const Frustum& fr) const
		{
			using namespace DirectX;

			if (fr.Intersects(*this) == false)
				return EmContainment::eDisjoint;

			Math::Vector3 Corners[Frustum::CORNER_COUNT];
			fr.GetCorners(Corners);

			// Load the box
			XMVECTOR vCenter = Center;
			XMVECTOR vExtents = Extents;
			XMVECTOR vOrientation = Orientation;

			assert(DirectX::Internal::XMQuaternionIsUnit(vOrientation));

			for (size_t i = 0; i < Frustum::CORNER_COUNT; ++i)
			{
				XMVECTOR C = XMVector3InverseRotate(XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(&Corners[i])) - vCenter, vOrientation);

				if (XMVector3InBounds(C, vExtents) == false)
					return EmContainment::eIntersects;
			}

			return EmContainment::eContains;
		}

		bool OBB::Intersects(_In_ const Sphere& sh) const
		{
			using namespace DirectX;

			XMVECTOR SphereCenter = sh.Center;
			XMVECTOR SphereRadius = XMVectorReplicatePtr(&sh.Radius);

			XMVECTOR BoxCenter = Center;
			XMVECTOR BoxExtents = Extents;
			XMVECTOR BoxOrientation = Orientation;

			assert(DirectX::Internal::XMQuaternionIsUnit(BoxOrientation));

			// Transform the center of the sphere to be local to the box.
			// BoxMin = -BoxExtents
			// BoxMax = +BoxExtents
			SphereCenter = XMVector3InverseRotate(SphereCenter - BoxCenter, BoxOrientation);

			// Find the distance to the nearest point on the box.
			// for each i in (x, y, z)
			// if (SphereCenter(i) < BoxMin(i)) d2 += (SphereCenter(i) - BoxMin(i)) ^ 2
			// else if (SphereCenter(i) > BoxMax(i)) d2 += (SphereCenter(i) - BoxMax(i)) ^ 2

			XMVECTOR d = XMVectorZero();

			// Compute d for each dimension.
			XMVECTOR LessThanMin = XMVectorLess(SphereCenter, -BoxExtents);
			XMVECTOR GreaterThanMax = XMVectorGreater(SphereCenter, BoxExtents);

			XMVECTOR MinDelta = SphereCenter + BoxExtents;
			XMVECTOR MaxDelta = SphereCenter - BoxExtents;

			// Choose value for each dimension based on the comparison.
			d = XMVectorSelect(d, MinDelta, LessThanMin);
			d = XMVectorSelect(d, MaxDelta, GreaterThanMax);

			// Use a dot-product to square them and sum them together.
			XMVECTOR d2 = XMVector3Dot(d, d);

			return XMVector4LessOrEqual(d2, XMVectorMultiply(SphereRadius, SphereRadius)) ? true : false;
		}

		bool OBB::Intersects(_In_ const AABB& box) const
		{
			// Make the axis aligned box oriented and do an OBB vs OBB test.
			OBB obox(box.Center, box.Extents, Math::Quaternion::Identity);
			return Intersects(obox);
		}

		bool OBB::Intersects(_In_ const OBB& box) const
		{
			using namespace DirectX;

			// Build the 3x3 rotation matrix that defines the orientation of B relative to A.
			XMVECTOR A_quat = Orientation;
			XMVECTOR B_quat = box.Orientation;

			assert(DirectX::Internal::XMQuaternionIsUnit(A_quat));
			assert(DirectX::Internal::XMQuaternionIsUnit(B_quat));

			XMVECTOR Q = XMQuaternionMultiply(A_quat, XMQuaternionConjugate(B_quat));
			XMMATRIX R = XMMatrixRotationQuaternion(Q);

			// Compute the translation of B relative to A.
			XMVECTOR A_cent = Center;
			XMVECTOR B_cent = box.Center;
			XMVECTOR t = XMVector3InverseRotate(B_cent - A_cent, A_quat);

			//
			// h(A) = extents of A.
			// h(B) = extents of B.
			//
			// a(u) = axes of A = (1,0,0), (0,1,0), (0,0,1)
			// b(u) = axes of B relative to A = (r00,r10,r20), (r01,r11,r21), (r02,r12,r22)
			//  
			// For each possible separating axis l:
			//   d(A) = sum (for i = u,v,w) h(A)(i) * abs( a(i) dot l )
			//   d(B) = sum (for i = u,v,w) h(B)(i) * abs( b(i) dot l )
			//   if abs( t dot l ) > d(A) + d(B) then disjoint
			//

			// Load extents of A and B.
			XMVECTOR h_A = Extents;
			XMVECTOR h_B = box.Extents;

			// Rows. Note R[0,1,2]X.w = 0.
			XMVECTOR R0X = R.r[0];
			XMVECTOR R1X = R.r[1];
			XMVECTOR R2X = R.r[2];

			R = XMMatrixTranspose(R);

			// Columns. Note RX[0,1,2].w = 0.
			XMVECTOR RX0 = R.r[0];
			XMVECTOR RX1 = R.r[1];
			XMVECTOR RX2 = R.r[2];

			// Absolute value of rows.
			XMVECTOR AR0X = XMVectorAbs(R0X);
			XMVECTOR AR1X = XMVectorAbs(R1X);
			XMVECTOR AR2X = XMVectorAbs(R2X);

			// Absolute value of columns.
			XMVECTOR ARX0 = XMVectorAbs(RX0);
			XMVECTOR ARX1 = XMVectorAbs(RX1);
			XMVECTOR ARX2 = XMVectorAbs(RX2);

			// Test each of the 15 possible seperating axii.
			XMVECTOR d, d_A, d_B;

			// l = a(u) = (1, 0, 0)
			// t dot l = t.x
			// d(A) = h(A).x
			// d(B) = h(B) dot abs(r00, r01, r02)
			d = XMVectorSplatX(t);
			d_A = XMVectorSplatX(h_A);
			d_B = XMVector3Dot(h_B, AR0X);
			XMVECTOR NoIntersection = XMVectorGreater(XMVectorAbs(d), XMVectorAdd(d_A, d_B));

			// l = a(v) = (0, 1, 0)
			// t dot l = t.y
			// d(A) = h(A).y
			// d(B) = h(B) dot abs(r10, r11, r12)
			d = XMVectorSplatY(t);
			d_A = XMVectorSplatY(h_A);
			d_B = XMVector3Dot(h_B, AR1X);
			NoIntersection = XMVectorOrInt(NoIntersection,
				XMVectorGreater(XMVectorAbs(d), XMVectorAdd(d_A, d_B)));

			// l = a(w) = (0, 0, 1)
			// t dot l = t.z
			// d(A) = h(A).z
			// d(B) = h(B) dot abs(r20, r21, r22)
			d = XMVectorSplatZ(t);
			d_A = XMVectorSplatZ(h_A);
			d_B = XMVector3Dot(h_B, AR2X);
			NoIntersection = XMVectorOrInt(NoIntersection,
				XMVectorGreater(XMVectorAbs(d), XMVectorAdd(d_A, d_B)));

			// l = b(u) = (r00, r10, r20)
			// d(A) = h(A) dot abs(r00, r10, r20)
			// d(B) = h(B).x
			d = XMVector3Dot(t, RX0);
			d_A = XMVector3Dot(h_A, ARX0);
			d_B = XMVectorSplatX(h_B);
			NoIntersection = XMVectorOrInt(NoIntersection,
				XMVectorGreater(XMVectorAbs(d), XMVectorAdd(d_A, d_B)));

			// l = b(v) = (r01, r11, r21)
			// d(A) = h(A) dot abs(r01, r11, r21)
			// d(B) = h(B).y
			d = XMVector3Dot(t, RX1);
			d_A = XMVector3Dot(h_A, ARX1);
			d_B = XMVectorSplatY(h_B);
			NoIntersection = XMVectorOrInt(NoIntersection,
				XMVectorGreater(XMVectorAbs(d), XMVectorAdd(d_A, d_B)));

			// l = b(w) = (r02, r12, r22)
			// d(A) = h(A) dot abs(r02, r12, r22)
			// d(B) = h(B).z
			d = XMVector3Dot(t, RX2);
			d_A = XMVector3Dot(h_A, ARX2);
			d_B = XMVectorSplatZ(h_B);
			NoIntersection = XMVectorOrInt(NoIntersection,
				XMVectorGreater(XMVectorAbs(d), XMVectorAdd(d_A, d_B)));

			// l = a(u) x b(u) = (0, -r20, r10)
			// d(A) = h(A) dot abs(0, r20, r10)
			// d(B) = h(B) dot abs(0, r02, r01)
			d = XMVector3Dot(t, XMVectorPermute<XM_PERMUTE_0W, XM_PERMUTE_1Z, XM_PERMUTE_0Y, XM_PERMUTE_0X>(RX0, -RX0));
			d_A = XMVector3Dot(h_A, XMVectorSwizzle<XM_SWIZZLE_W, XM_SWIZZLE_Z, XM_SWIZZLE_Y, XM_SWIZZLE_X>(ARX0));
			d_B = XMVector3Dot(h_B, XMVectorSwizzle<XM_SWIZZLE_W, XM_SWIZZLE_Z, XM_SWIZZLE_Y, XM_SWIZZLE_X>(AR0X));
			NoIntersection = XMVectorOrInt(NoIntersection,
				XMVectorGreater(XMVectorAbs(d), XMVectorAdd(d_A, d_B)));

			// l = a(u) x b(v) = (0, -r21, r11)
			// d(A) = h(A) dot abs(0, r21, r11)
			// d(B) = h(B) dot abs(r02, 0, r00)
			d = XMVector3Dot(t, XMVectorPermute<XM_PERMUTE_0W, XM_PERMUTE_1Z, XM_PERMUTE_0Y, XM_PERMUTE_0X>(RX1, -RX1));
			d_A = XMVector3Dot(h_A, XMVectorSwizzle<XM_SWIZZLE_W, XM_SWIZZLE_Z, XM_SWIZZLE_Y, XM_SWIZZLE_X>(ARX1));
			d_B = XMVector3Dot(h_B, XMVectorSwizzle<XM_SWIZZLE_Z, XM_SWIZZLE_W, XM_SWIZZLE_X, XM_SWIZZLE_Y>(AR0X));
			NoIntersection = XMVectorOrInt(NoIntersection,
				XMVectorGreater(XMVectorAbs(d), XMVectorAdd(d_A, d_B)));

			// l = a(u) x b(w) = (0, -r22, r12)
			// d(A) = h(A) dot abs(0, r22, r12)
			// d(B) = h(B) dot abs(r01, r00, 0)
			d = XMVector3Dot(t, XMVectorPermute<XM_PERMUTE_0W, XM_PERMUTE_1Z, XM_PERMUTE_0Y, XM_PERMUTE_0X>(RX2, -RX2));
			d_A = XMVector3Dot(h_A, XMVectorSwizzle<XM_SWIZZLE_W, XM_SWIZZLE_Z, XM_SWIZZLE_Y, XM_SWIZZLE_X>(ARX2));
			d_B = XMVector3Dot(h_B, XMVectorSwizzle<XM_SWIZZLE_Y, XM_SWIZZLE_X, XM_SWIZZLE_W, XM_SWIZZLE_Z>(AR0X));
			NoIntersection = XMVectorOrInt(NoIntersection,
				XMVectorGreater(XMVectorAbs(d), XMVectorAdd(d_A, d_B)));

			// l = a(v) x b(u) = (r20, 0, -r00)
			// d(A) = h(A) dot abs(r20, 0, r00)
			// d(B) = h(B) dot abs(0, r12, r11)
			d = XMVector3Dot(t, XMVectorPermute<XM_PERMUTE_0Z, XM_PERMUTE_0W, XM_PERMUTE_1X, XM_PERMUTE_0Y>(RX0, -RX0));
			d_A = XMVector3Dot(h_A, XMVectorSwizzle<XM_SWIZZLE_Z, XM_SWIZZLE_W, XM_SWIZZLE_X, XM_SWIZZLE_Y>(ARX0));
			d_B = XMVector3Dot(h_B, XMVectorSwizzle<XM_SWIZZLE_W, XM_SWIZZLE_Z, XM_SWIZZLE_Y, XM_SWIZZLE_X>(AR1X));
			NoIntersection = XMVectorOrInt(NoIntersection,
				XMVectorGreater(XMVectorAbs(d), XMVectorAdd(d_A, d_B)));

			// l = a(v) x b(v) = (r21, 0, -r01)
			// d(A) = h(A) dot abs(r21, 0, r01)
			// d(B) = h(B) dot abs(r12, 0, r10)
			d = XMVector3Dot(t, XMVectorPermute<XM_PERMUTE_0Z, XM_PERMUTE_0W, XM_PERMUTE_1X, XM_PERMUTE_0Y>(RX1, -RX1));
			d_A = XMVector3Dot(h_A, XMVectorSwizzle<XM_SWIZZLE_Z, XM_SWIZZLE_W, XM_SWIZZLE_X, XM_SWIZZLE_Y>(ARX1));
			d_B = XMVector3Dot(h_B, XMVectorSwizzle<XM_SWIZZLE_Z, XM_SWIZZLE_W, XM_SWIZZLE_X, XM_SWIZZLE_Y>(AR1X));
			NoIntersection = XMVectorOrInt(NoIntersection,
				XMVectorGreater(XMVectorAbs(d), XMVectorAdd(d_A, d_B)));

			// l = a(v) x b(w) = (r22, 0, -r02)
			// d(A) = h(A) dot abs(r22, 0, r02)
			// d(B) = h(B) dot abs(r11, r10, 0)
			d = XMVector3Dot(t, XMVectorPermute<XM_PERMUTE_0Z, XM_PERMUTE_0W, XM_PERMUTE_1X, XM_PERMUTE_0Y>(RX2, -RX2));
			d_A = XMVector3Dot(h_A, XMVectorSwizzle<XM_SWIZZLE_Z, XM_SWIZZLE_W, XM_SWIZZLE_X, XM_SWIZZLE_Y>(ARX2));
			d_B = XMVector3Dot(h_B, XMVectorSwizzle<XM_SWIZZLE_Y, XM_SWIZZLE_X, XM_SWIZZLE_W, XM_SWIZZLE_Z>(AR1X));
			NoIntersection = XMVectorOrInt(NoIntersection,
				XMVectorGreater(XMVectorAbs(d), XMVectorAdd(d_A, d_B)));

			// l = a(w) x b(u) = (-r10, r00, 0)
			// d(A) = h(A) dot abs(r10, r00, 0)
			// d(B) = h(B) dot abs(0, r22, r21)
			d = XMVector3Dot(t, XMVectorPermute<XM_PERMUTE_1Y, XM_PERMUTE_0X, XM_PERMUTE_0W, XM_PERMUTE_0Z>(RX0, -RX0));
			d_A = XMVector3Dot(h_A, XMVectorSwizzle<XM_SWIZZLE_Y, XM_SWIZZLE_X, XM_SWIZZLE_W, XM_SWIZZLE_Z>(ARX0));
			d_B = XMVector3Dot(h_B, XMVectorSwizzle<XM_SWIZZLE_W, XM_SWIZZLE_Z, XM_SWIZZLE_Y, XM_SWIZZLE_X>(AR2X));
			NoIntersection = XMVectorOrInt(NoIntersection,
				XMVectorGreater(XMVectorAbs(d), XMVectorAdd(d_A, d_B)));

			// l = a(w) x b(v) = (-r11, r01, 0)
			// d(A) = h(A) dot abs(r11, r01, 0)
			// d(B) = h(B) dot abs(r22, 0, r20)
			d = XMVector3Dot(t, XMVectorPermute<XM_PERMUTE_1Y, XM_PERMUTE_0X, XM_PERMUTE_0W, XM_PERMUTE_0Z>(RX1, -RX1));
			d_A = XMVector3Dot(h_A, XMVectorSwizzle<XM_SWIZZLE_Y, XM_SWIZZLE_X, XM_SWIZZLE_W, XM_SWIZZLE_Z>(ARX1));
			d_B = XMVector3Dot(h_B, XMVectorSwizzle<XM_SWIZZLE_Z, XM_SWIZZLE_W, XM_SWIZZLE_X, XM_SWIZZLE_Y>(AR2X));
			NoIntersection = XMVectorOrInt(NoIntersection,
				XMVectorGreater(XMVectorAbs(d), XMVectorAdd(d_A, d_B)));

			// l = a(w) x b(w) = (-r12, r02, 0)
			// d(A) = h(A) dot abs(r12, r02, 0)
			// d(B) = h(B) dot abs(r21, r20, 0)
			d = XMVector3Dot(t, XMVectorPermute<XM_PERMUTE_1Y, XM_PERMUTE_0X, XM_PERMUTE_0W, XM_PERMUTE_0Z>(RX2, -RX2));
			d_A = XMVector3Dot(h_A, XMVectorSwizzle<XM_SWIZZLE_Y, XM_SWIZZLE_X, XM_SWIZZLE_W, XM_SWIZZLE_Z>(ARX2));
			d_B = XMVector3Dot(h_B, XMVectorSwizzle<XM_SWIZZLE_Y, XM_SWIZZLE_X, XM_SWIZZLE_W, XM_SWIZZLE_Z>(AR2X));
			NoIntersection = XMVectorOrInt(NoIntersection,
				XMVectorGreater(XMVectorAbs(d), XMVectorAdd(d_A, d_B)));

			// No seperating axis found, boxes must intersect.
			return XMVector4NotEqualInt(NoIntersection, XMVectorTrueInt()) ? true : false;
		}

		bool OBB::Intersects(_In_ const Frustum& fr) const
		{
			return fr.Intersects(*this);
		}

		bool __vectorcall OBB::Intersects(_In_ const Math::Vector3& v0, _In_ const Math::Vector3& v1, _In_ const Math::Vector3& v2) const
		{
			using namespace DirectX;

			XMVECTOR V0 = v0;
			XMVECTOR V1 = v1;
			XMVECTOR V2 = v2;

			// Load the box center & orientation.
			XMVECTOR vCenter = Center;
			XMVECTOR vOrientation = Orientation;

			// Transform the triangle vertices into the space of the box.
			XMVECTOR TV0 = XMVector3InverseRotate(V0 - vCenter, vOrientation);
			XMVECTOR TV1 = XMVector3InverseRotate(V1 - vCenter, vOrientation);
			XMVECTOR TV2 = XMVector3InverseRotate(V2 - vCenter, vOrientation);

			AABB box;
			box.Center = Math::Vector3(0.0f, 0.0f, 0.0f);
			box.Extents = Extents;

			// Use the triangle vs axis aligned box intersection routine.
			return box.Intersects(TV0, TV1, TV2);
		}

		EmPlaneIntersection::Type __vectorcall OBB::Intersects(_In_ const Math::Plane& Plane) const
		{
			using namespace DirectX;

			XMVECTOR vPlane = Plane;

			assert(DirectX::Internal::XMPlaneIsUnit(vPlane));

			// Load the box.
			XMVECTOR vCenter = Center;
			XMVECTOR vExtents = Extents;
			XMVECTOR BoxOrientation = Orientation;

			assert(DirectX::Internal::XMQuaternionIsUnit(BoxOrientation));

			// Set w of the center to one so we can dot4 with a plane.
			vCenter = XMVectorInsert<0, 0, 0, 0, 1>(vCenter, XMVectorSplatOne());

			// Build the 3x3 rotation matrix that defines the box axes.
			XMMATRIX R = XMMatrixRotationQuaternion(BoxOrientation);

			XMVECTOR Outside, Inside;
			DirectX::Internal::FastIntersectOrientedBoxPlane(vCenter, vExtents, R.r[0], R.r[1], R.r[2], vPlane, Outside, Inside);

			// If the box is outside any plane it is outside.
			if (XMVector4EqualInt(Outside, XMVectorTrueInt()))
				return EmPlaneIntersection::eFront;

			// If the box is inside all planes it is inside.
			if (XMVector4EqualInt(Inside, XMVectorTrueInt()))
				return EmPlaneIntersection::eBack;

			// The box is not inside all planes or outside a plane it intersects.
			return EmPlaneIntersection::eIntersecting;
		}

		bool __vectorcall OBB::Intersects(_In_ const Math::Vector3& Origin, _In_ const Math::Vector3& Direction, _Out_ float& Dist) const
		{
			using namespace DirectX;

			XMVECTOR vDirection = Direction;
			XMVECTOR vOrigin = Origin;

			assert(DirectX::Internal::XMVector3IsUnit(vDirection));

			static const XMVECTORU32 SelectY =
			{
				XM_SELECT_0, XM_SELECT_1, XM_SELECT_0, XM_SELECT_0
			};
			static const XMVECTORU32 SelectZ =
			{
				XM_SELECT_0, XM_SELECT_0, XM_SELECT_1, XM_SELECT_0
			};

			// Load the box.
			XMVECTOR vCenter = Center;
			XMVECTOR vExtents = Extents;
			XMVECTOR vOrientation = Orientation;

			assert(DirectX::Internal::XMQuaternionIsUnit(vOrientation));

			// Get the boxes normalized side directions.
			XMMATRIX R = XMMatrixRotationQuaternion(vOrientation);

			// Adjust ray origin to be relative to center of the box.
			XMVECTOR TOrigin = vCenter - vOrigin;

			// Compute the dot product againt each axis of the box.
			XMVECTOR AxisDotOrigin = XMVector3Dot(R.r[0], TOrigin);
			AxisDotOrigin = XMVectorSelect(AxisDotOrigin, XMVector3Dot(R.r[1], TOrigin), SelectY);
			AxisDotOrigin = XMVectorSelect(AxisDotOrigin, XMVector3Dot(R.r[2], TOrigin), SelectZ);

			XMVECTOR AxisDotDirection = XMVector3Dot(R.r[0], vDirection);
			AxisDotDirection = XMVectorSelect(AxisDotDirection, XMVector3Dot(R.r[1], vDirection), SelectY);
			AxisDotDirection = XMVectorSelect(AxisDotDirection, XMVector3Dot(R.r[2], vDirection), SelectZ);

			// if (fabs(AxisDotDirection) <= Epsilon) the ray is nearly parallel to the slab.
			XMVECTOR IsParallel = XMVectorLessOrEqual(XMVectorAbs(AxisDotDirection), g_RayEpsilon);

			// Test against all three axes simultaneously.
			XMVECTOR InverseAxisDotDirection = XMVectorReciprocal(AxisDotDirection);
			XMVECTOR t1 = (AxisDotOrigin - vExtents) * InverseAxisDotDirection;
			XMVECTOR t2 = (AxisDotOrigin + vExtents) * InverseAxisDotDirection;

			// Compute the max of min(t1,t2) and the min of max(t1,t2) ensuring we don't
			// use the results from any directions parallel to the slab.
			XMVECTOR t_min = XMVectorSelect(XMVectorMin(t1, t2), g_FltMin, IsParallel);
			XMVECTOR t_max = XMVectorSelect(XMVectorMax(t1, t2), g_FltMax, IsParallel);

			// t_min.x = maximum( t_min.x, t_min.y, t_min.z );
			// t_max.x = minimum( t_max.x, t_max.y, t_max.z );
			t_min = XMVectorMax(t_min, XMVectorSplatY(t_min));  // x = max(x,y)
			t_min = XMVectorMax(t_min, XMVectorSplatZ(t_min));  // x = max(max(x,y),z)
			t_max = XMVectorMin(t_max, XMVectorSplatY(t_max));  // x = min(x,y)
			t_max = XMVectorMin(t_max, XMVectorSplatZ(t_max));  // x = min(min(x,y),z)

																// if ( t_min > t_max ) return false;
			XMVECTOR NoIntersection = XMVectorGreater(XMVectorSplatX(t_min), XMVectorSplatX(t_max));

			// if ( t_max < 0.0f ) return false;
			NoIntersection = XMVectorOrInt(NoIntersection, XMVectorLess(XMVectorSplatX(t_max), XMVectorZero()));

			// if (IsParallel && (-Extents > AxisDotOrigin || Extents < AxisDotOrigin)) return false;
			XMVECTOR ParallelOverlap = XMVectorInBounds(AxisDotOrigin, vExtents);
			NoIntersection = XMVectorOrInt(NoIntersection, XMVectorAndCInt(IsParallel, ParallelOverlap));

			if (DirectX::Internal::XMVector3AnyTrue(NoIntersection) == false)
			{
				// Store the x-component to *pDist
				XMStoreFloat(&Dist, t_min);
				return true;
			}

			Dist = 0.f;
			return false;
		}

		EmContainment::Type __vectorcall OBB::ContainedBy(_In_ const Math::Plane& Plane0, _In_ const Math::Plane& Plane1, _In_ const Math::Plane& Plane2,
			_In_ const Math::Plane& Plane3, _In_ const Math::Plane& Plane4, _In_ const Math::Plane& Plane5) const
		{
			using namespace DirectX;

			XMVECTOR vPlane0 = Plane0;
			XMVECTOR vPlane1 = Plane1;
			XMVECTOR vPlane2 = Plane2;
			XMVECTOR vPlane3 = Plane3;
			XMVECTOR vPlane4 = Plane4;
			XMVECTOR vPlane5 = Plane5;

			// Load the box.
			XMVECTOR vCenter = Center;
			XMVECTOR vExtents = Extents;
			XMVECTOR BoxOrientation = Orientation;

			assert(DirectX::Internal::XMQuaternionIsUnit(BoxOrientation));

			// Set w of the center to one so we can dot4 with a plane.
			vCenter = XMVectorInsert<0, 0, 0, 0, 1>(vCenter, XMVectorSplatOne());

			// Build the 3x3 rotation matrix that defines the box axes.
			XMMATRIX R = XMMatrixRotationQuaternion(BoxOrientation);

			XMVECTOR Outside, Inside;

			// Test against each plane.
			DirectX::Internal::FastIntersectOrientedBoxPlane(vCenter, vExtents, R.r[0], R.r[1], R.r[2], vPlane0, Outside, Inside);

			XMVECTOR AnyOutside = Outside;
			XMVECTOR AllInside = Inside;

			DirectX::Internal::FastIntersectOrientedBoxPlane(vCenter, vExtents, R.r[0], R.r[1], R.r[2], vPlane1, Outside, Inside);
			AnyOutside = XMVectorOrInt(AnyOutside, Outside);
			AllInside = XMVectorAndInt(AllInside, Inside);

			DirectX::Internal::FastIntersectOrientedBoxPlane(vCenter, vExtents, R.r[0], R.r[1], R.r[2], vPlane2, Outside, Inside);
			AnyOutside = XMVectorOrInt(AnyOutside, Outside);
			AllInside = XMVectorAndInt(AllInside, Inside);

			DirectX::Internal::FastIntersectOrientedBoxPlane(vCenter, vExtents, R.r[0], R.r[1], R.r[2], vPlane3, Outside, Inside);
			AnyOutside = XMVectorOrInt(AnyOutside, Outside);
			AllInside = XMVectorAndInt(AllInside, Inside);

			DirectX::Internal::FastIntersectOrientedBoxPlane(vCenter, vExtents, R.r[0], R.r[1], R.r[2], vPlane4, Outside, Inside);
			AnyOutside = XMVectorOrInt(AnyOutside, Outside);
			AllInside = XMVectorAndInt(AllInside, Inside);

			DirectX::Internal::FastIntersectOrientedBoxPlane(vCenter, vExtents, R.r[0], R.r[1], R.r[2], vPlane5, Outside, Inside);
			AnyOutside = XMVectorOrInt(AnyOutside, Outside);
			AllInside = XMVectorAndInt(AllInside, Inside);

			// If the box is outside any plane it is outside.
			if (XMVector4EqualInt(AnyOutside, XMVectorTrueInt()))
				return EmContainment::eDisjoint;

			// If the box is inside all planes it is inside.
			if (XMVector4EqualInt(AllInside, XMVectorTrueInt()))
				return EmContainment::eContains;

			// The box is not inside all planes or outside a plane, it may intersect.
			return EmContainment::eIntersects;
		}

		void OBB::CreateFromAABB(_Out_ OBB& Out, _In_ const AABB& box)
		{
			Out.Center = box.Center;
			Out.Extents = box.Extents;
			Out.Orientation = Math::Quaternion::Identity;
		}

		void OBB::CreateFromPoints(_Out_ OBB& Out, _In_ size_t Count,
			_In_reads_bytes_(sizeof(Math::Vector3) + Stride*(Count - 1)) const Math::Vector3* pPoints, _In_ size_t Stride)
		{
			using namespace DirectX;

			assert(Count > 0);
			assert(pPoints != 0);

			XMVECTOR CenterOfMass = XMVectorZero();

			// Compute the center of mass and inertia tensor of the points.
			for (size_t i = 0; i < Count; ++i)
			{
				XMVECTOR Point = XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(reinterpret_cast<const uint8_t*>(pPoints) + i * Stride));

				CenterOfMass += Point;
			}

			CenterOfMass *= XMVectorReciprocal(XMVectorReplicate(float(Count)));

			// Compute the inertia tensor of the points around the center of mass.
			// Using the center of mass is not strictly necessary, but will hopefully
			// improve the stability of finding the eigenvectors.
			XMVECTOR XX_YY_ZZ = XMVectorZero();
			XMVECTOR XY_XZ_YZ = XMVectorZero();

			for (size_t i = 0; i < Count; ++i)
			{
				XMVECTOR Point = XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(reinterpret_cast<const uint8_t*>(pPoints) + i * Stride)) - CenterOfMass;

				XX_YY_ZZ += Point * Point;

				XMVECTOR XXY = XMVectorSwizzle<XM_SWIZZLE_X, XM_SWIZZLE_X, XM_SWIZZLE_Y, XM_SWIZZLE_W>(Point);
				XMVECTOR YZZ = XMVectorSwizzle<XM_SWIZZLE_Y, XM_SWIZZLE_Z, XM_SWIZZLE_Z, XM_SWIZZLE_W>(Point);

				XY_XZ_YZ += XXY * YZZ;
			}

			XMVECTOR v1, v2, v3;

			// Compute the eigenvectors of the inertia tensor.
			DirectX::Internal::CalculateEigenVectorsFromCovarianceMatrix(XMVectorGetX(XX_YY_ZZ), XMVectorGetY(XX_YY_ZZ),
				XMVectorGetZ(XX_YY_ZZ),
				XMVectorGetX(XY_XZ_YZ), XMVectorGetY(XY_XZ_YZ),
				XMVectorGetZ(XY_XZ_YZ),
				&v1, &v2, &v3);

			// Put them in a matrix.
			XMMATRIX R;

			R.r[0] = XMVectorSetW(v1, 0.f);
			R.r[1] = XMVectorSetW(v2, 0.f);
			R.r[2] = XMVectorSetW(v3, 0.f);
			R.r[3] = g_XMIdentityR3.v;

			// Multiply by -1 to convert the matrix into a right handed coordinate 
			// system (Det ~= 1) in case the eigenvectors form a left handed 
			// coordinate system (Det ~= -1) because XMQuaternionRotationMatrix only 
			// works on right handed matrices.
			XMVECTOR Det = XMMatrixDeterminant(R);

			if (XMVector4Less(Det, XMVectorZero()))
			{
				R.r[0] *= g_XMNegativeOne.v;
				R.r[1] *= g_XMNegativeOne.v;
				R.r[2] *= g_XMNegativeOne.v;
			}

			// Get the rotation quaternion from the matrix.
			XMVECTOR vOrientation = XMQuaternionRotationMatrix(R);

			// Make sure it is normal (in case the vectors are slightly non-orthogonal).
			vOrientation = XMQuaternionNormalize(vOrientation);

			// Rebuild the rotation matrix from the quaternion.
			R = XMMatrixRotationQuaternion(vOrientation);

			// Build the rotation into the rotated space.
			XMMATRIX InverseR = XMMatrixTranspose(R);

			// Find the minimum OBB using the eigenvectors as the axes.
			XMVECTOR vMin, vMax;

			vMin = vMax = XMVector3TransformNormal(*pPoints, InverseR);

			for (size_t i = 1; i < Count; ++i)
			{
				XMVECTOR Point = XMVector3TransformNormal(XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(reinterpret_cast<const uint8_t*>(pPoints) + i * Stride)),
					InverseR);

				vMin = XMVectorMin(vMin, Point);
				vMax = XMVectorMax(vMax, Point);
			}

			// Rotate the center into world space.
			XMVECTOR vCenter = (vMin + vMax) * 0.5f;
			vCenter = XMVector3TransformNormal(vCenter, R);

			// Store center, extents, and orientation.
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&Out.Center), vCenter);
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&Out.Extents), (vMax - vMin) * 0.5f);
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&Out.Orientation), vOrientation);
		}

		Frustum::Frustum() {}
		Frustum::Frustum(_In_ const Math::Vector3& Origin, _In_ const Math::Quaternion& Orientation,
			_In_ float RightSlope, _In_ float LeftSlope, _In_ float TopSlope, _In_ float BottomSlope,
			_In_ float Near, _In_ float Far)
			: Origin(Origin), Orientation(Orientation), RightSlope(RightSlope), LeftSlope(LeftSlope), TopSlope(TopSlope), BottomSlope(BottomSlope), Near(Near), Far(Far) {}
		Frustum::Frustum(_In_ const Frustum& fr)
			: Origin(fr.Origin), Orientation(fr.Orientation), RightSlope(fr.RightSlope), LeftSlope(fr.LeftSlope), TopSlope(fr.TopSlope), BottomSlope(fr.BottomSlope), Near(fr.Near), Far(fr.Far) {}
		Frustum::Frustum(_In_ const Math::Matrix& Projection) { CreateFromMatrix(*this, Projection); }

		void __vectorcall Frustum::Transform(_In_ const Math::Matrix& matrix)
		{
			using namespace DirectX;

			// Load the frustum.
			XMVECTOR vOrigin = Origin;
			XMVECTOR vOrientation = Orientation;

			assert(DirectX::Internal::XMQuaternionIsUnit(vOrientation));

			XMMATRIX M = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(&matrix));

			// Composite the frustum rotation and the transform rotation
			XMMATRIX nM;
			nM.r[0] = XMVector3Normalize(M.r[0]);
			nM.r[1] = XMVector3Normalize(M.r[1]);
			nM.r[2] = XMVector3Normalize(M.r[2]);
			nM.r[3] = g_XMIdentityR3;
			XMVECTOR Rotation = XMQuaternionRotationMatrix(nM);
			vOrientation = XMQuaternionMultiply(vOrientation, Rotation);

			// Transform the center.
			vOrigin = XMVector3Transform(vOrigin, M);

			// Store the frustum.
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&Origin), vOrigin);
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&Orientation), vOrientation);

			// Scale the near and far distances (the slopes remain the same).
			XMVECTOR dX = XMVector3Dot(M.r[0], M.r[0]);
			XMVECTOR dY = XMVector3Dot(M.r[1], M.r[1]);
			XMVECTOR dZ = XMVector3Dot(M.r[2], M.r[2]);

			XMVECTOR d = XMVectorMax(dX, XMVectorMax(dY, dZ));
			float Scale = sqrtf(XMVectorGetX(d));

			Near = Near * Scale;
			Far = Far * Scale;

			// Copy the slopes.
			RightSlope = RightSlope;
			LeftSlope = LeftSlope;
			TopSlope = TopSlope;
			BottomSlope = BottomSlope;
		}

		void __vectorcall Frustum::Transform(_In_ float Scale, _In_ const Math::Quaternion& Rotation, _In_ const Math::Vector3& Translation)
		{
			using namespace DirectX;

			XMVECTOR R = Rotation;
			XMVECTOR T = Translation;

			assert(DirectX::Internal::XMQuaternionIsUnit(R));

			// Load the frustum.
			XMVECTOR vOrigin = Origin;
			XMVECTOR vOrientation = Orientation;

			assert(DirectX::Internal::XMQuaternionIsUnit(vOrientation));

			// Composite the frustum rotation and the transform rotation.
			vOrientation = XMQuaternionMultiply(vOrientation, R);

			// Transform the origin.
			vOrigin = XMVector3Rotate(vOrigin * XMVectorReplicate(Scale), R) + T;

			// Store the frustum.
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&Origin), vOrigin);
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&Orientation), vOrientation);

			// Scale the near and far distances (the slopes remain the same).
			Near = Near * Scale;
			Far = Far * Scale;

			// Copy the slopes.
			RightSlope = RightSlope;
			LeftSlope = LeftSlope;
			TopSlope = TopSlope;
			BottomSlope = BottomSlope;
		}

		void Frustum::GetCorners(_Out_writes_(8) Math::Vector3* Corners) const
		{
			using namespace DirectX;

			assert(Corners != 0);

			// Load origin and orientation of the frustum.
			XMVECTOR vOrigin = Origin;
			XMVECTOR vOrientation = Orientation;

			assert(DirectX::Internal::XMQuaternionIsUnit(vOrientation));

			// Build the corners of the frustum.
			XMVECTOR vRightTop = XMVectorSet(RightSlope, TopSlope, 1.0f, 0.0f);
			XMVECTOR vRightBottom = XMVectorSet(RightSlope, BottomSlope, 1.0f, 0.0f);
			XMVECTOR vLeftTop = XMVectorSet(LeftSlope, TopSlope, 1.0f, 0.0f);
			XMVECTOR vLeftBottom = XMVectorSet(LeftSlope, BottomSlope, 1.0f, 0.0f);
			XMVECTOR vNear = XMVectorReplicatePtr(&Near);
			XMVECTOR vFar = XMVectorReplicatePtr(&Far);

			// Returns 8 corners position of bounding frustum.
			//     Near    Far
			//    0----1  4----5
			//    |    |  |    |
			//    |    |  |    |
			//    3----2  7----6

			XMVECTOR vCorners[CORNER_COUNT];
			vCorners[0] = vLeftTop * vNear;
			vCorners[1] = vRightTop * vNear;
			vCorners[2] = vRightBottom * vNear;
			vCorners[3] = vLeftBottom * vNear;
			vCorners[4] = vLeftTop * vFar;
			vCorners[5] = vRightTop * vFar;
			vCorners[6] = vRightBottom * vFar;
			vCorners[7] = vLeftBottom * vFar;

			for (size_t i = 0; i < CORNER_COUNT; ++i)
			{
				XMVECTOR C = XMVector3Rotate(vCorners[i], vOrientation) + vOrigin;
				XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&Corners[i]), C);
			}
		}

		EmContainment::Type __vectorcall Frustum::Contains(_In_ const Math::Vector3& Point) const
		{
			using namespace DirectX;

			XMVECTOR vPoint = Point;

			// Build frustum planes.
			XMVECTOR Planes[6];
			Planes[0] = XMVectorSet(0.0f, 0.0f, -1.0f, Near);
			Planes[1] = XMVectorSet(0.0f, 0.0f, 1.0f, -Far);
			Planes[2] = XMVectorSet(1.0f, 0.0f, -RightSlope, 0.0f);
			Planes[3] = XMVectorSet(-1.0f, 0.0f, LeftSlope, 0.0f);
			Planes[4] = XMVectorSet(0.0f, 1.0f, -TopSlope, 0.0f);
			Planes[5] = XMVectorSet(0.0f, -1.0f, BottomSlope, 0.0f);

			// Load origin and orientation.
			XMVECTOR vOrigin = Origin;
			XMVECTOR vOrientation = Orientation;

			assert(DirectX::Internal::XMQuaternionIsUnit(vOrientation));

			// Transform point into local space of frustum.
			XMVECTOR TPoint = XMVector3InverseRotate(vPoint - vOrigin, vOrientation);

			// Set w to one.
			TPoint = XMVectorInsert<0, 0, 0, 0, 1>(TPoint, XMVectorSplatOne());

			XMVECTOR Zero = XMVectorZero();
			XMVECTOR Outside = Zero;

			// Test point against each plane of the frustum.
			for (size_t i = 0; i < 6; ++i)
			{
				XMVECTOR Dot = XMVector4Dot(TPoint, Planes[i]);
				Outside = XMVectorOrInt(Outside, XMVectorGreater(Dot, Zero));
			}

			return XMVector4NotEqualInt(Outside, XMVectorTrueInt()) ? EmContainment::eContains : EmContainment::eDisjoint;
		}

		EmContainment::Type __vectorcall Frustum::Contains(_In_ const Math::Vector3& V0, _In_ const Math::Vector3& V1, _In_ const Math::Vector3& V2) const
		{
			using namespace DirectX;

			// Load origin and orientation of the frustum.
			XMVECTOR vOrigin = Origin;
			XMVECTOR vOrientation = Orientation;

			// Create 6 planes (do it inline to encourage use of registers)
			XMVECTOR NearPlane = XMVectorSet(0.0f, 0.0f, -1.0f, Near);
			NearPlane = DirectX::Internal::XMPlaneTransform(NearPlane, vOrientation, vOrigin);
			NearPlane = XMPlaneNormalize(NearPlane);

			XMVECTOR FarPlane = XMVectorSet(0.0f, 0.0f, 1.0f, -Far);
			FarPlane = DirectX::Internal::XMPlaneTransform(FarPlane, vOrientation, vOrigin);
			FarPlane = XMPlaneNormalize(FarPlane);

			XMVECTOR RightPlane = XMVectorSet(1.0f, 0.0f, -RightSlope, 0.0f);
			RightPlane = DirectX::Internal::XMPlaneTransform(RightPlane, vOrientation, vOrigin);
			RightPlane = XMPlaneNormalize(RightPlane);

			XMVECTOR LeftPlane = XMVectorSet(-1.0f, 0.0f, LeftSlope, 0.0f);
			LeftPlane = DirectX::Internal::XMPlaneTransform(LeftPlane, vOrientation, vOrigin);
			LeftPlane = XMPlaneNormalize(LeftPlane);

			XMVECTOR TopPlane = XMVectorSet(0.0f, 1.0f, -TopSlope, 0.0f);
			TopPlane = DirectX::Internal::XMPlaneTransform(TopPlane, vOrientation, vOrigin);
			TopPlane = XMPlaneNormalize(TopPlane);

			XMVECTOR BottomPlane = XMVectorSet(0.0f, -1.0f, BottomSlope, 0.0f);
			BottomPlane = DirectX::Internal::XMPlaneTransform(BottomPlane, vOrientation, vOrigin);
			BottomPlane = XMPlaneNormalize(BottomPlane);

			return TriangleTests::ContainedBy(V0, V1, V2, NearPlane, FarPlane, RightPlane, LeftPlane, TopPlane, BottomPlane);
		}

		EmContainment::Type Frustum::Contains(_In_ const Sphere& sh) const
		{
			using namespace DirectX;

			// Load origin and orientation of the frustum.
			XMVECTOR vOrigin = Origin;
			XMVECTOR vOrientation = Orientation;

			// Create 6 planes (do it inline to encourage use of registers)
			XMVECTOR NearPlane = XMVectorSet(0.0f, 0.0f, -1.0f, Near);
			NearPlane = DirectX::Internal::XMPlaneTransform(NearPlane, vOrientation, vOrigin);
			NearPlane = XMPlaneNormalize(NearPlane);

			XMVECTOR FarPlane = XMVectorSet(0.0f, 0.0f, 1.0f, -Far);
			FarPlane = DirectX::Internal::XMPlaneTransform(FarPlane, vOrientation, vOrigin);
			FarPlane = XMPlaneNormalize(FarPlane);

			XMVECTOR RightPlane = XMVectorSet(1.0f, 0.0f, -RightSlope, 0.0f);
			RightPlane = DirectX::Internal::XMPlaneTransform(RightPlane, vOrientation, vOrigin);
			RightPlane = XMPlaneNormalize(RightPlane);

			XMVECTOR LeftPlane = XMVectorSet(-1.0f, 0.0f, LeftSlope, 0.0f);
			LeftPlane = DirectX::Internal::XMPlaneTransform(LeftPlane, vOrientation, vOrigin);
			LeftPlane = XMPlaneNormalize(LeftPlane);

			XMVECTOR TopPlane = XMVectorSet(0.0f, 1.0f, -TopSlope, 0.0f);
			TopPlane = DirectX::Internal::XMPlaneTransform(TopPlane, vOrientation, vOrigin);
			TopPlane = XMPlaneNormalize(TopPlane);

			XMVECTOR BottomPlane = XMVectorSet(0.0f, -1.0f, BottomSlope, 0.0f);
			BottomPlane = DirectX::Internal::XMPlaneTransform(BottomPlane, vOrientation, vOrigin);
			BottomPlane = XMPlaneNormalize(BottomPlane);

			return sh.ContainedBy(NearPlane, FarPlane, RightPlane, LeftPlane, TopPlane, BottomPlane);
		}

		EmContainment::Type Frustum::Contains(_In_ const AABB& box) const
		{
			using namespace DirectX;

			// Load origin and orientation of the frustum.
			XMVECTOR vOrigin = Origin;
			XMVECTOR vOrientation = Orientation;

			// Create 6 planes (do it inline to encourage use of registers)
			XMVECTOR NearPlane = XMVectorSet(0.0f, 0.0f, -1.0f, Near);
			NearPlane = DirectX::Internal::XMPlaneTransform(NearPlane, vOrientation, vOrigin);
			NearPlane = XMPlaneNormalize(NearPlane);

			XMVECTOR FarPlane = XMVectorSet(0.0f, 0.0f, 1.0f, -Far);
			FarPlane = DirectX::Internal::XMPlaneTransform(FarPlane, vOrientation, vOrigin);
			FarPlane = XMPlaneNormalize(FarPlane);

			XMVECTOR RightPlane = XMVectorSet(1.0f, 0.0f, -RightSlope, 0.0f);
			RightPlane = DirectX::Internal::XMPlaneTransform(RightPlane, vOrientation, vOrigin);
			RightPlane = XMPlaneNormalize(RightPlane);

			XMVECTOR LeftPlane = XMVectorSet(-1.0f, 0.0f, LeftSlope, 0.0f);
			LeftPlane = DirectX::Internal::XMPlaneTransform(LeftPlane, vOrientation, vOrigin);
			LeftPlane = XMPlaneNormalize(LeftPlane);

			XMVECTOR TopPlane = XMVectorSet(0.0f, 1.0f, -TopSlope, 0.0f);
			TopPlane = DirectX::Internal::XMPlaneTransform(TopPlane, vOrientation, vOrigin);
			TopPlane = XMPlaneNormalize(TopPlane);

			XMVECTOR BottomPlane = XMVectorSet(0.0f, -1.0f, BottomSlope, 0.0f);
			BottomPlane = DirectX::Internal::XMPlaneTransform(BottomPlane, vOrientation, vOrigin);
			BottomPlane = XMPlaneNormalize(BottomPlane);

			return box.ContainedBy(NearPlane, FarPlane, RightPlane, LeftPlane, TopPlane, BottomPlane);
		}

		EmContainment::Type Frustum::Contains(_In_ const OBB& box) const
		{
			using namespace DirectX;

			// Load origin and orientation of the frustum.
			XMVECTOR vOrigin = Origin;
			XMVECTOR vOrientation = Orientation;

			// Create 6 planes (do it inline to encourage use of registers)
			XMVECTOR NearPlane = XMVectorSet(0.0f, 0.0f, -1.0f, Near);
			NearPlane = DirectX::Internal::XMPlaneTransform(NearPlane, vOrientation, vOrigin);
			NearPlane = XMPlaneNormalize(NearPlane);

			XMVECTOR FarPlane = XMVectorSet(0.0f, 0.0f, 1.0f, -Far);
			FarPlane = DirectX::Internal::XMPlaneTransform(FarPlane, vOrientation, vOrigin);
			FarPlane = XMPlaneNormalize(FarPlane);

			XMVECTOR RightPlane = XMVectorSet(1.0f, 0.0f, -RightSlope, 0.0f);
			RightPlane = DirectX::Internal::XMPlaneTransform(RightPlane, vOrientation, vOrigin);
			RightPlane = XMPlaneNormalize(RightPlane);

			XMVECTOR LeftPlane = XMVectorSet(-1.0f, 0.0f, LeftSlope, 0.0f);
			LeftPlane = DirectX::Internal::XMPlaneTransform(LeftPlane, vOrientation, vOrigin);
			LeftPlane = XMPlaneNormalize(LeftPlane);

			XMVECTOR TopPlane = XMVectorSet(0.0f, 1.0f, -TopSlope, 0.0f);
			TopPlane = DirectX::Internal::XMPlaneTransform(TopPlane, vOrientation, vOrigin);
			TopPlane = XMPlaneNormalize(TopPlane);

			XMVECTOR BottomPlane = XMVectorSet(0.0f, -1.0f, BottomSlope, 0.0f);
			BottomPlane = DirectX::Internal::XMPlaneTransform(BottomPlane, vOrientation, vOrigin);
			BottomPlane = XMPlaneNormalize(BottomPlane);

			return box.ContainedBy(NearPlane, FarPlane, RightPlane, LeftPlane, TopPlane, BottomPlane);
		}

		EmContainment::Type Frustum::Contains(_In_ const Frustum& fr) const
		{
			using namespace DirectX;

			// Load origin and orientation of the frustum.
			XMVECTOR vOrigin = Origin;
			XMVECTOR vOrientation = Orientation;

			// Create 6 planes (do it inline to encourage use of registers)
			XMVECTOR NearPlane = XMVectorSet(0.0f, 0.0f, -1.0f, Near);
			NearPlane = DirectX::Internal::XMPlaneTransform(NearPlane, vOrientation, vOrigin);
			NearPlane = XMPlaneNormalize(NearPlane);

			XMVECTOR FarPlane = XMVectorSet(0.0f, 0.0f, 1.0f, -Far);
			FarPlane = DirectX::Internal::XMPlaneTransform(FarPlane, vOrientation, vOrigin);
			FarPlane = XMPlaneNormalize(FarPlane);

			XMVECTOR RightPlane = XMVectorSet(1.0f, 0.0f, -RightSlope, 0.0f);
			RightPlane = DirectX::Internal::XMPlaneTransform(RightPlane, vOrientation, vOrigin);
			RightPlane = XMPlaneNormalize(RightPlane);

			XMVECTOR LeftPlane = XMVectorSet(-1.0f, 0.0f, LeftSlope, 0.0f);
			LeftPlane = DirectX::Internal::XMPlaneTransform(LeftPlane, vOrientation, vOrigin);
			LeftPlane = XMPlaneNormalize(LeftPlane);

			XMVECTOR TopPlane = XMVectorSet(0.0f, 1.0f, -TopSlope, 0.0f);
			TopPlane = DirectX::Internal::XMPlaneTransform(TopPlane, vOrientation, vOrigin);
			TopPlane = XMPlaneNormalize(TopPlane);

			XMVECTOR BottomPlane = XMVectorSet(0.0f, -1.0f, BottomSlope, 0.0f);
			BottomPlane = DirectX::Internal::XMPlaneTransform(BottomPlane, vOrientation, vOrigin);
			BottomPlane = XMPlaneNormalize(BottomPlane);

			return fr.ContainedBy(NearPlane, FarPlane, RightPlane, LeftPlane, TopPlane, BottomPlane);
		}

		bool Frustum::Intersects(_In_ const Sphere& sh) const
		{
			using namespace DirectX;

			XMVECTOR Zero = XMVectorZero();

			// Build the frustum planes.
			XMVECTOR Planes[6];
			Planes[0] = XMVectorSet(0.0f, 0.0f, -1.0f, Near);
			Planes[1] = XMVectorSet(0.0f, 0.0f, 1.0f, -Far);
			Planes[2] = XMVectorSet(1.0f, 0.0f, -RightSlope, 0.0f);
			Planes[3] = XMVectorSet(-1.0f, 0.0f, LeftSlope, 0.0f);
			Planes[4] = XMVectorSet(0.0f, 1.0f, -TopSlope, 0.0f);
			Planes[5] = XMVectorSet(0.0f, -1.0f, BottomSlope, 0.0f);

			// Normalize the planes so we can compare to the sphere radius.
			Planes[2] = XMVector3Normalize(Planes[2]);
			Planes[3] = XMVector3Normalize(Planes[3]);
			Planes[4] = XMVector3Normalize(Planes[4]);
			Planes[5] = XMVector3Normalize(Planes[5]);

			// Load origin and orientation of the frustum.
			XMVECTOR vOrigin = Origin;
			XMVECTOR vOrientation = Orientation;

			assert(DirectX::Internal::XMQuaternionIsUnit(vOrientation));

			// Load the sphere.
			XMVECTOR vCenter = sh.Center;
			XMVECTOR vRadius = XMVectorReplicatePtr(&sh.Radius);

			// Transform the center of the sphere into the local space of frustum.
			vCenter = XMVector3InverseRotate(vCenter - vOrigin, vOrientation);

			// Set w of the center to one so we can dot4 with the plane.
			vCenter = XMVectorInsert<0, 0, 0, 0, 1>(vCenter, XMVectorSplatOne());

			// Check against each plane of the frustum.
			XMVECTOR Outside = XMVectorFalseInt();
			XMVECTOR InsideAll = XMVectorTrueInt();
			XMVECTOR CenterInsideAll = XMVectorTrueInt();

			XMVECTOR Dist[6];

			for (size_t i = 0; i < 6; ++i)
			{
				Dist[i] = XMVector4Dot(vCenter, Planes[i]);

				// Outside the plane?
				Outside = XMVectorOrInt(Outside, XMVectorGreater(Dist[i], vRadius));

				// Fully inside the plane?
				InsideAll = XMVectorAndInt(InsideAll, XMVectorLessOrEqual(Dist[i], -vRadius));

				// Check if the center is inside the plane.
				CenterInsideAll = XMVectorAndInt(CenterInsideAll, XMVectorLessOrEqual(Dist[i], Zero));
			}

			// If the sphere is outside any of the planes it is outside. 
			if (XMVector4EqualInt(Outside, XMVectorTrueInt()))
				return false;

			// If the sphere is inside all planes it is fully inside.
			if (XMVector4EqualInt(InsideAll, XMVectorTrueInt()))
				return true;

			// If the center of the sphere is inside all planes and the sphere intersects 
			// one or more planes then it must intersect.
			if (XMVector4EqualInt(CenterInsideAll, XMVectorTrueInt()))
				return true;

			// The sphere may be outside the frustum or intersecting the frustum.
			// Find the nearest feature (face, edge, or corner) on the frustum 
			// to the sphere.

			// The faces adjacent to each face are:
			static const size_t adjacent_faces[6][4] =
			{
				{ 2, 3, 4, 5 },    // 0
				{ 2, 3, 4, 5 },    // 1
				{ 0, 1, 4, 5 },    // 2
				{ 0, 1, 4, 5 },    // 3
				{ 0, 1, 2, 3 },    // 4
				{ 0, 1, 2, 3 }
			};  // 5

			XMVECTOR Intersects = XMVectorFalseInt();

			// Check to see if the nearest feature is one of the planes.
			for (size_t i = 0; i < 6; ++i)
			{
				// Find the nearest point on the plane to the center of the sphere.
				XMVECTOR Point = vCenter - (Planes[i] * Dist[i]);

				// Set w of the point to one.
				Point = XMVectorInsert<0, 0, 0, 0, 1>(Point, XMVectorSplatOne());

				// If the point is inside the face (inside the adjacent planes) then
				// this plane is the nearest feature.
				XMVECTOR InsideFace = XMVectorTrueInt();

				for (size_t j = 0; j < 4; j++)
				{
					size_t plane_index = adjacent_faces[i][j];

					InsideFace = XMVectorAndInt(InsideFace,
						XMVectorLessOrEqual(XMVector4Dot(Point, Planes[plane_index]), Zero));
				}

				// Since we have already checked distance from the plane we know that the
				// sphere must intersect if this plane is the nearest feature.
				Intersects = XMVectorOrInt(Intersects,
					XMVectorAndInt(XMVectorGreater(Dist[i], Zero), InsideFace));
			}

			if (XMVector4EqualInt(Intersects, XMVectorTrueInt()))
				return true;

			// Build the corners of the frustum.
			XMVECTOR vRightTop = XMVectorSet(RightSlope, TopSlope, 1.0f, 0.0f);
			XMVECTOR vRightBottom = XMVectorSet(RightSlope, BottomSlope, 1.0f, 0.0f);
			XMVECTOR vLeftTop = XMVectorSet(LeftSlope, TopSlope, 1.0f, 0.0f);
			XMVECTOR vLeftBottom = XMVectorSet(LeftSlope, BottomSlope, 1.0f, 0.0f);
			XMVECTOR vNear = XMVectorReplicatePtr(&Near);
			XMVECTOR vFar = XMVectorReplicatePtr(&Far);

			XMVECTOR Corners[CORNER_COUNT];
			Corners[0] = vRightTop * vNear;
			Corners[1] = vRightBottom * vNear;
			Corners[2] = vLeftTop * vNear;
			Corners[3] = vLeftBottom * vNear;
			Corners[4] = vRightTop * vFar;
			Corners[5] = vRightBottom * vFar;
			Corners[6] = vLeftTop * vFar;
			Corners[7] = vLeftBottom * vFar;

			// The Edges are:
			static const size_t edges[12][2] =
			{
				{ 0, 1 },{ 2, 3 },{ 0, 2 },{ 1, 3 },    // Near plane
				{ 4, 5 },{ 6, 7 },{ 4, 6 },{ 5, 7 },    // Far plane
				{ 0, 4 },{ 1, 5 },{ 2, 6 },{ 3, 7 },
			}; // Near to far

			XMVECTOR RadiusSq = vRadius * vRadius;

			// Check to see if the nearest feature is one of the edges (or corners).
			for (size_t i = 0; i < 12; ++i)
			{
				size_t ei0 = edges[i][0];
				size_t ei1 = edges[i][1];

				// Find the nearest point on the edge to the center of the sphere.
				// The corners of the frustum are included as the endpoints of the edges.
				XMVECTOR Point = DirectX::Internal::PointOnLineSegmentNearestPoint(Corners[ei0], Corners[ei1], vCenter);

				XMVECTOR Delta = vCenter - Point;

				XMVECTOR DistSq = XMVector3Dot(Delta, Delta);

				// If the distance to the center of the sphere to the point is less than 
				// the radius of the sphere then it must intersect.
				Intersects = XMVectorOrInt(Intersects, XMVectorLessOrEqual(DistSq, RadiusSq));
			}

			if (XMVector4EqualInt(Intersects, XMVectorTrueInt()))
				return true;

			// The sphere must be outside the frustum.
			return false;
		}

		bool Frustum::Intersects(_In_ const AABB& box) const
		{
			using namespace DirectX;

			// Make the axis aligned box oriented and do an OBB vs frustum test.
			OBB obox(box.Center, box.Extents, Math::Quaternion::Identity);
			return Intersects(obox);
		}

		bool Frustum::Intersects(_In_ const OBB& box) const
		{
			using namespace DirectX;

			static const XMVECTORU32 SelectY =
			{
				XM_SELECT_0, XM_SELECT_1, XM_SELECT_0, XM_SELECT_0
			};
			static const XMVECTORU32 SelectZ =
			{
				XM_SELECT_0, XM_SELECT_0, XM_SELECT_1, XM_SELECT_0
			};

			XMVECTOR Zero = XMVectorZero();

			// Build the frustum planes.
			XMVECTOR Planes[6];
			Planes[0] = XMVectorSet(0.0f, 0.0f, -1.0f, Near);
			Planes[1] = XMVectorSet(0.0f, 0.0f, 1.0f, -Far);
			Planes[2] = XMVectorSet(1.0f, 0.0f, -RightSlope, 0.0f);
			Planes[3] = XMVectorSet(-1.0f, 0.0f, LeftSlope, 0.0f);
			Planes[4] = XMVectorSet(0.0f, 1.0f, -TopSlope, 0.0f);
			Planes[5] = XMVectorSet(0.0f, -1.0f, BottomSlope, 0.0f);

			// Load origin and orientation of the frustum.
			XMVECTOR vOrigin = Origin;
			XMVECTOR FrustumOrientation = Orientation;

			assert(DirectX::Internal::XMQuaternionIsUnit(FrustumOrientation));

			// Load the box.
			XMVECTOR Center = box.Center;
			XMVECTOR Extents = box.Extents;
			XMVECTOR BoxOrientation = box.Orientation;

			assert(DirectX::Internal::XMQuaternionIsUnit(BoxOrientation));

			// Transform the oriented box into the space of the frustum in order to 
			// minimize the number of transforms we have to do.
			Center = XMVector3InverseRotate(Center - vOrigin, FrustumOrientation);
			BoxOrientation = XMQuaternionMultiply(BoxOrientation, XMQuaternionConjugate(FrustumOrientation));

			// Set w of the center to one so we can dot4 with the plane.
			Center = XMVectorInsert<0, 0, 0, 0, 1>(Center, XMVectorSplatOne());

			// Build the 3x3 rotation matrix that defines the box axes.
			XMMATRIX R = XMMatrixRotationQuaternion(BoxOrientation);

			// Check against each plane of the frustum.
			XMVECTOR Outside = XMVectorFalseInt();
			XMVECTOR InsideAll = XMVectorTrueInt();
			XMVECTOR CenterInsideAll = XMVectorTrueInt();

			for (size_t i = 0; i < 6; ++i)
			{
				// Compute the distance to the center of the box.
				XMVECTOR Dist = XMVector4Dot(Center, Planes[i]);

				// Project the axes of the box onto the normal of the plane.  Half the
				// length of the projection (sometime called the "radius") is equal to
				// h(u) * abs(n dot b(u))) + h(v) * abs(n dot b(v)) + h(w) * abs(n dot b(w))
				// where h(i) are extents of the box, n is the plane normal, and b(i) are the 
				// axes of the box.
				XMVECTOR Radius = XMVector3Dot(Planes[i], R.r[0]);
				Radius = XMVectorSelect(Radius, XMVector3Dot(Planes[i], R.r[1]), SelectY);
				Radius = XMVectorSelect(Radius, XMVector3Dot(Planes[i], R.r[2]), SelectZ);
				Radius = XMVector3Dot(Extents, XMVectorAbs(Radius));

				// Outside the plane?
				Outside = XMVectorOrInt(Outside, XMVectorGreater(Dist, Radius));

				// Fully inside the plane?
				InsideAll = XMVectorAndInt(InsideAll, XMVectorLessOrEqual(Dist, -Radius));

				// Check if the center is inside the plane.
				CenterInsideAll = XMVectorAndInt(CenterInsideAll, XMVectorLessOrEqual(Dist, Zero));
			}

			// If the box is outside any of the planes it is outside. 
			if (XMVector4EqualInt(Outside, XMVectorTrueInt()))
				return false;

			// If the box is inside all planes it is fully inside.
			if (XMVector4EqualInt(InsideAll, XMVectorTrueInt()))
				return true;

			// If the center of the box is inside all planes and the box intersects 
			// one or more planes then it must intersect.
			if (XMVector4EqualInt(CenterInsideAll, XMVectorTrueInt()))
				return true;

			// Build the corners of the frustum.
			XMVECTOR vRightTop = XMVectorSet(RightSlope, TopSlope, 1.0f, 0.0f);
			XMVECTOR vRightBottom = XMVectorSet(RightSlope, BottomSlope, 1.0f, 0.0f);
			XMVECTOR vLeftTop = XMVectorSet(LeftSlope, TopSlope, 1.0f, 0.0f);
			XMVECTOR vLeftBottom = XMVectorSet(LeftSlope, BottomSlope, 1.0f, 0.0f);
			XMVECTOR vNear = XMVectorReplicatePtr(&Near);
			XMVECTOR vFar = XMVectorReplicatePtr(&Far);

			XMVECTOR Corners[CORNER_COUNT];
			Corners[0] = vRightTop * vNear;
			Corners[1] = vRightBottom * vNear;
			Corners[2] = vLeftTop * vNear;
			Corners[3] = vLeftBottom * vNear;
			Corners[4] = vRightTop * vFar;
			Corners[5] = vRightBottom * vFar;
			Corners[6] = vLeftTop * vFar;
			Corners[7] = vLeftBottom * vFar;

			// Test against box axes (3)
			{
				// Find the min/max values of the projection of the frustum onto each axis.
				XMVECTOR FrustumMin, FrustumMax;

				FrustumMin = XMVector3Dot(Corners[0], R.r[0]);
				FrustumMin = XMVectorSelect(FrustumMin, XMVector3Dot(Corners[0], R.r[1]), SelectY);
				FrustumMin = XMVectorSelect(FrustumMin, XMVector3Dot(Corners[0], R.r[2]), SelectZ);
				FrustumMax = FrustumMin;

				for (size_t i = 1; i < BoundingOrientedBox::CORNER_COUNT; ++i)
				{
					XMVECTOR Temp = XMVector3Dot(Corners[i], R.r[0]);
					Temp = XMVectorSelect(Temp, XMVector3Dot(Corners[i], R.r[1]), SelectY);
					Temp = XMVectorSelect(Temp, XMVector3Dot(Corners[i], R.r[2]), SelectZ);

					FrustumMin = XMVectorMin(FrustumMin, Temp);
					FrustumMax = XMVectorMax(FrustumMax, Temp);
				}

				// Project the center of the box onto the axes.
				XMVECTOR BoxDist = XMVector3Dot(Center, R.r[0]);
				BoxDist = XMVectorSelect(BoxDist, XMVector3Dot(Center, R.r[1]), SelectY);
				BoxDist = XMVectorSelect(BoxDist, XMVector3Dot(Center, R.r[2]), SelectZ);

				// The projection of the box onto the axis is just its Center and Extents.
				// if (min > box_max || max < box_min) reject;
				XMVECTOR Result = XMVectorOrInt(XMVectorGreater(FrustumMin, BoxDist + Extents),
					XMVectorLess(FrustumMax, BoxDist - Extents));

				if (DirectX::Internal::XMVector3AnyTrue(Result))
					return false;
			}

			// Test against edge/edge axes (3*6).
			XMVECTOR FrustumEdgeAxis[6];

			FrustumEdgeAxis[0] = vRightTop;
			FrustumEdgeAxis[1] = vRightBottom;
			FrustumEdgeAxis[2] = vLeftTop;
			FrustumEdgeAxis[3] = vLeftBottom;
			FrustumEdgeAxis[4] = vRightTop - vLeftTop;
			FrustumEdgeAxis[5] = vLeftBottom - vLeftTop;

			for (size_t i = 0; i < 3; ++i)
			{
				for (size_t j = 0; j < 6; j++)
				{
					// Compute the axis we are going to test.
					XMVECTOR Axis = XMVector3Cross(R.r[i], FrustumEdgeAxis[j]);

					// Find the min/max values of the projection of the frustum onto the axis.
					XMVECTOR FrustumMin, FrustumMax;

					FrustumMin = FrustumMax = XMVector3Dot(Axis, Corners[0]);

					for (size_t k = 1; k < CORNER_COUNT; k++)
					{
						XMVECTOR Temp = XMVector3Dot(Axis, Corners[k]);
						FrustumMin = XMVectorMin(FrustumMin, Temp);
						FrustumMax = XMVectorMax(FrustumMax, Temp);
					}

					// Project the center of the box onto the axis.
					XMVECTOR Dist = XMVector3Dot(Center, Axis);

					// Project the axes of the box onto the axis to find the "radius" of the box.
					XMVECTOR Radius = XMVector3Dot(Axis, R.r[0]);
					Radius = XMVectorSelect(Radius, XMVector3Dot(Axis, R.r[1]), SelectY);
					Radius = XMVectorSelect(Radius, XMVector3Dot(Axis, R.r[2]), SelectZ);
					Radius = XMVector3Dot(Extents, XMVectorAbs(Radius));

					// if (center > max + radius || center < min - radius) reject;
					Outside = XMVectorOrInt(Outside, XMVectorGreater(Dist, FrustumMax + Radius));
					Outside = XMVectorOrInt(Outside, XMVectorLess(Dist, FrustumMin - Radius));
				}
			}

			if (XMVector4EqualInt(Outside, XMVectorTrueInt()))
				return false;

			// If we did not find a separating plane then the box must intersect the frustum.
			return true;
		}

		bool Frustum::Intersects(_In_ const Frustum& fr) const
		{
			using namespace DirectX;

			// Load origin and orientation of frustum B.
			XMVECTOR OriginB = Origin;
			XMVECTOR OrientationB = Orientation;

			assert(DirectX::Internal::XMQuaternionIsUnit(OrientationB));

			// Build the planes of frustum B.
			XMVECTOR AxisB[6];
			AxisB[0] = XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f);
			AxisB[1] = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
			AxisB[2] = XMVectorSet(1.0f, 0.0f, -RightSlope, 0.0f);
			AxisB[3] = XMVectorSet(-1.0f, 0.0f, LeftSlope, 0.0f);
			AxisB[4] = XMVectorSet(0.0f, 1.0f, -TopSlope, 0.0f);
			AxisB[5] = XMVectorSet(0.0f, -1.0f, BottomSlope, 0.0f);

			XMVECTOR PlaneDistB[6];
			PlaneDistB[0] = -XMVectorReplicatePtr(&Near);
			PlaneDistB[1] = XMVectorReplicatePtr(&Far);
			PlaneDistB[2] = XMVectorZero();
			PlaneDistB[3] = XMVectorZero();
			PlaneDistB[4] = XMVectorZero();
			PlaneDistB[5] = XMVectorZero();

			// Load origin and orientation of frustum A.
			XMVECTOR OriginA = fr.Origin;
			XMVECTOR OrientationA = fr.Orientation;

			assert(DirectX::Internal::XMQuaternionIsUnit(OrientationA));

			// Transform frustum A into the space of the frustum B in order to 
			// minimize the number of transforms we have to do.
			OriginA = XMVector3InverseRotate(OriginA - OriginB, OrientationB);
			OrientationA = XMQuaternionMultiply(OrientationA, XMQuaternionConjugate(OrientationB));

			// Build the corners of frustum A (in the local space of B).
			XMVECTOR RightTopA = XMVectorSet(fr.RightSlope, fr.TopSlope, 1.0f, 0.0f);
			XMVECTOR RightBottomA = XMVectorSet(fr.RightSlope, fr.BottomSlope, 1.0f, 0.0f);
			XMVECTOR LeftTopA = XMVectorSet(fr.LeftSlope, fr.TopSlope, 1.0f, 0.0f);
			XMVECTOR LeftBottomA = XMVectorSet(fr.LeftSlope, fr.BottomSlope, 1.0f, 0.0f);
			XMVECTOR NearA = XMVectorReplicatePtr(&fr.Near);
			XMVECTOR FarA = XMVectorReplicatePtr(&fr.Far);

			RightTopA = XMVector3Rotate(RightTopA, OrientationA);
			RightBottomA = XMVector3Rotate(RightBottomA, OrientationA);
			LeftTopA = XMVector3Rotate(LeftTopA, OrientationA);
			LeftBottomA = XMVector3Rotate(LeftBottomA, OrientationA);

			XMVECTOR CornersA[CORNER_COUNT];
			CornersA[0] = OriginA + RightTopA * NearA;
			CornersA[1] = OriginA + RightBottomA * NearA;
			CornersA[2] = OriginA + LeftTopA * NearA;
			CornersA[3] = OriginA + LeftBottomA * NearA;
			CornersA[4] = OriginA + RightTopA * FarA;
			CornersA[5] = OriginA + RightBottomA * FarA;
			CornersA[6] = OriginA + LeftTopA * FarA;
			CornersA[7] = OriginA + LeftBottomA * FarA;

			// Check frustum A against each plane of frustum B.
			XMVECTOR Outside = XMVectorFalseInt();
			XMVECTOR InsideAll = XMVectorTrueInt();

			for (size_t i = 0; i < 6; ++i)
			{
				// Find the min/max projection of the frustum onto the plane normal.
				XMVECTOR Min, Max;

				Min = Max = XMVector3Dot(AxisB[i], CornersA[0]);

				for (size_t j = 1; j < CORNER_COUNT; j++)
				{
					XMVECTOR Temp = XMVector3Dot(AxisB[i], CornersA[j]);
					Min = XMVectorMin(Min, Temp);
					Max = XMVectorMax(Max, Temp);
				}

				// Outside the plane?
				Outside = XMVectorOrInt(Outside, XMVectorGreater(Min, PlaneDistB[i]));

				// Fully inside the plane?
				InsideAll = XMVectorAndInt(InsideAll, XMVectorLessOrEqual(Max, PlaneDistB[i]));
			}

			// If the frustum A is outside any of the planes of frustum B it is outside. 
			if (XMVector4EqualInt(Outside, XMVectorTrueInt()))
				return false;

			// If frustum A is inside all planes of frustum B it is fully inside.
			if (XMVector4EqualInt(InsideAll, XMVectorTrueInt()))
				return true;

			// Build the corners of frustum B.
			XMVECTOR RightTopB = XMVectorSet(RightSlope, TopSlope, 1.0f, 0.0f);
			XMVECTOR RightBottomB = XMVectorSet(RightSlope, BottomSlope, 1.0f, 0.0f);
			XMVECTOR LeftTopB = XMVectorSet(LeftSlope, TopSlope, 1.0f, 0.0f);
			XMVECTOR LeftBottomB = XMVectorSet(LeftSlope, BottomSlope, 1.0f, 0.0f);
			XMVECTOR NearB = XMVectorReplicatePtr(&Near);
			XMVECTOR FarB = XMVectorReplicatePtr(&Far);

			XMVECTOR CornersB[BoundingFrustum::CORNER_COUNT];
			CornersB[0] = RightTopB * NearB;
			CornersB[1] = RightBottomB * NearB;
			CornersB[2] = LeftTopB * NearB;
			CornersB[3] = LeftBottomB * NearB;
			CornersB[4] = RightTopB * FarB;
			CornersB[5] = RightBottomB * FarB;
			CornersB[6] = LeftTopB * FarB;
			CornersB[7] = LeftBottomB * FarB;

			// Build the planes of frustum A (in the local space of B).
			XMVECTOR AxisA[6];
			XMVECTOR PlaneDistA[6];

			AxisA[0] = XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f);
			AxisA[1] = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
			AxisA[2] = XMVectorSet(1.0f, 0.0f, -fr.RightSlope, 0.0f);
			AxisA[3] = XMVectorSet(-1.0f, 0.0f, fr.LeftSlope, 0.0f);
			AxisA[4] = XMVectorSet(0.0f, 1.0f, -fr.TopSlope, 0.0f);
			AxisA[5] = XMVectorSet(0.0f, -1.0f, fr.BottomSlope, 0.0f);

			AxisA[0] = XMVector3Rotate(AxisA[0], OrientationA);
			AxisA[1] = -AxisA[0];
			AxisA[2] = XMVector3Rotate(AxisA[2], OrientationA);
			AxisA[3] = XMVector3Rotate(AxisA[3], OrientationA);
			AxisA[4] = XMVector3Rotate(AxisA[4], OrientationA);
			AxisA[5] = XMVector3Rotate(AxisA[5], OrientationA);

			PlaneDistA[0] = XMVector3Dot(AxisA[0], CornersA[0]);  // Re-use corner on near plane.
			PlaneDistA[1] = XMVector3Dot(AxisA[1], CornersA[4]);  // Re-use corner on far plane.
			PlaneDistA[2] = XMVector3Dot(AxisA[2], OriginA);
			PlaneDistA[3] = XMVector3Dot(AxisA[3], OriginA);
			PlaneDistA[4] = XMVector3Dot(AxisA[4], OriginA);
			PlaneDistA[5] = XMVector3Dot(AxisA[5], OriginA);

			// Check each axis of frustum A for a seperating plane (5).
			for (size_t i = 0; i < 6; ++i)
			{
				// Find the minimum projection of the frustum onto the plane normal.
				XMVECTOR Min;

				Min = XMVector3Dot(AxisA[i], CornersB[0]);

				for (size_t j = 1; j < CORNER_COUNT; j++)
				{
					XMVECTOR Temp = XMVector3Dot(AxisA[i], CornersB[j]);
					Min = XMVectorMin(Min, Temp);
				}

				// Outside the plane?
				Outside = XMVectorOrInt(Outside, XMVectorGreater(Min, PlaneDistA[i]));
			}

			// If the frustum B is outside any of the planes of frustum A it is outside. 
			if (XMVector4EqualInt(Outside, XMVectorTrueInt()))
				return false;

			// Check edge/edge axes (6 * 6).
			XMVECTOR FrustumEdgeAxisA[6];
			FrustumEdgeAxisA[0] = RightTopA;
			FrustumEdgeAxisA[1] = RightBottomA;
			FrustumEdgeAxisA[2] = LeftTopA;
			FrustumEdgeAxisA[3] = LeftBottomA;
			FrustumEdgeAxisA[4] = RightTopA - LeftTopA;
			FrustumEdgeAxisA[5] = LeftBottomA - LeftTopA;

			XMVECTOR FrustumEdgeAxisB[6];
			FrustumEdgeAxisB[0] = RightTopB;
			FrustumEdgeAxisB[1] = RightBottomB;
			FrustumEdgeAxisB[2] = LeftTopB;
			FrustumEdgeAxisB[3] = LeftBottomB;
			FrustumEdgeAxisB[4] = RightTopB - LeftTopB;
			FrustumEdgeAxisB[5] = LeftBottomB - LeftTopB;

			for (size_t i = 0; i < 6; ++i)
			{
				for (size_t j = 0; j < 6; j++)
				{
					// Compute the axis we are going to test.
					XMVECTOR Axis = XMVector3Cross(FrustumEdgeAxisA[i], FrustumEdgeAxisB[j]);

					// Find the min/max values of the projection of both frustums onto the axis.
					XMVECTOR MinA, MaxA;
					XMVECTOR MinB, MaxB;

					MinA = MaxA = XMVector3Dot(Axis, CornersA[0]);
					MinB = MaxB = XMVector3Dot(Axis, CornersB[0]);

					for (size_t k = 1; k < CORNER_COUNT; k++)
					{
						XMVECTOR TempA = XMVector3Dot(Axis, CornersA[k]);
						MinA = XMVectorMin(MinA, TempA);
						MaxA = XMVectorMax(MaxA, TempA);

						XMVECTOR TempB = XMVector3Dot(Axis, CornersB[k]);
						MinB = XMVectorMin(MinB, TempB);
						MaxB = XMVectorMax(MaxB, TempB);
					}

					// if (MinA > MaxB || MinB > MaxA) reject
					Outside = XMVectorOrInt(Outside, XMVectorGreater(MinA, MaxB));
					Outside = XMVectorOrInt(Outside, XMVectorGreater(MinB, MaxA));
				}
			}

			// If there is a seperating plane, then the frustums do not intersect.
			if (XMVector4EqualInt(Outside, XMVectorTrueInt()))
				return false;

			// If we did not find a separating plane then the frustums intersect.
			return true;
		}

		bool __vectorcall Frustum::Intersects(_In_ const Math::Vector3& v0, _In_ const Math::Vector3& v1, _In_ const Math::Vector3& v2) const
		{
			using namespace DirectX;

			XMVECTOR V0 = v0;
			XMVECTOR V1 = v1;
			XMVECTOR V2 = v2;

			// Build the frustum planes (NOTE: D is negated from the usual).
			XMVECTOR Planes[6];
			Planes[0] = XMVectorSet(0.0f, 0.0f, -1.0f, -Near);
			Planes[1] = XMVectorSet(0.0f, 0.0f, 1.0f, Far);
			Planes[2] = XMVectorSet(1.0f, 0.0f, -RightSlope, 0.0f);
			Planes[3] = XMVectorSet(-1.0f, 0.0f, LeftSlope, 0.0f);
			Planes[4] = XMVectorSet(0.0f, 1.0f, -TopSlope, 0.0f);
			Planes[5] = XMVectorSet(0.0f, -1.0f, BottomSlope, 0.0f);

			// Load origin and orientation of the frustum.
			XMVECTOR vOrigin = Origin;
			XMVECTOR vOrientation = Orientation;

			assert(DirectX::Internal::XMQuaternionIsUnit(vOrientation));

			// Transform triangle into the local space of frustum.
			XMVECTOR TV0 = XMVector3InverseRotate(V0 - vOrigin, vOrientation);
			XMVECTOR TV1 = XMVector3InverseRotate(V1 - vOrigin, vOrientation);
			XMVECTOR TV2 = XMVector3InverseRotate(V2 - vOrigin, vOrientation);

			// Test each vertex of the triangle against the frustum planes.
			XMVECTOR Outside = XMVectorFalseInt();
			XMVECTOR InsideAll = XMVectorTrueInt();

			for (size_t i = 0; i < 6; ++i)
			{
				XMVECTOR Dist0 = XMVector3Dot(TV0, Planes[i]);
				XMVECTOR Dist1 = XMVector3Dot(TV1, Planes[i]);
				XMVECTOR Dist2 = XMVector3Dot(TV2, Planes[i]);

				XMVECTOR MinDist = XMVectorMin(Dist0, Dist1);
				MinDist = XMVectorMin(MinDist, Dist2);
				XMVECTOR MaxDist = XMVectorMax(Dist0, Dist1);
				MaxDist = XMVectorMax(MaxDist, Dist2);

				XMVECTOR PlaneDist = XMVectorSplatW(Planes[i]);

				// Outside the plane?
				Outside = XMVectorOrInt(Outside, XMVectorGreater(MinDist, PlaneDist));

				// Fully inside the plane?
				InsideAll = XMVectorAndInt(InsideAll, XMVectorLessOrEqual(MaxDist, PlaneDist));
			}

			// If the triangle is outside any of the planes it is outside. 
			if (XMVector4EqualInt(Outside, XMVectorTrueInt()))
				return false;

			// If the triangle is inside all planes it is fully inside.
			if (XMVector4EqualInt(InsideAll, XMVectorTrueInt()))
				return true;

			// Build the corners of the frustum.
			XMVECTOR vRightTop = XMVectorSet(RightSlope, TopSlope, 1.0f, 0.0f);
			XMVECTOR vRightBottom = XMVectorSet(RightSlope, BottomSlope, 1.0f, 0.0f);
			XMVECTOR vLeftTop = XMVectorSet(LeftSlope, TopSlope, 1.0f, 0.0f);
			XMVECTOR vLeftBottom = XMVectorSet(LeftSlope, BottomSlope, 1.0f, 0.0f);
			XMVECTOR vNear = XMVectorReplicatePtr(&Near);
			XMVECTOR vFar = XMVectorReplicatePtr(&Far);

			XMVECTOR Corners[CORNER_COUNT];
			Corners[0] = vRightTop * vNear;
			Corners[1] = vRightBottom * vNear;
			Corners[2] = vLeftTop * vNear;
			Corners[3] = vLeftBottom * vNear;
			Corners[4] = vRightTop * vFar;
			Corners[5] = vRightBottom * vFar;
			Corners[6] = vLeftTop * vFar;
			Corners[7] = vLeftBottom * vFar;

			// Test the plane of the triangle.
			XMVECTOR Normal = XMVector3Cross(V1 - V0, V2 - V0);
			XMVECTOR Dist = XMVector3Dot(Normal, V0);

			XMVECTOR MinDist, MaxDist;
			MinDist = MaxDist = XMVector3Dot(Corners[0], Normal);
			for (size_t i = 1; i < CORNER_COUNT; ++i)
			{
				XMVECTOR Temp = XMVector3Dot(Corners[i], Normal);
				MinDist = XMVectorMin(MinDist, Temp);
				MaxDist = XMVectorMax(MaxDist, Temp);
			}

			Outside = XMVectorOrInt(XMVectorGreater(MinDist, Dist), XMVectorLess(MaxDist, Dist));
			if (XMVector4EqualInt(Outside, XMVectorTrueInt()))
				return false;

			// Check the edge/edge axes (3*6).
			XMVECTOR TriangleEdgeAxis[3];
			TriangleEdgeAxis[0] = V1 - V0;
			TriangleEdgeAxis[1] = V2 - V1;
			TriangleEdgeAxis[2] = V0 - V2;

			XMVECTOR FrustumEdgeAxis[6];
			FrustumEdgeAxis[0] = vRightTop;
			FrustumEdgeAxis[1] = vRightBottom;
			FrustumEdgeAxis[2] = vLeftTop;
			FrustumEdgeAxis[3] = vLeftBottom;
			FrustumEdgeAxis[4] = vRightTop - vLeftTop;
			FrustumEdgeAxis[5] = vLeftBottom - vLeftTop;

			for (size_t i = 0; i < 3; ++i)
			{
				for (size_t j = 0; j < 6; j++)
				{
					// Compute the axis we are going to test.
					XMVECTOR Axis = XMVector3Cross(TriangleEdgeAxis[i], FrustumEdgeAxis[j]);

					// Find the min/max of the projection of the triangle onto the axis.
					XMVECTOR MinA, MaxA;

					XMVECTOR Dist0 = XMVector3Dot(V0, Axis);
					XMVECTOR Dist1 = XMVector3Dot(V1, Axis);
					XMVECTOR Dist2 = XMVector3Dot(V2, Axis);

					MinA = XMVectorMin(Dist0, Dist1);
					MinA = XMVectorMin(MinA, Dist2);
					MaxA = XMVectorMax(Dist0, Dist1);
					MaxA = XMVectorMax(MaxA, Dist2);

					// Find the min/max of the projection of the frustum onto the axis.
					XMVECTOR MinB, MaxB;

					MinB = MaxB = XMVector3Dot(Axis, Corners[0]);

					for (size_t k = 1; k < CORNER_COUNT; k++)
					{
						XMVECTOR Temp = XMVector3Dot(Axis, Corners[k]);
						MinB = XMVectorMin(MinB, Temp);
						MaxB = XMVectorMax(MaxB, Temp);
					}

					// if (MinA > MaxB || MinB > MaxA) reject;
					Outside = XMVectorOrInt(Outside, XMVectorGreater(MinA, MaxB));
					Outside = XMVectorOrInt(Outside, XMVectorGreater(MinB, MaxA));
				}
			}

			if (XMVector4EqualInt(Outside, XMVectorTrueInt()))
				return false;

			// If we did not find a separating plane then the triangle must intersect the frustum.
			return true;
		}

		EmPlaneIntersection::Type __vectorcall Frustum::Intersects(_In_ const Math::Plane& Plane) const
		{
			using namespace DirectX;

			assert(DirectX::Internal::XMPlaneIsUnit(Plane));

			// Load origin and orientation of the frustum.
			XMVECTOR vOrigin = Origin;
			XMVECTOR vOrientation = Orientation;

			assert(DirectX::Internal::XMQuaternionIsUnit(vOrientation));

			// Set w of the origin to one so we can dot4 with a plane.
			vOrigin = XMVectorInsert<0, 0, 0, 0, 1>(vOrigin, XMVectorSplatOne());

			// Build the corners of the frustum (in world space).
			XMVECTOR RightTop = XMVectorSet(RightSlope, TopSlope, 1.0f, 0.0f);
			XMVECTOR RightBottom = XMVectorSet(RightSlope, BottomSlope, 1.0f, 0.0f);
			XMVECTOR LeftTop = XMVectorSet(LeftSlope, TopSlope, 1.0f, 0.0f);
			XMVECTOR LeftBottom = XMVectorSet(LeftSlope, BottomSlope, 1.0f, 0.0f);
			XMVECTOR vNear = XMVectorReplicatePtr(&Near);
			XMVECTOR vFar = XMVectorReplicatePtr(&Far);

			RightTop = XMVector3Rotate(RightTop, vOrientation);
			RightBottom = XMVector3Rotate(RightBottom, vOrientation);
			LeftTop = XMVector3Rotate(LeftTop, vOrientation);
			LeftBottom = XMVector3Rotate(LeftBottom, vOrientation);

			XMVECTOR Corners0 = vOrigin + RightTop * vNear;
			XMVECTOR Corners1 = vOrigin + RightBottom * vNear;
			XMVECTOR Corners2 = vOrigin + LeftTop * vNear;
			XMVECTOR Corners3 = vOrigin + LeftBottom * vNear;
			XMVECTOR Corners4 = vOrigin + RightTop * vFar;
			XMVECTOR Corners5 = vOrigin + RightBottom * vFar;
			XMVECTOR Corners6 = vOrigin + LeftTop * vFar;
			XMVECTOR Corners7 = vOrigin + LeftBottom * vFar;

			XMVECTOR Outside, Inside;
			DirectX::Internal::FastIntersectFrustumPlane(Corners0, Corners1, Corners2, Corners3,
				Corners4, Corners5, Corners6, Corners7,
				Plane, Outside, Inside);

			// If the frustum is outside any plane it is outside.
			if (XMVector4EqualInt(Outside, XMVectorTrueInt()))
				return EmPlaneIntersection::eFront;

			// If the frustum is inside all planes it is inside.
			if (XMVector4EqualInt(Inside, XMVectorTrueInt()))
				return EmPlaneIntersection::eBack;

			// The frustum is not inside all planes or outside a plane it intersects.
			return EmPlaneIntersection::eIntersecting;
		}

		bool __vectorcall Frustum::Intersects(_In_ const Math::Vector3& rayOrigin, _In_ const Math::Vector3& Direction, _Out_ float& Dist) const
		{
			using namespace DirectX;

			// If ray starts inside the frustum, return a distance of 0 for the hit
			if (Contains(rayOrigin) == CONTAINS)
			{
				Dist = 0.0f;
				return true;
			}

			// Build the frustum planes.
			XMVECTOR Planes[6];
			Planes[0] = XMVectorSet(0.0f, 0.0f, -1.0f, Near);
			Planes[1] = XMVectorSet(0.0f, 0.0f, 1.0f, -Far);
			Planes[2] = XMVectorSet(1.0f, 0.0f, -RightSlope, 0.0f);
			Planes[3] = XMVectorSet(-1.0f, 0.0f, LeftSlope, 0.0f);
			Planes[4] = XMVectorSet(0.0f, 1.0f, -TopSlope, 0.0f);
			Planes[5] = XMVectorSet(0.0f, -1.0f, BottomSlope, 0.0f);

			// Load origin and orientation of the frustum.
			XMVECTOR frOrigin = Origin;
			XMVECTOR frOrientation = Orientation;

			// This algorithm based on "Fast Ray-Convex Polyhedron Intersectin," in James Arvo, ed., Graphics Gems II pp. 247-250
			float tnear = -FLT_MAX;
			float tfar = FLT_MAX;

			for (size_t i = 0; i < 6; ++i)
			{
				XMVECTOR Plane = DirectX::Internal::XMPlaneTransform(Planes[i], frOrientation, frOrigin);
				Plane = XMPlaneNormalize(Plane);

				XMVECTOR AxisDotOrigin = XMPlaneDotCoord(Plane, rayOrigin);
				XMVECTOR AxisDotDirection = XMVector3Dot(Plane, Direction);

				if (XMVector3LessOrEqual(XMVectorAbs(AxisDotDirection), g_RayEpsilon))
				{
					// Ray is parallel to plane - check if ray origin is inside plane's
					if (XMVector3Greater(AxisDotOrigin, g_XMZero))
					{
						// Ray origin is outside half-space.
						Dist = 0.f;
						return false;
					}
				}
				else
				{
					// Ray not parallel - get distance to plane.
					float vd = XMVectorGetX(AxisDotDirection);
					float vn = XMVectorGetX(AxisDotOrigin);
					float t = -vn / vd;
					if (vd < 0.0f)
					{
						// Front face - T is a near point.
						if (t > tfar)
						{
							Dist = 0.f;
							return false;
						}
						if (t > tnear)
						{
							// Hit near face.
							tnear = t;
						}
					}
					else
					{
						// back face - T is far point.
						if (t < tnear)
						{
							Dist = 0.f;
							return false;
						}
						if (t < tfar)
						{
							// Hit far face.
							tfar = t;
						}
					}
				}
			}

			// Survived all tests.
			// Note: if ray originates on polyhedron, may want to change 0.0f to some
			// epsilon to avoid intersecting the originating face.
			float distance = (tnear >= 0.0f) ? tnear : tfar;
			if (distance >= 0.0f)
			{
				Dist = distance;
				return true;
			}

			Dist = 0.f;
			return false;
		}

		EmContainment::Type __vectorcall Frustum::ContainedBy(_In_ const Math::Plane& Plane0, _In_ const Math::Plane& Plane1, _In_ const Math::Plane& Plane2,
			_In_ const Math::Plane& Plane3, _In_ const Math::Plane& Plane4, _In_ const Math::Plane& Plane5) const
		{
			using namespace DirectX;

			// Load origin and orientation of the frustum.
			XMVECTOR vOrigin = Origin;
			XMVECTOR vOrientation = Orientation;

			assert(DirectX::Internal::XMQuaternionIsUnit(vOrientation));

			// Set w of the origin to one so we can dot4 with a plane.
			vOrigin = XMVectorInsert<0, 0, 0, 0, 1>(vOrigin, XMVectorSplatOne());

			// Build the corners of the frustum (in world space).
			XMVECTOR RightTop = XMVectorSet(RightSlope, TopSlope, 1.0f, 0.0f);
			XMVECTOR RightBottom = XMVectorSet(RightSlope, BottomSlope, 1.0f, 0.0f);
			XMVECTOR LeftTop = XMVectorSet(LeftSlope, TopSlope, 1.0f, 0.0f);
			XMVECTOR LeftBottom = XMVectorSet(LeftSlope, BottomSlope, 1.0f, 0.0f);
			XMVECTOR vNear = XMVectorReplicatePtr(&Near);
			XMVECTOR vFar = XMVectorReplicatePtr(&Far);

			RightTop = XMVector3Rotate(RightTop, vOrientation);
			RightBottom = XMVector3Rotate(RightBottom, vOrientation);
			LeftTop = XMVector3Rotate(LeftTop, vOrientation);
			LeftBottom = XMVector3Rotate(LeftBottom, vOrientation);

			XMVECTOR Corners0 = vOrigin + RightTop * vNear;
			XMVECTOR Corners1 = vOrigin + RightBottom * vNear;
			XMVECTOR Corners2 = vOrigin + LeftTop * vNear;
			XMVECTOR Corners3 = vOrigin + LeftBottom * vNear;
			XMVECTOR Corners4 = vOrigin + RightTop * vFar;
			XMVECTOR Corners5 = vOrigin + RightBottom * vFar;
			XMVECTOR Corners6 = vOrigin + LeftTop * vFar;
			XMVECTOR Corners7 = vOrigin + LeftBottom * vFar;

			XMVECTOR Outside, Inside;

			// Test against each plane.
			DirectX::Internal::FastIntersectFrustumPlane(Corners0, Corners1, Corners2, Corners3,
				Corners4, Corners5, Corners6, Corners7,
				Plane0, Outside, Inside);

			XMVECTOR AnyOutside = Outside;
			XMVECTOR AllInside = Inside;

			DirectX::Internal::FastIntersectFrustumPlane(Corners0, Corners1, Corners2, Corners3,
				Corners4, Corners5, Corners6, Corners7,
				Plane1, Outside, Inside);

			AnyOutside = XMVectorOrInt(AnyOutside, Outside);
			AllInside = XMVectorAndInt(AllInside, Inside);

			DirectX::Internal::FastIntersectFrustumPlane(Corners0, Corners1, Corners2, Corners3,
				Corners4, Corners5, Corners6, Corners7,
				Plane2, Outside, Inside);

			AnyOutside = XMVectorOrInt(AnyOutside, Outside);
			AllInside = XMVectorAndInt(AllInside, Inside);

			DirectX::Internal::FastIntersectFrustumPlane(Corners0, Corners1, Corners2, Corners3,
				Corners4, Corners5, Corners6, Corners7,
				Plane3, Outside, Inside);

			AnyOutside = XMVectorOrInt(AnyOutside, Outside);
			AllInside = XMVectorAndInt(AllInside, Inside);

			DirectX::Internal::FastIntersectFrustumPlane(Corners0, Corners1, Corners2, Corners3,
				Corners4, Corners5, Corners6, Corners7,
				Plane4, Outside, Inside);

			AnyOutside = XMVectorOrInt(AnyOutside, Outside);
			AllInside = XMVectorAndInt(AllInside, Inside);

			DirectX::Internal::FastIntersectFrustumPlane(Corners0, Corners1, Corners2, Corners3,
				Corners4, Corners5, Corners6, Corners7,
				Plane5, Outside, Inside);

			AnyOutside = XMVectorOrInt(AnyOutside, Outside);
			AllInside = XMVectorAndInt(AllInside, Inside);

			// If the frustum is outside any plane it is outside.
			if (XMVector4EqualInt(AnyOutside, XMVectorTrueInt()))
				return EmContainment::eDisjoint;

			// If the frustum is inside all planes it is inside.
			if (XMVector4EqualInt(AllInside, XMVectorTrueInt()))
				return EmContainment::eContains;

			// The frustum is not inside all planes or outside a plane, it may intersect.
			return EmContainment::eIntersects;
		}
		// Test frustum against six planes (see Frustum::GetPlanes)

		void Frustum::GetPlanes(_Out_opt_ Math::Plane* NearPlane, _Out_opt_ Math::Plane* FarPlane, _Out_opt_ Math::Plane* RightPlane,
			_Out_opt_ Math::Plane* LeftPlane, _Out_opt_ Math::Plane* TopPlane, _Out_opt_ Math::Plane* BottomPlane) const
		{
			using namespace DirectX;

			// Load origin and orientation of the frustum.
			XMVECTOR vOrigin = Origin;
			XMVECTOR vOrientation = Orientation;

			if (NearPlane)
			{
				XMVECTOR vNearPlane = XMVectorSet(0.0f, 0.0f, -1.0f, Near);
				vNearPlane = DirectX::Internal::XMPlaneTransform(vNearPlane, vOrientation, vOrigin);
				*NearPlane = XMPlaneNormalize(vNearPlane);
			}

			if (FarPlane)
			{
				XMVECTOR vFarPlane = XMVectorSet(0.0f, 0.0f, 1.0f, -Far);
				vFarPlane = DirectX::Internal::XMPlaneTransform(vFarPlane, vOrientation, vOrigin);
				*FarPlane = XMPlaneNormalize(vFarPlane);
			}

			if (RightPlane)
			{
				XMVECTOR vRightPlane = XMVectorSet(1.0f, 0.0f, -RightSlope, 0.0f);
				vRightPlane = DirectX::Internal::XMPlaneTransform(vRightPlane, vOrientation, vOrigin);
				*RightPlane = XMPlaneNormalize(vRightPlane);
			}

			if (LeftPlane)
			{
				XMVECTOR vLeftPlane = XMVectorSet(-1.0f, 0.0f, LeftSlope, 0.0f);
				vLeftPlane = DirectX::Internal::XMPlaneTransform(vLeftPlane, vOrientation, vOrigin);
				*LeftPlane = XMPlaneNormalize(vLeftPlane);
			}

			if (TopPlane)
			{
				XMVECTOR vTopPlane = XMVectorSet(0.0f, 1.0f, -TopSlope, 0.0f);
				vTopPlane = DirectX::Internal::XMPlaneTransform(vTopPlane, vOrientation, vOrigin);
				*TopPlane = XMPlaneNormalize(vTopPlane);
			}

			if (BottomPlane)
			{
				XMVECTOR vBottomPlane = XMVectorSet(0.0f, -1.0f, BottomSlope, 0.0f);
				vBottomPlane = DirectX::Internal::XMPlaneTransform(vBottomPlane, vOrientation, vOrigin);
				*BottomPlane = XMPlaneNormalize(vBottomPlane);
			}
		}

		// Static methods
		void __vectorcall Frustum::CreateFromMatrix(_Out_ Frustum& Out, _In_ const Math::Matrix& Projection)
		{
			using namespace DirectX;

			// Corners of the projection frustum in homogenous space.
			static XMVECTORF32 HomogenousPoints[6] =
			{
				{ 1.0f,  0.0f, 1.0f, 1.0f },   // right (at far plane)
				{ -1.0f,  0.0f, 1.0f, 1.0f },   // left
				{ 0.0f,  1.0f, 1.0f, 1.0f },   // top
				{ 0.0f, -1.0f, 1.0f, 1.0f },   // bottom

				{ 0.0f, 0.0f, 0.0f, 1.0f },     // near
				{ 0.0f, 0.0f, 1.0f, 1.0f }      // far
			};

			XMMATRIX matProjection = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(&Projection));

			XMVECTOR Determinant;
			XMMATRIX matInverse = XMMatrixInverse(&Determinant, matProjection);

			// Compute the frustum corners in world space.
			XMVECTOR Points[6];

			for (size_t i = 0; i < 6; ++i)
			{
				// Transform point.
				Points[i] = XMVector4Transform(HomogenousPoints[i], matInverse);
			}

			Out.Origin = Math::Vector3::Zero;
			Out.Orientation = Math::Quaternion::Identity;

			// Compute the slopes.
			Points[0] = Points[0] * XMVectorReciprocal(XMVectorSplatZ(Points[0]));
			Points[1] = Points[1] * XMVectorReciprocal(XMVectorSplatZ(Points[1]));
			Points[2] = Points[2] * XMVectorReciprocal(XMVectorSplatZ(Points[2]));
			Points[3] = Points[3] * XMVectorReciprocal(XMVectorSplatZ(Points[3]));

			Out.RightSlope = XMVectorGetX(Points[0]);
			Out.LeftSlope = XMVectorGetX(Points[1]);
			Out.TopSlope = XMVectorGetY(Points[2]);
			Out.BottomSlope = XMVectorGetY(Points[3]);

			// Compute near and far.
			Points[4] = Points[4] * XMVectorReciprocal(XMVectorSplatW(Points[4]));
			Points[5] = Points[5] * XMVectorReciprocal(XMVectorSplatW(Points[5]));

			Out.Near = XMVectorGetZ(Points[4]);
			Out.Far = XMVectorGetZ(Points[5]);
		}

		Ray::Ray() : position(0.f, 0.f, 0.f), direction(0.f, 0.f, 1.f) {}
		Ray::Ray(const Math::Vector3& pos, const Math::Vector3& dir) : position(pos), direction(dir) {}

		bool Ray::operator == (const Ray& r) const
		{
			using namespace DirectX;
			XMVECTOR r1p = position;
			XMVECTOR r2p = r.position;
			XMVECTOR r1d = direction;
			XMVECTOR r2d = r.direction;
			return XMVector3Equal(r1p, r2p) && XMVector3Equal(r1d, r2d);
		}

		bool Ray::operator != (const Ray& r) const
		{
			using namespace DirectX;
			XMVECTOR r1p = position;
			XMVECTOR r2p = r.position;
			XMVECTOR r1d = direction;
			XMVECTOR r2d = r.direction;
			return XMVector3NotEqual(r1p, r2p) && XMVector3NotEqual(r1d, r2d);
		}

		bool Ray::Intersects(const Sphere& sphere, _Out_ float& Dist) const
		{
			return sphere.Intersects(position, direction, Dist);
		}

		bool Ray::Intersects(const AABB& box, _Out_ float& Dist) const
		{
			return box.Intersects(position, direction, Dist);
		}

		bool Ray::Intersects(const Math::Vector3& tri0, const Math::Vector3& tri1, const Math::Vector3& tri2, _Out_ float& Dist) const
		{
			return DirectX::TriangleTests::Intersects(position, direction, tri0, tri1, tri2, Dist);
		}

		bool Ray::Intersects(const Math::Plane& plane, _Out_ float& Dist) const
		{
			using namespace DirectX;

			XMVECTOR p = plane;
			XMVECTOR dir = direction;

			XMVECTOR nd = XMPlaneDotNormal(p, dir);

			if (XMVector3LessOrEqual(XMVectorAbs(nd), g_RayEpsilon))
			{
				Dist = 0.f;
				return false;
			}
			else
			{
				// t = -(dot(n,origin) + D) / dot(n,dir)
				XMVECTOR pos = position;
				XMVECTOR v = XMPlaneDotNormal(p, pos);
				v = XMVectorAdd(v, XMVectorSplatW(p));
				v = XMVectorDivide(v, nd);
				float dist = -XMVectorGetX(v);
				if (dist < 0)
				{
					Dist = 0.f;
					return false;
				}
				else
				{
					Dist = dist;
					return true;
				}
			}
		}

		//-----------------------------------------------------------------------------
		// TriangleTests Function
		//-----------------------------------------------------------------------------
		bool __vectorcall TriangleTests::Intersects(_In_ const Math::Vector3& Origin, _In_ const Math::Vector3& Direction, _In_ const Math::Vector3& V0, _In_ const Math::Vector3& V1, _In_ const Math::Vector3& V2, _Out_ float& Dist)
		{
			return DirectX::TriangleTests::Intersects(Origin, Direction, V0, V1, V2, Dist);
		}
		// Ray-Triangle

		bool __vectorcall TriangleTests::Intersects(_In_ const Math::Vector3& A0, _In_ const Math::Vector3& A1, _In_ const Math::Vector3& A2, _In_ const Math::Vector3& B0, _In_ const Math::Vector3& B1, _In_ const Math::Vector3& B2)
		{
			return DirectX::TriangleTests::Intersects(A0, A1, A2, B0, B1, B2);
		}
		// Triangle-Triangle

		EmPlaneIntersection::Type __vectorcall TriangleTests::Intersects(_In_ const Math::Vector3& V0, _In_ const Math::Vector3& V1, _In_ const Math::Vector3& V2, _In_ const Math::Plane& Plane)
		{
			return static_cast<EmPlaneIntersection::Type>(DirectX::TriangleTests::Intersects(V0, V1, V2, Plane));
		}
		// Plane-Triangle

		EmContainment::Type __vectorcall TriangleTests::ContainedBy(_In_ const Math::Vector3& V0, _In_ const Math::Vector3& V1, _In_ const Math::Vector3& V2,
			_In_ const Math::Vector3& Plane0, _In_ const Math::Vector3& Plane1, _In_ const Math::Vector3& Plane2,
			_In_ const Math::Vector3& Plane3, _In_ const Math::Vector3& Plane4, _In_ const Math::Vector3& Plane5)
		{
			return static_cast<EmContainment::Type>(DirectX::TriangleTests::ContainedBy(V0, V1, V2, Plane0, Plane1, Plane2, Plane3, Plane4, Plane5));
		}
		// Test a triangle against six planes at once (see Frustum::GetPlanes)
	}
}