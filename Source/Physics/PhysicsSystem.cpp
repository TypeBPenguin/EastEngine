#include "stdafx.h"
#include "PhysicsSystem.h"

#include "MathConvertor.h"
#include "Constraint.h"

namespace EastEngine
{
	namespace Physics
	{
		PhysicsSystem::PhysicsSystem()
			: m_pCollisionConfig(nullptr)
			, m_pDispatcher(nullptr)
			, m_pOverlappingPairCache(nullptr)
			, m_pSolver(nullptr)
			, m_pDynamicsWorld(nullptr)
			, m_isInit(false)
		{
		}

		PhysicsSystem::~PhysicsSystem()
		{
			Release();
		}

		bool PhysicsSystem::Init()
		{
			if (m_isInit == true)
				return true;

			m_isInit = true;

			m_pCollisionConfig = new btDefaultCollisionConfiguration;
			m_pDispatcher = new	btCollisionDispatcher(m_pCollisionConfig);
			m_pOverlappingPairCache = new btDbvtBroadphase;
			m_pSolver = new btSequentialImpulseConstraintSolver;
			
			m_pDynamicsWorld = new btDiscreteDynamicsWorld(m_pDispatcher, m_pOverlappingPairCache, m_pSolver, m_pCollisionConfig);
			m_pDynamicsWorld->getDispatchInfo().m_enableSPU = true;
			m_pDynamicsWorld->setGravity(btVector3(0, Gravity, 0));
			m_pDynamicsWorld->setInternalTickCallback(PhysicsSystem::tickCallback, this);

			// 이런 녀석들도 있다. 필요할때 쓰자
			//gContactAddedCallback = PhysicsSystem::contactAddedCallback;
			//gContactProcessedCallback = PhysicsSystem::contactProcessedCallback;
			//gContactDestroyedCallback = PhysicsSystem::contactDestroyedCallback;

			return true;
		}

		void PhysicsSystem::Release()
		{
			if (m_isInit == false)
				return;

			for (int i = m_pDynamicsWorld->getNumConstraints() - 1; i >= 0; i--)
			{
				m_pDynamicsWorld->removeConstraint(m_pDynamicsWorld->getConstraint(i));
			}

			for (int i = m_pDynamicsWorld->getNumCollisionObjects() - 1; i >= 0; --i)
			{
				btCollisionObject* pObj = m_pDynamicsWorld->getCollisionObjectArray()[i];
				btRigidBody* pBody = btRigidBody::upcast(pObj);
				if (pBody != nullptr && pBody->getMotionState() != nullptr)
				{
					delete pBody->getMotionState();
				}

				m_pDynamicsWorld->removeCollisionObject(pObj);
				SafeDelete(pObj);
			}

			SafeDelete(m_pDynamicsWorld);
			SafeDelete(m_pOverlappingPairCache);
			SafeDelete(m_pSolver);
			SafeDelete(m_pDispatcher);
			SafeDelete(m_pCollisionConfig);

			m_isInit = false;
		}

		void PhysicsSystem::Update(float fElapsedtime)
		{
			ProcessAddWaitObject();

			btCollisionObjectArray& objectArray = m_pDynamicsWorld->getCollisionObjectArray();
			for (int i = 0; i < objectArray.size(); ++i)
			{
				btCollisionObject* pObject = objectArray[i];
				RigidBody* pRigidBody = reinterpret_cast<RigidBody*>(pObject->getUserPointer());
				if (pRigidBody == nullptr)
					continue;

				pRigidBody->ClearCollisionResults();
			}
			
			m_pDynamicsWorld->stepSimulation(fElapsedtime);
		}

		void PhysicsSystem::AddRigidBody(RigidBody* pRigidBody)
		{
			AddWaitRigidBody job;
			AddWaitRigidBody::Default& defaultJob = job.rigidBody.emplace<AddWaitRigidBody::Default>();
			defaultJob.pRigidBody = pRigidBody;

			m_conQueueAddWaitRigidBody.push(job);
		}

		void PhysicsSystem::AddRigidBody(RigidBody* pRigidBody, short group, short mask)
		{
			AddWaitRigidBody job;
			AddWaitRigidBody::Group& groupJob = job.rigidBody.emplace<AddWaitRigidBody::Group>();
			groupJob.pRigidBody = pRigidBody;
			groupJob.group = group;
			groupJob.mask = mask;

			m_conQueueAddWaitRigidBody.push(job);
		}

		void PhysicsSystem::AddConstraint(ConstraintInterface* pConstraint, bool isEanbleCollisionBetweenLinkedBodies)
		{
			m_conQueueAddWaitConstraintInterface.push({ pConstraint, isEanbleCollisionBetweenLinkedBodies });
		}
		
		void PhysicsSystem::ProcessAddWaitObject()
		{
			while (m_conQueueAddWaitRigidBody.empty() == false)
			{
				AddWaitRigidBody job;
				if (m_conQueueAddWaitRigidBody.try_pop(job) == true)
				{
					AddWaitRigidBody::Default* p0 = std::get_if<AddWaitRigidBody::Default>(&job.rigidBody);
					if (p0 != nullptr)
					{
						m_pDynamicsWorld->addRigidBody(p0->pRigidBody->GetInterface());
					}

					AddWaitRigidBody::Group* p1 = std::get_if<AddWaitRigidBody::Group>(&job.rigidBody);
					if (p1 != nullptr)
					{
						m_pDynamicsWorld->addRigidBody(p1->pRigidBody->GetInterface(), p1->group, p1->mask);
					}
				}
			}

			while (m_conQueueAddWaitConstraintInterface.empty() == false)
			{
				AddWaitConstraintInterface job;
				if (m_conQueueAddWaitConstraintInterface.try_pop(job) == true)
				{
					m_pDynamicsWorld->addConstraint(job.pConstraint->GetInterface(), job.isEanbleCollisionBetweenLinkedBodies);
				}
			}
		}

		void PhysicsSystem::tickCallback(btDynamicsWorld* pDynamicsWorld, float fTimeStep)
		{
			PhysicsSystem* pPhysicsSystem = reinterpret_cast<PhysicsSystem*>(pDynamicsWorld->getWorldUserInfo());
			if (pPhysicsSystem == nullptr)
				return;

			btDispatcher* pDispatcher = pDynamicsWorld->getDispatcher();
			if (pDispatcher == nullptr)
				return;

			int nManifolds = pDispatcher->getNumManifolds();
			for (int i = 0; i < nManifolds; ++i)
			{
				btPersistentManifold* pContactManifold = pDispatcher->getManifoldByIndexInternal(i);
				if (pContactManifold == nullptr)
					continue;

				const btCollisionObject* pObjA = pContactManifold->getBody0();
				const btCollisionObject* pObjB = pContactManifold->getBody1();
				if (pObjA == nullptr || pObjB == nullptr)
					continue;

				RigidBody* pRigidBodyA = reinterpret_cast<RigidBody*>(pObjA->getUserPointer());
				RigidBody* pRigidBodyB = reinterpret_cast<RigidBody*>(pObjB->getUserPointer());
				if (pRigidBodyA == nullptr || pRigidBodyB == nullptr)
					continue;

				int nContacts = pContactManifold->getNumContacts();
				for (int j = 0; j < nContacts; ++j)
				{
					btManifoldPoint& point = pContactManifold->getContactPoint(j);
					Math::Vector3 f3PointA(Math::Convert(point.getPositionWorldOnA()));
					Math::Vector3 f3PointB(Math::Convert(point.getPositionWorldOnB()));

					pRigidBodyA->AddCollisionResult(pRigidBodyB, f3PointB, f3PointA);
					pRigidBodyB->AddCollisionResult(pRigidBodyA, f3PointA, f3PointB);
				}
			}
		}

		bool PhysicsSystem::contactAddedCallback(btManifoldPoint& cp, const btCollisionObjectWrapper* pCollisionObj0Wrap, int nPartId0, int nIndex0, const btCollisionObjectWrapper* pCollisionObj1Wrap, int nPartId1, int nIndex1)
		{
			return true;
		}

		bool PhysicsSystem::contactProcessedCallback(btManifoldPoint& cp, void* body0, void* body1)
		{
			return true;
		}

		bool PhysicsSystem::contactDestroyedCallback(void* userPersistentData)
		{
			return true;
		}
	}
}