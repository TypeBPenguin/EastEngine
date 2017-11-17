#pragma once

#include "PhysicsDefine.h"

class btPairCachingGhostObject;

namespace EastEngine
{
	namespace Physics
	{
		struct GhostProperty
		{
			int nCollisionFlag = EmCollision::eCharacterObject;

			Math::Vector3 f3OriginPos;
			Math::Quaternion originQuat;

			Shape shapeInfo;

			EmPhysicsShape::Type GetShapeType() const { return shapeInfo.emPhysicsShapeType; }
		};

		class GhostObject
		{
		private:
			GhostObject();

		public:
			~GhostObject();

			static GhostObject* Create(const GhostProperty& ghostProperty);

		public:
			Math::Matrix GetWorldMatrix();

		public:
			btPairCachingGhostObject* GetInterface() { return m_pGhostObject; }
			btCollisionShape* GetCollisionShape() { return m_pCollisionShape; }

		private:
			bool init(const GhostProperty& ghostProperty);

		private:
			btDiscreteDynamicsWorld* m_pDynamicsWorld;
			btPairCachingGhostObject* m_pGhostObject;
			btCollisionShape* m_pCollisionShape;
			btTriangleMesh* m_pTriangleMesh;
		};
	}
}