#pragma once

#include "PhysicsDefine.h"

class btDiscreteDynamicsWorld;
class btCollisionShape;
class btRigidBody;
class btTriangleMesh;
class btTriangleIndexVertexArray;
class btMotionState;

namespace EastEngine
{
	namespace Physics
	{
		class RigidBody;
		struct CollisionResult
		{
			const RigidBody* pOpponentObject = nullptr;
			const Math::Vector3 f3OpponentPoint;
			const Math::Vector3 f3MyPoint;

			CollisionResult(const RigidBody* pOpponentObject, const Math::Vector3& f3OpponentPoint, const Math::Vector3& f3MyPoint)
				: pOpponentObject(pOpponentObject)
				, f3OpponentPoint(f3OpponentPoint)
				, f3MyPoint(f3MyPoint)
			{
			}
		};
		typedef void(*FuncCollisionCallback)(const std::vector<CollisionResult>&);

		using FuncTriangleDrawCallback = std::function<void(const Math::Vector3* pTriangles, const size_t nCount)>;

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

			String::StringID strName;

			Math::Vector3 f3OriginPos;
			Math::Quaternion originQuat;
			Math::Matrix matOffset;

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
			void UpdateBoundingBox(const Math::Matrix& matWorld);

			bool IsCollision(const RigidBody* pRigidBody);
			bool RayTest(const Math::Vector3& f3From, const Math::Vector3& f3To, Math::Vector3* pHitPoint_out = nullptr, Math::Vector3* pHitNormal_out = nullptr);

			void AddCollisionResult(const RigidBody* pRigidBody, const Math::Vector3& f3OpponentPoint, const Math::Vector3& f3MyPoint)
			{
				if (m_rigidBodyProperty.funcCollisionCallback != nullptr)
				{
					m_vecCollisionResults.emplace_back(pRigidBody, f3OpponentPoint, f3MyPoint);
				}
			}

			void ClearCollisionResults() { m_vecCollisionResults.clear(); }

			void SetEnableTriangleDrawCallback(bool isEnableTriangleDrawCallback) { m_isEnableTriangleDrawCallback = isEnableTriangleDrawCallback; }

		public:
			const RigidBodyProperty& GetRigidBodyProperty() { return m_rigidBodyProperty; }

			void SetWorldMatrix(const Math::Matrix& mat);
			Math::Matrix GetWorldMatrix();

			Math::Quaternion GetOrientation();
			Math::Vector3 GetCenterOfMassPosition();

			const String::StringID& GetName() { return m_rigidBodyProperty.strName; }
			void SetName(const String::StringID& strName) { m_rigidBodyProperty.strName = strName; }

			void SetLinearVelocity(const Math::Vector3& f3Velocity);

			void SetDamping(float fLinearDamping, float fAngularDamping);
			void SetDeactivationTime(float fTime);
			void SetSleepingThresholds(float fLinear, float fAngular);
			void SetAngularFactor(float fFactor);

			void SetActiveState(EmActiveState::Type emActiveState);

			const Collision::AABB& GetAABB() { return m_boundingBox; }
			const Collision::Sphere& GetBoundingSphere() { return m_boundingSphere; }
			const Collision::OBB& GetOBB() { return m_boundingOrientedBox; }

			btRigidBody* GetInterface() { return m_pRigidBody; }

		private:
			bool init(const RigidBodyProperty& physicsProperty);

		private:
			btDiscreteDynamicsWorld* m_pDynamicsWorld;
			btRigidBody* m_pRigidBody;
			btCollisionShape* m_pCollisionShape;
			std::variant<btTriangleMesh*, btTriangleIndexVertexArray*> m_varTriangleMesh;
			btMotionState* m_pMotionState;

			RigidBodyProperty m_rigidBodyProperty;

			Math::Matrix m_matWorld;

			Collision::AABB m_boundingBox;
			Collision::Sphere m_boundingSphere;
			Collision::OBB m_boundingOrientedBox;

			std::vector<CollisionResult> m_vecCollisionResults;
			bool m_isEnableTriangleDrawCallback;
		};
	}
}