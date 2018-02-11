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
			Math::Matrix GetWorldMatrix() const;

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