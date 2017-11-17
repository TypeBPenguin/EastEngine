#include "stdafx.h"
#include "GhostObject.h"

#include "PhysicsSystem.h"
#include "MathConvertor.h"

#include "BulletCollision/CollisionDispatch/btGhostObject.h"

namespace EastEngine
{
	namespace Physics
	{
		GhostObject::GhostObject()
			: m_pDynamicsWorld(nullptr)
			, m_pGhostObject(nullptr)
			, m_pCollisionShape(nullptr)
			, m_pTriangleMesh(nullptr)
		{
		}

		GhostObject::~GhostObject()
		{
			if (m_pDynamicsWorld != nullptr)
			{
				m_pDynamicsWorld->removeCollisionObject(m_pGhostObject);
				m_pDynamicsWorld = nullptr;
			}

			SafeDelete(m_pGhostObject);
			SafeDelete(m_pCollisionShape);
			SafeDelete(m_pTriangleMesh);
		}

		GhostObject* GhostObject::Create(const GhostProperty& ghostProperty)
		{
			GhostObject* pGhostObject = new GhostObject;
			if (pGhostObject->init(ghostProperty) == false)
			{
				SafeDelete(pGhostObject);
				return nullptr;
			}

			return pGhostObject;
		}

		Math::Matrix GhostObject::GetWorldMatrix()
		{
			return Math::Convert(m_pGhostObject->getWorldTransform());
		}

		bool GhostObject::init(const GhostProperty& ghostProperty)
		{
			m_pCollisionShape = CreateShape(ghostProperty.shapeInfo);
			if (m_pCollisionShape == nullptr)
				return false;

			if (ghostProperty.GetShapeType() == EmPhysicsShape::eTriangleMesh)
			{
				btBvhTriangleMeshShape* pTriangleMeshShape = static_cast<btBvhTriangleMeshShape*>(m_pCollisionShape);
				m_pTriangleMesh = static_cast<btTriangleMesh*>(pTriangleMeshShape->getMeshInterface());

				if (m_pTriangleMesh == nullptr)
					return false;
			}

			m_pDynamicsWorld = PhysicsSystem::GetInstance()->GetDynamicsWorld();

			btTransform transform;
			transform.setIdentity();
			transform.setOrigin(Math::ConvertToBt(ghostProperty.f3OriginPos));
			transform.setRotation(Math::ConvertToBt(ghostProperty.originQuat));

			m_pGhostObject = new btPairCachingGhostObject;
			m_pGhostObject->setWorldTransform(transform);
			m_pGhostObject->setCollisionShape(m_pCollisionShape);
			m_pGhostObject->setCollisionFlags(ghostProperty.nCollisionFlag);

			m_pDynamicsWorld->addCollisionObject(m_pGhostObject, btBroadphaseProxy::CharacterFilter, btBroadphaseProxy::StaticFilter | btBroadphaseProxy::DefaultFilter);
			
			return true;
		}
	}
}