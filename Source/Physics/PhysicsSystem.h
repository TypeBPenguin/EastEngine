#pragma once

#include "CommonLib/Singleton.h"

#include "RigidBody.h"
#include "Constraint.h"

class btBroadphaseInterface;
class btDiscreteDynamicsWorld;

namespace eastengine
{
	namespace Physics
	{
		class RigidBody;
		class ConstraintInterface;

		class System : public Singleton<System>
		{
			friend Singleton<System>;
		private:
			System();
			virtual ~System();

		public:
			void Update(float fElapsedtime);

		public:
			void AddRigidBody(RigidBody* pRigidBody);
			void AddRigidBody(RigidBody* pRigidBody, short group, short mask);
			void AddConstraint(ConstraintInterface* pConstraint, bool isEanbleCollisionBetweenLinkedBodies = true);
			btDiscreteDynamicsWorld* GetDynamicsWorld();
			btBroadphaseInterface* GetBoradphaseInterface();

		private:
			class Impl;
			std::unique_ptr<Impl> m_pImpl;
		};
	}
}