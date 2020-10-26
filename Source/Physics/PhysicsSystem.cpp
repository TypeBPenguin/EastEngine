#include "stdafx.h"
#include "PhysicsSystem.h"

#include "PhysicsUtil.h"

namespace est
{
	namespace physics
	{
		physx::PxPhysics* s_pPhysics{ nullptr };
		physx::PxCooking* s_pCooking{ nullptr };
		physx::PxScene* s_pScene{ nullptr };

		physx::PxPhysics* GetPhysics()
		{
			return s_pPhysics;
		}

		physx::PxCooking* GetCooking()
		{
			return s_pCooking;
		}

		physx::PxScene* GetScene()
		{
			return s_pScene;
		}

		class System::Impl
		{
		public:
			Impl();
			~Impl();

		public:
			bool Initialize(const Initializer& initializer);

		public:
			void Update(float elapsedtime);

		private:
			physx::PxDefaultAllocator m_allocator;
			physx::PxDefaultErrorCallback m_errorCallback;

			physx::PxFoundation* m_pFoundation{ nullptr };
			physx::PxPhysics* m_pPhysics{ nullptr };

			physx::PxCooking* m_pCooking{ nullptr };

			physx::PxDefaultCpuDispatcher* m_pDispatcher{ nullptr };
			physx::PxScene* m_pScene{ nullptr };

			physx::PxPvd* m_pPvd{ nullptr };
		};

		System::Impl::Impl()
		{
		}

		System::Impl::~Impl()
		{
			s_pPhysics = nullptr;
			s_pCooking = nullptr;
			s_pScene = nullptr;

			m_pScene->release();
			m_pDispatcher->release();
			m_pPhysics->release();
			m_pCooking->release();
			physx::PxPvdTransport* pTransport = m_pPvd->getTransport();
			m_pPvd->release();
			if (pTransport != nullptr)
			{
				pTransport->release();
			}

			m_pFoundation->release();
		}
		
		bool System::Impl::Initialize(const Initializer& initializer)
		{
			const char PVD_HOST[] = { "127.0.0.1" };

			m_pFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, m_allocator, m_errorCallback);

			m_pPvd = physx::PxCreatePvd(*m_pFoundation);
			physx::PxPvdTransport* transport = physx::PxDefaultPvdSocketTransportCreate(PVD_HOST, 5425, 10);
			m_pPvd->connect(*transport, physx::PxPvdInstrumentationFlag::eALL);

			m_pPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_pFoundation, physx::PxTolerancesScale(), true, m_pPvd);

			const uint32_t numCores = std::thread::hardware_concurrency();
			m_pDispatcher = physx::PxDefaultCpuDispatcherCreate(numCores == 0 ? 0 : numCores - 1);

			physx::PxSceneDesc sceneDesc(m_pPhysics->getTolerancesScale());
			sceneDesc.gravity = Convert<const physx::PxVec3>(initializer.Gravity);
			sceneDesc.cpuDispatcher = m_pDispatcher;
			sceneDesc.filterShader = physx::PxDefaultSimulationFilterShader;
			m_pScene = m_pPhysics->createScene(sceneDesc);

			m_pCooking = PxCreateCooking(PX_PHYSICS_VERSION, *m_pFoundation, physx::PxCookingParams(physx::PxTolerancesScale()));

			s_pPhysics = m_pPhysics;
			s_pCooking = m_pCooking;
			s_pScene = m_pScene;

			return true;
		}

		void System::Impl::Update(float elapsedtime)
		{
			TRACER_EVENT(__FUNCTIONW__);
			m_pScene->simulate(elapsedtime);
			m_pScene->fetchResults(true);
		}

		System::System()
			: m_pImpl{ std::make_unique<Impl>() }
		{
		}

		System::~System()
		{
		}

		bool System::Initialize(const Initializer& initializer)
		{
			return m_pImpl->Initialize(initializer);
		}

		void System::Update(float elapsedtime)
		{
			m_pImpl->Update(elapsedtime);
		}
	}
}