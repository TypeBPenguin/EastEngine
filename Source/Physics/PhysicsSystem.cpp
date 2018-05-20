#include "stdafx.h"
#include "PhysicsSystem.h"

#include "MathConvertor.h"
#include "Constraint.h"

namespace eastengine
{
	namespace Physics
	{
		class System::Impl
		{
		public:
			Impl();
			~Impl();

		public:
			void Update(float fElapsedtime);

		public:
			void AddRigidBody(RigidBody* pRigidBody);
			void AddRigidBody(RigidBody* pRigidBody, short group, short mask);
			void AddConstraint(ConstraintInterface* pConstraint, bool isEanbleCollisionBetweenLinkedBodies = true);
			btDiscreteDynamicsWorld* GetDynamicsWorld() { return m_pDynamicsWorld.get(); }
			btBroadphaseInterface* GetBoradphaseInterface() { return m_pOverlappingPairCache.get(); }

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
			std::unique_ptr<btDefaultCollisionConfiguration> m_pCollisionConfig;
			std::unique_ptr<btCollisionDispatcher> m_pDispatcher;
			std::unique_ptr<btBroadphaseInterface> m_pOverlappingPairCache;
			std::unique_ptr<btSequentialImpulseConstraintSolver> m_pSolver;

			std::unique_ptr<btDiscreteDynamicsWorld> m_pDynamicsWorld;

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

		System::Impl::Impl()
		{
			m_pCollisionConfig = std::make_unique<btDefaultCollisionConfiguration>();
			m_pDispatcher = std::make_unique<btCollisionDispatcher>(m_pCollisionConfig.get());
			m_pOverlappingPairCache = std::make_unique<btDbvtBroadphase>();
			m_pSolver = std::make_unique<btSequentialImpulseConstraintSolver>();

			m_pDynamicsWorld = std::make_unique<btDiscreteDynamicsWorld>(m_pDispatcher.get(), m_pOverlappingPairCache.get(), m_pSolver.get(), m_pCollisionConfig.get());
			m_pDynamicsWorld->getDispatchInfo().m_enableSPU = true;
			m_pDynamicsWorld->setGravity(btVector3(0, Gravity, 0));
			m_pDynamicsWorld->setInternalTickCallback(System::Impl::tickCallback, this);

			// 이런 녀석들도 있다. 필요할때 쓰자
			//gContactAddedCallback = System::contactAddedCallback;
			//gContactProcessedCallback = System::contactProcessedCallback;
			//gContactDestroyedCallback = System::contactDestroyedCallback;
		}

		System::Impl::~Impl()
		{
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

			m_pDynamicsWorld.reset();
			m_pOverlappingPairCache.reset();
			m_pSolver.reset();
			m_pDispatcher.reset();
			m_pCollisionConfig.reset();
		}
		
		void System::Impl::Update(float fElapsedtime)
		{
			TRACER_EVENT("System::Update");
			ProcessAddWaitObject();

			TRACER_BEGINEVENT("ClearCollisionResults");
			btCollisionObjectArray& objectArray = m_pDynamicsWorld->getCollisionObjectArray();
			int nSize = objectArray.size();
			for (int i = 0; i < nSize; ++i)
			{
				btCollisionObject* pObject = objectArray[i];
				RigidBody* pRigidBody = reinterpret_cast<RigidBody*>(pObject->getUserPointer());
				if (pRigidBody == nullptr)
					continue;

				pRigidBody->ClearCollisionResults();
			}
			TRACER_ENDEVENT();

			TRACER_BEGINEVENT("StepSimulation");
			m_pDynamicsWorld->stepSimulation(fElapsedtime);
			TRACER_ENDEVENT();
		}

		void System::Impl::AddRigidBody(RigidBody* pRigidBody)
		{
			AddWaitRigidBody job;
			AddWaitRigidBody::Default& defaultJob = job.rigidBody.emplace<AddWaitRigidBody::Default>();
			defaultJob.pRigidBody = pRigidBody;

			m_conQueueAddWaitRigidBody.push(job);
		}

		void System::Impl::AddRigidBody(RigidBody* pRigidBody, short group, short mask)
		{
			AddWaitRigidBody job;
			AddWaitRigidBody::Group& groupJob = job.rigidBody.emplace<AddWaitRigidBody::Group>();
			groupJob.pRigidBody = pRigidBody;
			groupJob.group = group;
			groupJob.mask = mask;

			m_conQueueAddWaitRigidBody.push(job);
		}

		void System::Impl::AddConstraint(ConstraintInterface* pConstraint, bool isEanbleCollisionBetweenLinkedBodies)
		{
			m_conQueueAddWaitConstraintInterface.push({ pConstraint, isEanbleCollisionBetweenLinkedBodies });
		}

		void System::Impl::ProcessAddWaitObject()
		{
			TRACER_EVENT("System::ProcessAddWaitObject");
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

		void System::Impl::tickCallback(btDynamicsWorld* pDynamicsWorld, float fTimeStep)
		{
			System* pSystem = reinterpret_cast<System*>(pDynamicsWorld->getWorldUserInfo());
			if (pSystem == nullptr)
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
					math::Vector3 f3PointA(math::Convert(point.getPositionWorldOnA()));
					math::Vector3 f3PointB(math::Convert(point.getPositionWorldOnB()));

					pRigidBodyA->AddCollisionResult(pRigidBodyB, f3PointB, f3PointA);
					pRigidBodyB->AddCollisionResult(pRigidBodyA, f3PointA, f3PointB);
				}
			}
		}

		bool System::Impl::contactAddedCallback(btManifoldPoint& cp, const btCollisionObjectWrapper* pCollisionObj0Wrap, int nPartId0, int nIndex0, const btCollisionObjectWrapper* pCollisionObj1Wrap, int nPartId1, int nIndex1)
		{
			return true;
		}

		bool System::Impl::contactProcessedCallback(btManifoldPoint& cp, void* body0, void* body1)
		{
			return true;
		}

		bool System::Impl::contactDestroyedCallback(void* userPersistentData)
		{
			return true;
		}

		System::System()
			: m_pImpl{ std::make_unique<Impl>() }
		{
		}

		System::~System()
		{
		}

		void System::Update(float fElapsedtime)
		{
			m_pImpl->Update(fElapsedtime);
		}

		void System::AddRigidBody(RigidBody* pRigidBody)
		{
			m_pImpl->AddRigidBody(pRigidBody);
		}

		void System::AddRigidBody(RigidBody* pRigidBody, short group, short mask)
		{
			m_pImpl->AddRigidBody(pRigidBody, group, mask);
		}

		void System::AddConstraint(ConstraintInterface* pConstraint, bool isEanbleCollisionBetweenLinkedBodies)
		{
			m_pImpl->AddConstraint(pConstraint, isEanbleCollisionBetweenLinkedBodies);
		}

		btDiscreteDynamicsWorld* System::GetDynamicsWorld()
		{
			return m_pImpl->GetDynamicsWorld();
		}

		btBroadphaseInterface* System::GetBoradphaseInterface()
		{
			return m_pImpl->GetBoradphaseInterface();
		}
	}
}