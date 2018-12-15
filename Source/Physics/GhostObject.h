#pragma once

#include "PhysicsDefine.h"

class btPairCachingGhostObject;

namespace eastengine
{
	namespace physics
	{
		struct GhostProperty
		{
			int nCollisionFlag = CollisionFlag::eCharacterObject;

			math::float3 f3OriginPos;
			math::Quaternion originQuat;

			Shape shapeInfo;

			ShapeType GetShapeType() const { return shapeInfo.emShapeType; }
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