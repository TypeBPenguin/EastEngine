#include "stdafx.h"
#include "GhostObject.h"

#include "PhysicsSystem.h"
#include "MathConvertor.h"

#include "BulletCollision/CollisionDispatch/btGhostObject.h"

namespace eastengine
{
	namespace physics
	{
		class GhostObject::Impl
		{
		public:
			Impl();
			~Impl();

		public:
			bool Initialize(const GhostProperty& ghostProperty);

		public:
			math::Matrix GetWorldMatrix() const;

		public:
			btPairCachingGhostObject* GetInterface();
			btCollisionShape* GetCollisionShape();

		private:
			btDiscreteDynamicsWorld* m_pDynamicsWorld{ nullptr };
			std::unique_ptr<btPairCachingGhostObject> m_pGhostObject;
			std::unique_ptr<btCollisionShape> m_pCollisionShape;
			btTriangleMesh* m_pTriangleMesh{ nullptr };
		};

		GhostObject::Impl::Impl()
		{
		}

		GhostObject::Impl::~Impl()
		{
			if (m_pDynamicsWorld != nullptr)
			{
				m_pDynamicsWorld->removeCollisionObject(m_pGhostObject.get());
				m_pDynamicsWorld = nullptr;
			}

			m_pGhostObject.reset();
			m_pCollisionShape.reset();

			SafeDelete(m_pTriangleMesh);
		}

		bool GhostObject::Impl::Initialize(const GhostProperty& ghostProperty)
		{
			m_pCollisionShape = CreateShape(ghostProperty.shapeInfo);
			if (m_pCollisionShape == nullptr)
				return false;

			if (ghostProperty.GetShapeType() == ShapeType::eTriangleMesh)
			{
				btBvhTriangleMeshShape* pTriangleMeshShape = static_cast<btBvhTriangleMeshShape*>(m_pCollisionShape.get());
				m_pTriangleMesh = static_cast<btTriangleMesh*>(pTriangleMeshShape->getMeshInterface());

				if (m_pTriangleMesh == nullptr)
					return false;
			}

			m_pDynamicsWorld = System::GetInstance()->GetDynamicsWorld();

			btTransform transform;
			transform.setIdentity();
			transform.setOrigin(math::ConvertToBt(ghostProperty.f3OriginPos));
			transform.setRotation(math::ConvertToBt(ghostProperty.originQuat));

			m_pGhostObject = std::make_unique<btPairCachingGhostObject>();
			m_pGhostObject->setWorldTransform(transform);
			m_pGhostObject->setCollisionShape(m_pCollisionShape.get());
			m_pGhostObject->setCollisionFlags(ghostProperty.nCollisionFlag);

			m_pDynamicsWorld->addCollisionObject(m_pGhostObject.get(), btBroadphaseProxy::CharacterFilter, btBroadphaseProxy::StaticFilter | btBroadphaseProxy::DefaultFilter);

			return true;
		}

		math::Matrix GhostObject::Impl::GetWorldMatrix() const
		{
			return math::Convert(m_pGhostObject->getWorldTransform());
		}

		btPairCachingGhostObject* GhostObject::Impl::GetInterface()
		{
			return m_pGhostObject.get();
		}

		btCollisionShape* GhostObject::Impl::GetCollisionShape()
		{
			return m_pCollisionShape.get();
		}

		GhostObject::GhostObject()
			: m_pImpl{ std::make_unique<Impl>() }
		{
		}

		GhostObject::~GhostObject()
		{
		}

		GhostObject* GhostObject::Create(const GhostProperty& ghostProperty)
		{
			GhostObject* pGhostObject = new GhostObject;
			if (pGhostObject->Initialize(ghostProperty) == false)
			{
				SafeDelete(pGhostObject);
				return nullptr;
			}

			return pGhostObject;
		}

		math::Matrix GhostObject::GetWorldMatrix() const
		{
			return m_pImpl->GetWorldMatrix();
		}

		btPairCachingGhostObject* GhostObject::GetInterface()
		{
			return m_pImpl->GetInterface();
		}

		btCollisionShape* GhostObject::GetCollisionShape()
		{
			return m_pImpl->GetCollisionShape();
		}

		bool GhostObject::Initialize(const GhostProperty& ghostProperty)
		{
			return m_pImpl->Initialize(ghostProperty);
		}
	}
}