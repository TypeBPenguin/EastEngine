#include "stdafx.h"
#include "RigidBody.h"

#include "PhysicsSystem.h"
#include "DebugHelper.h"
#include "MathConvertor.h"

namespace EastEngine
{
	namespace Physics
	{
		RigidBody::RigidBody()
			: m_pDynamicsWorld(nullptr)
			, m_pRigidBody(nullptr)
			, m_pCollisionShape(nullptr)
			//, m_pTriangleMesh(nullptr)
			, m_pMotionState(nullptr)
			, m_isEnableTriangleDrawCallback(false)
		{
		}

		RigidBody::~RigidBody()
		{
			if (m_pDynamicsWorld != nullptr)
			{
				m_pDynamicsWorld->removeRigidBody(m_pRigidBody);
				m_pDynamicsWorld = nullptr;
			}

			//SafeDelete(m_pTriangleMesh);

			btTriangleMesh** pp1 = std::get_if<btTriangleMesh*>(&m_varTriangleMesh);
			if (pp1 != nullptr)
			{
				SafeDelete(*pp1);
			}

			btTriangleIndexVertexArray** pp2 = std::get_if<btTriangleIndexVertexArray*>(&m_varTriangleMesh);
			if (pp2 != nullptr)
			{
				SafeDelete(*pp2);
			}

			SafeDelete(m_pCollisionShape);
			SafeDelete(m_pMotionState);
			SafeDelete(m_pRigidBody);
		}

		RigidBody* RigidBody::Create(const RigidBodyProperty& rigidBodyProperty)
		{
			if (rigidBodyProperty.GetShapeType() == EmPhysicsShape::eEmpty)
				return nullptr;

			RigidBody* pRigidBody = new RigidBody;
			if (pRigidBody->init(rigidBodyProperty) == false)
			{
				SafeDelete(pRigidBody);

				return nullptr;
			}
			
			return pRigidBody;
		}

		void RigidBody::Update(float fElapsedTime)
		{
			m_matWorld = Math::Convert(m_pRigidBody->getWorldTransform());

			UpdateBoundingBox(m_matWorld);

			if (m_rigidBodyProperty.funcCollisionCallback != nullptr && m_vecCollisionResults.empty() == false)
			{
				m_rigidBodyProperty.funcCollisionCallback(m_vecCollisionResults);
			}

			if (m_rigidBodyProperty.funcTriangleDrawCallback != nullptr && m_isEnableTriangleDrawCallback == true)
			{
				if (m_rigidBodyProperty.shapeInfo.emPhysicsShapeType == EmPhysicsShape::Type::eTriangleMesh ||
					m_rigidBodyProperty.shapeInfo.emPhysicsShapeType == EmPhysicsShape::Type::eTerrain)
				{
					btConcaveShape* pShape = static_cast<btConcaveShape*>(m_pCollisionShape);

					btVector3 aabbMin(btScalar(-1e30), btScalar(-1e30), btScalar(-1e30));
					btVector3 aabbMax(btScalar(1e30), btScalar(1e30), btScalar(1e30));

					std::vector<Math::Vector3> triangles;
					if (m_rigidBodyProperty.shapeInfo.emPhysicsShapeType == EmPhysicsShape::Type::eTriangleMesh)
					{
					}
					else if (m_rigidBodyProperty.shapeInfo.emPhysicsShapeType == EmPhysicsShape::Type::eTerrain)
					{
						const Shape::Terrain* pShapeInfo = std::get_if<Shape::Terrain>(&m_rigidBodyProperty.shapeInfo.element);
						if (pShapeInfo != nullptr)
						{
							triangles.reserve(pShapeInfo->nHeightArarySize * 6);
						}
					}

					DebugTriangleDrawCallback callback(&triangles);
					pShape->processAllTriangles(&callback, aabbMin, aabbMax);

					m_rigidBodyProperty.funcTriangleDrawCallback(triangles.data(), triangles.size());
				}
			}
		}

		void RigidBody::UpdateBoundingBox(const Math::Matrix& matWorld)
		{
			btVector3 vMin, vMax;
			m_pRigidBody->getAabb(vMin, vMax);

			Math::Vector3 f3Min = Math::Convert(vMin);
			Math::Vector3 f3Max = Math::Convert(vMax);

			Collision::AABB::CreateFromPoints(m_boundingBox, f3Min, f3Max);
			m_boundingBox.Extents = Math::Vector3::Max(m_boundingBox.Extents, Math::Vector3(0.01f));
			Collision::Sphere::CreateFromAABB(m_boundingSphere, m_boundingBox);
			Collision::OBB::CreateFromAABB(m_boundingOrientedBox, m_boundingBox);
		}

		bool RigidBody::IsCollision(const RigidBody* pRigidBody)
		{
			if (m_vecCollisionResults.empty() == false)
			{
				for (auto& result : m_vecCollisionResults)
				{
					if (result.pOpponentObject == pRigidBody)
						return true;
				}
			}

			struct CustomResultCallBack : public btCollisionWorld::ContactResultCallback
			{
				bool isCollision = false;

				virtual btScalar addSingleResult(btManifoldPoint& cp, const btCollisionObjectWrapper* pCollisionObj0, int nPartID0, int nIndex0, const btCollisionObjectWrapper* pCollisionObj1, int nPartID1, int nIndex1) override
				{
					isCollision = true;

					return 0.f;
				}
			};

			CustomResultCallBack result;

			m_pDynamicsWorld->contactPairTest(m_pRigidBody, pRigidBody->m_pRigidBody, result);

			return result.isCollision;
		}
		
		void RigidBody::SetWorldMatrix(const Math::Matrix& mat)
		{
			Math::Vector3 f3Pos, vScale;
			Math::Quaternion quat;

			mat.Decompose(vScale, quat, f3Pos);

			m_pMotionState->setWorldTransform(Math::ConvertToBt(f3Pos, quat));
			m_pCollisionShape->setLocalScaling(Math::ConvertToBt(vScale));
		}

		Math::Matrix RigidBody::GetWorldMatrix()
		{
			btTransform bt;
			m_pMotionState->getWorldTransform(bt);

			return Math::Convert(bt);
		}

		Math::Quaternion RigidBody::GetOrientation()
		{
			return Math::Convert(m_pRigidBody->getOrientation());
		}

		Math::Vector3 RigidBody::GetCenterOfMassPosition()
		{
			return Math::Convert(m_pRigidBody->getCenterOfMassPosition());
		}

		void RigidBody::SetLinearVelocity(const Math::Vector3& f3Velocity)
		{
			if (Math::IsZero(f3Velocity.LengthSquared()) == true)
				return;

			if (m_pRigidBody->isActive() == false)
			{
				m_pRigidBody->setLinearVelocity(Math::ConvertToBt(f3Velocity));
				m_pRigidBody->activate();
			}
			else
			{
				auto btVelocity = m_pRigidBody->getLinearVelocity();
				btVelocity += Math::ConvertToBt(f3Velocity);

				m_pRigidBody->setLinearVelocity(btVelocity);
			}
		}

		void RigidBody::SetDamping(float fLinearDamping, float fAngularDamping)
		{
			m_pRigidBody->setDamping(fLinearDamping, fAngularDamping);
		}

		void RigidBody::SetDeactivationTime(float fTime)
		{
			m_pRigidBody->setDeactivationTime(fTime);
		}

		void RigidBody::SetSleepingThresholds(float fLinear, float fAngular)
		{
			m_pRigidBody->setSleepingThresholds(fLinear, fAngular);
		}

		void RigidBody::SetAngularFactor(float fFactor)
		{
			m_pRigidBody->setAngularFactor(fFactor);
		}

		void RigidBody::SetActiveState(EmActiveState::Type emActiveState)
		{
			if (m_pRigidBody != nullptr)
			{
				m_pRigidBody->setActivationState((int)(emActiveState));
			}
		}

		bool RigidBody::init(const RigidBodyProperty& rigidBodyProperty)
		{
			m_pCollisionShape = CreateShape(rigidBodyProperty.shapeInfo);
			if (m_pCollisionShape == nullptr)
				return false;

			if (rigidBodyProperty.GetShapeType() == EmPhysicsShape::eTriangleMesh)
			{
				const Shape::TriangleMesh* pShapeInfo = std::get_if<Shape::TriangleMesh>(&rigidBodyProperty.shapeInfo.element);
				if (pShapeInfo == nullptr)
					return false;

				if (pShapeInfo->pVertices != nullptr && pShapeInfo->pIndices != nullptr)
				{
					btBvhTriangleMeshShape* pTriangleMeshShape = static_cast<btBvhTriangleMeshShape*>(m_pCollisionShape);
					btTriangleIndexVertexArray* p1 = static_cast<btTriangleIndexVertexArray*>(pTriangleMeshShape->getMeshInterface());

					m_varTriangleMesh.emplace<btTriangleIndexVertexArray*>(p1);
				}
				else if (pShapeInfo->pVertices != nullptr)
				{
					btBvhTriangleMeshShape* pTriangleMeshShape = static_cast<btBvhTriangleMeshShape*>(m_pCollisionShape);
					btTriangleMesh* p1 = static_cast<btTriangleMesh*>(pTriangleMeshShape->getMeshInterface());

					m_varTriangleMesh.emplace<btTriangleMesh*>(p1);
				}
			}

			m_pDynamicsWorld = PhysicsSystem::GetInstance()->GetDynamicsWorld();

			m_rigidBodyProperty = rigidBodyProperty;

			btTransform offsetTransform = Math::ConvertToBt(rigidBodyProperty.matOffset);

			btTransform bodyTransform;
			bodyTransform.setIdentity();
			bodyTransform.setOrigin(Math::ConvertToBt(rigidBodyProperty.f3OriginPos));
			bodyTransform.setRotation(Math::ConvertToBt(rigidBodyProperty.originQuat));
			
			btVector3 localInertia(0.f, 0.f, 0.f);
			// IneritaTensor¸¦ °è»ê
			if ((rigidBodyProperty.nCollisionFlag & EmCollision::eStaticObject) == 0 &&
				(rigidBodyProperty.nCollisionFlag & EmCollision::eKinematicObject) == 0)
			{
				m_pCollisionShape->calculateLocalInertia(m_rigidBodyProperty.fMass, localInertia);
			}

			m_pMotionState = new btDefaultMotionState(offsetTransform * bodyTransform);
			btRigidBody::btRigidBodyConstructionInfo rbInfo(m_rigidBodyProperty.fMass, m_pMotionState, m_pCollisionShape, localInertia);
			rbInfo.m_restitution = m_rigidBodyProperty.fRestitution;
			rbInfo.m_friction = m_rigidBodyProperty.fFriction;
			rbInfo.m_linearDamping = m_rigidBodyProperty.fLinearDamping;
			rbInfo.m_angularDamping = m_rigidBodyProperty.fAngularDamping;

			m_pRigidBody = new btRigidBody(rbInfo);
			m_pRigidBody->setUserPointer(this);
			m_pRigidBody->setCollisionFlags(rigidBodyProperty.nCollisionFlag);

			PhysicsSystem::GetInstance()->AddRigidBody(this);

			SetName(rigidBodyProperty.strName);

			return true;
		}
	}
}