#pragma once

#include "PhysicsDefine.h"

class btPairCachingGhostObject;

namespace eastengine
{
	namespace Physics
	{
		struct GhostProperty
		{
			int nCollisionFlag = EmCollision::eCharacterObject;

			math::Vector3 f3OriginPos;
			math::Quaternion originQuat;

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
			math::Matrix GetWorldMatrix() const;

		public:
			btPairCachingGhostObject* GetInterface();
			btCollisionShape* GetCollisionShape();

		private:
			bool Initialize(const GhostProperty& ghostProperty);

		private:
			class Impl;
			std::unique_ptr<Impl> m_pImpl;
		};
	}
}