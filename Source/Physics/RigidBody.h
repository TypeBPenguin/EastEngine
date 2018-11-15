#pragma once

#include "PhysicsDefine.h"

class btRigidBody;

namespace eastengine
{
	namespace physics
	{
		class RigidBody;
		struct CollisionResult
		{
			const RigidBody* pOpponentObject = nullptr;
			const math::float3 f3OpponentPoint;
			const math::float3 f3MyPoint;

			CollisionResult(const RigidBody* pOpponentObject, const math::float3& f3OpponentPoint, const math::float3& f3MyPoint)
				: pOpponentObject(pOpponentObject)
				, f3OpponentPoint(f3OpponentPoint)
				, f3MyPoint(f3MyPoint)
			{
			}
		};
		using FuncCollisionCallback = void(*)(const std::vector<CollisionResult>&);

		using FuncTriangleDrawCallback = std::function<void(const math::float3* pTriangles, const size_t nCount)>;

		struct RigidBodyProperty
		{
			float fMass = 1.f;				// 무게

			float fRestitution = 0.5f;		// 탄성계수
			float fFriction = 0.5f;			// 마찰력
			float fLinearDamping = 0.f;		// 공기 저항
			float fAngularDamping = 0.f;	// 회전 저항(?)

			int nCollisionFlag = EmCollision::eStaticObject;

			short nGroup = 0;
			short nMask = 0;

			FuncCollisionCallback funcCollisionCallback = nullptr;
			FuncTriangleDrawCallback funcTriangleDrawCallback = nullptr;

			string::StringID strName;

			math::float3 f3OriginPos;
			math::Quaternion originQuat;
			math::Matrix matOffset;

			Shape shapeInfo;

			EmPhysicsShape::Type GetShapeType() const { return shapeInfo.emPhysicsShapeType; }
		};

		class RigidBody
		{
		private:
			RigidBody();

		public:
			~RigidBody();

			static RigidBody* Create(const RigidBodyProperty& physicsProperty);

			void Update(float fElapsedTime);
			void UpdateBoundingBox(const math::Matrix& matWorld);

			bool IsCollision(RigidBody* pRigidBody);
			bool RayTest(const math::float3& f3From, const math::float3& f3To, math::float3* pHitPoint_out = nullptr, math::float3* pHitNormal_out = nullptr) const;

			void AddCollisionResult(const RigidBody* pRigidBody, const math::float3& f3OpponentPoint, const math::float3& f3MyPoint);
			void ClearCollisionResults();
			void SetEnableTriangleDrawCallback(bool isEnableTriangleDrawCallback);

		public:
			const string::StringID& GetName() const;
			void SetName(const string::StringID& strName);

			const RigidBodyProperty& GetRigidBodyProperty() const;

			void SetWorldMatrix(const math::Matrix& mat);
			math::Matrix GetWorldMatrix() const;

			math::Quaternion GetOrientation() const;
			math::float3 GetCenterOfMassPosition() const;

			void SetLinearVelocity(const math::float3& f3Velocity);

			void SetDamping(float fLinearDamping, float fAngularDamping);
			void SetDeactivationTime(float fTime);
			void SetSleepingThresholds(float fLinear, float fAngular);
			void SetAngularFactor(float fFactor);

			void SetActiveState(EmActiveState::Type emActiveState);
			void SetGravity(bool isEnable);
			void SetGravity(const math::float3& f3Gravity);

			const Collision::AABB& GetAABB() const;
			const Collision::Sphere& GetBoundingSphere() const;
			const Collision::OBB& GetOBB() const;

			btRigidBody* GetInterface();

		private:
			bool Initislize(const RigidBodyProperty& physicsProperty);

		private:
			class Impl;
			std::unique_ptr<Impl> m_pImpl;
		};
	}
}