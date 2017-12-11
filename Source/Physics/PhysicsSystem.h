#pragma once

#include "CommonLib/Singleton.h"

#include "RigidBody.h"
#include "Constraint.h"

class btDefaultCollisionConfiguration;
class btCollisionDispatcher;
class btBroadphaseInterface;
class btSequentialImpulseConstraintSolver;
class btDynamicsWorld;
class btDiscreteDynamicsWorld;
class btCollisionObject;
class btManifoldPoint;
struct btCollisionObjectWrapper;

namespace EastEngine
{
	namespace Physics
	{
		class RigidBody;
		class ConstraintInterface;

		class PhysicsSystem : public Singleton<PhysicsSystem>
		{
			friend Singleton<PhysicsSystem>;
		private:
			PhysicsSystem();
			virtual ~PhysicsSystem();

		public:
			bool Init();
			void Release();

			void Update(float fElapsedtime);

		public:
			void AddRigidBody(RigidBody* pRigidBody);
			void AddRigidBody(RigidBody* pRigidBody, short group, short mask);
			void AddConstraint(ConstraintInterface* pConstraint, bool isEanbleCollisionBetweenLinkedBodies = true);
			btDiscreteDynamicsWorld* GetDynamicsWorld() { return m_pDynamicsWorld; }
			btBroadphaseInterface* GetBoradphaseInterface() { return m_pOverlappingPairCache; }

		private:
			void ProcessAddWaitObject();

		private:
			static void tickCallback(btDynamicsWorld* pDynamicsWorld, float fTimeStep);

			// 충돌하는 순간
			static bool contactAddedCallback(btManifoldPoint& cp, const btCollisionObjectWrapper* pCollisionObj0Wrap, int nPartId0, int nIndex0, const btCollisionObjectWrapper* pCollisionObj1Wrap, int nPartId1, int nIndex1);

			// 충돌 처리할때마다
			static bool contactProcessedCallback(btManifoldPoint& cp, void* body0, void* body1);

			// 아직 모름..
			static bool contactDestroyedCallback(void* userPersistentData);

		private:
			btDefaultCollisionConfiguration* m_pCollisionConfig;
			btCollisionDispatcher* m_pDispatcher;
			btBroadphaseInterface* m_pOverlappingPairCache;
			btSequentialImpulseConstraintSolver* m_pSolver;

			btDiscreteDynamicsWorld* m_pDynamicsWorld;

			bool m_isInit;

			struct AddWaitRigidBody
			{
				struct Default
				{
					RigidBody* pRigidBody = nullptr;
				};

				struct Group
				{
					RigidBody* pRigidBody = nullptr;
					short group = 0;
					short mask = 0;
				};

				std::variant<Default, Group> rigidBody;
			};
			Concurrency::concurrent_queue<AddWaitRigidBody> m_conQueueAddWaitRigidBody;

			struct AddWaitConstraintInterface
			{
				ConstraintInterface* pConstraint = nullptr;
				bool isEanbleCollisionBetweenLinkedBodies = true;
			};
			Concurrency::concurrent_queue<AddWaitConstraintInterface> m_conQueueAddWaitConstraintInterface;
		};
	}
}