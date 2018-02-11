#include "stdafx.h"
#include "RigidBody.h"

#include "PhysicsSystem.h"
#include "DebugHelper.h"
#include "MathConvertor.h"

namespace EastEngine
{
	namespace Physics
	{
		class RigidBody::Impl
		{
		public:
			Impl();
			~Impl();

		public:
			bool Initislize(const RigidBodyProperty& physicsProperty);

		public:
			void Update(float fElapsedTime);
			void UpdateBoundingBox(const Math::Matrix& matWorld);

			bool IsCollision(RigidBody* pRigidBody);
			bool RayTest(const Math::Vector3& f3From, const Math::Vector3& f3To, Math::Vector3* pHitPoint_out = nullptr, Math::Vector3* pHitNormal_out = nullptr) const;

			void AddCollisionResult(const RigidBody* pRigidBody, const Math::Vector3& f3OpponentPoint, const Math::Vector3& f3MyPoint);

			void ClearCollisionResults();

			void SetEnableTriangleDrawCallback(bool isEnableTriangleDrawCallback);

		public:
			const String::StringID& GetName() const;
			void SetName(const String::StringID& strName);

			const RigidBodyProperty& GetRigidBodyProperty() const;

			void SetWorldMatrix(const Math::Matrix& mat);
			Math::Matrix GetWorldMatrix() const;

			Math::Quaternion GetOrientation() const;
			Math::Vector3 GetCenterOfMassPosition() const;

			void SetLinearVelocity(const Math::Vector3& f3Velocity);

			void SetDamping(float fLinearDamping, float fAngularDamping);
			void SetDeactivationTime(float fTime);
			void SetSleepingThresholds(float fLinear, float fAngular);
			void SetAngularFactor(float fFactor);

			void SetActiveState(EmActiveState::Type emActiveState);
			void SetGravity(bool isEnable);
			void SetGravity(const Math::Vector3& f3Gravity);

			const Collision::AABB& GetAABB() const { return m_boundingBox; }
			const Collision::Sphere& GetBoundingSphere() const { return m_boundingSphere; }
			const Collision::OBB& GetOBB() const { return m_boundingOrientedBox; }

			btRigidBody* GetInterface() { return m_pRigidBody.get(); }

		private:
			btDiscreteDynamicsWorld* m_pDynamicsWorld{ nullptr };

			std::unique_ptr<btRigidBody> m_pRigidBody;
			std::unique_ptr<btCollisionShape> m_pCollisionShape;
			std::unique_ptr<btMotionState> m_pMotionState;

			std::variant<btTriangleMesh*, btTriangleIndexVertexArray*> m_varTriangleMesh;

			RigidBodyProperty m_rigidBodyProperty;

			Math::Matrix m_matWorld;

			Collision::AABB m_boundingBox;
			Collision::Sphere m_boundingSphere;
			Collision::OBB m_boundingOrientedBox;

			std::vector<CollisionResult> m_vecCollisionResults;
			bool m_isEnableTriangleDrawCallback{ false };
		};

		RigidBody::Impl::Impl()
		{
		}

		RigidBody::Impl::~Impl()
		{
			if (m_pDynamicsWorld != nullptr)
			{
				m_pDynamicsWorld->removeRigidBody(m_pRigidBody.get());
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

			m_pCollisionShape.reset();
			m_pMotionState.reset();
			m_pRigidBody.reset();
		}

		bool RigidBody::Impl::Initislize(const RigidBodyProperty& rigidBodyProperty)
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
					btBvhTriangleMeshShape* pTriangleMeshShape = static_cast<btBvhTriangleMeshShape*>(m_pCollisionShape.get());
					btTriangleIndexVertexArray* p1 = static_cast<btTriangleIndexVertexArray*>(pTriangleMeshShape->getMeshInterface());

					m_varTriangleMesh.emplace<btTriangleIndexVertexArray*>(p1);
				}
				else if (pShapeInfo->pVertices != nullptr)
				{
					btBvhTriangleMeshShape* pTriangleMeshShape = static_cast<btBvhTriangleMeshShape*>(m_pCollisionShape.get());
					btTriangleMesh* p1 = static_cast<btTriangleMesh*>(pTriangleMeshShape->getMeshInterface());

					m_varTriangleMesh.emplace<btTriangleMesh*>(p1);
				}
			}

			m_pDynamicsWorld = System::GetInstance()->GetDynamicsWorld();

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

			m_pMotionState = std::make_unique<btDefaultMotionState>(offsetTransform * bodyTransform);
			btRigidBody::btRigidBodyConstructionInfo rbInfo(m_rigidBodyProperty.fMass, m_pMotionState.get(), m_pCollisionShape.get(), localInertia);
			rbInfo.m_restitution = m_rigidBodyProperty.fRestitution;
			rbInfo.m_friction = m_rigidBodyProperty.fFriction;
			rbInfo.m_linearDamping = m_rigidBodyProperty.fLinearDamping;
			rbInfo.m_angularDamping = m_rigidBodyProperty.fAngularDamping;

			m_pRigidBody = std::make_unique<btRigidBody>(rbInfo);
			m_pRigidBody->setUserPointer(this);
			m_pRigidBody->setCollisionFlags(rigidBodyProperty.nCollisionFlag);

			SetName(rigidBodyProperty.strName);

			return true;
		}

		void RigidBody::Impl::Update(float fElapsedTime)
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
					btConcaveShape* pShape = static_cast<btConcaveShape*>(m_pCollisionShape.get());

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

		void RigidBody::Impl::UpdateBoundingBox(const Math::Matrix& matWorld)
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

		bool RigidBody::Impl::IsCollision(RigidBody* pRigidBody)
		{
			auto iter = std::find_if(m_vecCollisionResults.begin(), m_vecCollisionResults.end(), [pRigidBody](const CollisionResult& result)
			{
				return result.pOpponentObject == pRigidBody;
			});

			if (iter != m_vecCollisionResults.end())
				return true;

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

			m_pDynamicsWorld->contactPairTest(m_pRigidBody.get(), pRigidBody->GetInterface(), result);

			return result.isCollision;
		}

		bool RigidBody::Impl::RayTest(const Math::Vector3& f3From, const Math::Vector3& f3To, Math::Vector3* pHitPoint_out, Math::Vector3* pHitNormal_out) const
		{
			btVector3 rayFromWorld = Math::ConvertToBt(f3From);
			btVector3 rayToWorld = Math::ConvertToBt(f3To);

			btTransform	rayFromTrans;
			rayFromTrans.setIdentity();
			rayFromTrans.setOrigin(rayFromWorld);

			btTransform	rayToTrans;
			rayToTrans.setIdentity();
			rayToTrans.setOrigin(rayToWorld);

			btCollisionWorld::ClosestRayResultCallback resultCallback(rayFromWorld, rayToWorld);

			m_pDynamicsWorld->rayTestSingle(rayFromTrans, rayToTrans,
				m_pRigidBody.get(),
				m_pCollisionShape.get(),
				m_pRigidBody->getWorldTransform(),
				resultCallback);

			if (resultCallback.hasHit() == true)
			{
				if (pHitPoint_out != nullptr)
				{
					*pHitPoint_out = Math::Convert(resultCallback.m_hitPointWorld);
				}

				if (pHitNormal_out != nullptr)
				{
					*pHitNormal_out = Math::Convert(resultCallback.m_hitNormalWorld);
				}

				return true;
			}
			else
			{
				if (pHitPoint_out != nullptr)
				{
					*pHitPoint_out = Math::Vector3::Zero;
				}

				if (pHitNormal_out != nullptr)
				{
					*pHitNormal_out = Math::Vector3::Zero;
				}

				return false;
			}
		}

		void RigidBody::Impl::AddCollisionResult(const RigidBody* pRigidBody, const Math::Vector3& f3OpponentPoint, const Math::Vector3& f3MyPoint)
		{
			if (m_rigidBodyProperty.funcCollisionCallback != nullptr)
			{
				m_vecCollisionResults.emplace_back(pRigidBody, f3OpponentPoint, f3MyPoint);
			}
		}

		void RigidBody::Impl::ClearCollisionResults()
		{
			m_vecCollisionResults.clear();
		}

		void RigidBody::Impl::SetEnableTriangleDrawCallback(bool isEnableTriangleDrawCallback)
		{
			m_isEnableTriangleDrawCallback = isEnableTriangleDrawCallback;
		}

		const String::StringID& RigidBody::Impl::GetName() const
		{
			return m_rigidBodyProperty.strName;
		}

		void RigidBody::Impl::SetName(const String::StringID& strName)
		{
			m_rigidBodyProperty.strName = strName;
		}

		const RigidBodyProperty& RigidBody::Impl::GetRigidBodyProperty() const
		{
			return m_rigidBodyProperty;
		}

		void RigidBody::Impl::SetWorldMatrix(const Math::Matrix& mat)
		{
			Math::Vector3 f3Pos, vScale;
			Math::Quaternion quat;

			mat.Decompose(vScale, quat, f3Pos);
			btTransform transform = Math::ConvertToBt(f3Pos, quat);
			m_pMotionState->setWorldTransform(transform);
			m_pRigidBody->setWorldTransform(transform);
			m_pCollisionShape->setLocalScaling(Math::ConvertToBt(vScale));
		}

		Math::Matrix RigidBody::Impl::GetWorldMatrix() const
		{
			return Math::Convert(m_pRigidBody->getWorldTransform());
		}

		Math::Quaternion RigidBody::Impl::GetOrientation() const
		{
			return Math::Convert(m_pRigidBody->getOrientation());
		}

		Math::Vector3 RigidBody::Impl::GetCenterOfMassPosition() const
		{
			return Math::Convert(m_pRigidBody->getCenterOfMassPosition());
		}

		void RigidBody::Impl::SetLinearVelocity(const Math::Vector3& f3Velocity)
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

		void RigidBody::Impl::SetDamping(float fLinearDamping, float fAngularDamping)
		{
			m_pRigidBody->setDamping(fLinearDamping, fAngularDamping);
		}

		void RigidBody::Impl::SetDeactivationTime(float fTime)
		{
			m_pRigidBody->setDeactivationTime(fTime);
		}

		void RigidBody::Impl::SetSleepingThresholds(float fLinear, float fAngular)
		{
			m_pRigidBody->setSleepingThresholds(fLinear, fAngular);
		}

		void RigidBody::Impl::SetAngularFactor(float fFactor)
		{
			m_pRigidBody->setAngularFactor(fFactor);
		}

		void RigidBody::Impl::SetActiveState(EmActiveState::Type emActiveState)
		{
			m_pRigidBody->setActivationState((int)(emActiveState));
		}

		void RigidBody::Impl::SetGravity(bool isEnable)
		{
			if (isEnable == true)
			{
				m_pRigidBody->setGravity(m_pDynamicsWorld->getGravity());
			}
			else
			{
				const btVector3 zeroVector(0.f, 0.f, 0.f);
				m_pRigidBody->setGravity(zeroVector);
			}

			m_pRigidBody->applyGravity();
		}

		void RigidBody::Impl::SetGravity(const Math::Vector3& f3Gravity)
		{
			m_pRigidBody->setGravity(Math::ConvertToBt(f3Gravity));
			m_pRigidBody->applyGravity();
		}

		RigidBody::RigidBody()
			: m_pImpl{ std::make_unique<Impl>() }
		{
		}

		RigidBody::~RigidBody()
		{
		}

		RigidBody* RigidBody::Create(const RigidBodyProperty& rigidBodyProperty)
		{
			if (rigidBodyProperty.GetShapeType() == EmPhysicsShape::eEmpty)
				return nullptr;

			RigidBody* pRigidBody = new RigidBody;
			if (pRigidBody->Initislize(rigidBodyProperty) == false)
			{
				SafeDelete(pRigidBody);

				return nullptr;
			}

			System::GetInstance()->AddRigidBody(pRigidBody);
			
			return pRigidBody;
		}

		void RigidBody::Update(float fElapsedTime)
		{
			m_pImpl->Update(fElapsedTime);
		}

		void RigidBody::UpdateBoundingBox(const Math::Matrix& matWorld)
		{
			m_pImpl->UpdateBoundingBox(matWorld);
		}

		bool RigidBody::IsCollision(RigidBody* pRigidBody)
		{
			return m_pImpl->IsCollision(pRigidBody);
		}

		bool RigidBody::RayTest(const Math::Vector3& f3From, const Math::Vector3& f3To, Math::Vector3* pHitPoint_out, Math::Vector3* pHitNormal_out) const
		{
			return m_pImpl->RayTest(f3From, f3To, pHitPoint_out, pHitNormal_out);
		}

		void RigidBody::AddCollisionResult(const RigidBody* pRigidBody, const Math::Vector3& f3OpponentPoint, const Math::Vector3& f3MyPoint)
		{
			m_pImpl->AddCollisionResult(pRigidBody, f3OpponentPoint, f3MyPoint);
		}

		void RigidBody::ClearCollisionResults()
		{
			m_pImpl->ClearCollisionResults();
		}

		void RigidBody::SetEnableTriangleDrawCallback(bool isEnableTriangleDrawCallback)
		{
			m_pImpl->SetEnableTriangleDrawCallback(isEnableTriangleDrawCallback);
		}

		const String::StringID& RigidBody::GetName() const
		{
			return m_pImpl->GetName();
		}

		void RigidBody::SetName(const String::StringID& strName)
		{
			m_pImpl->SetName(strName);
		}

		const RigidBodyProperty& RigidBody::GetRigidBodyProperty() const
		{
			return m_pImpl->GetRigidBodyProperty();
		}

		void RigidBody::SetWorldMatrix(const Math::Matrix& mat)
		{
			m_pImpl->SetWorldMatrix(mat);
		}

		Math::Matrix RigidBody::GetWorldMatrix() const
		{
			return m_pImpl->GetWorldMatrix();
		}

		Math::Quaternion RigidBody::GetOrientation() const
		{
			return m_pImpl->GetOrientation();
		}

		Math::Vector3 RigidBody::GetCenterOfMassPosition() const
		{
			return m_pImpl->GetCenterOfMassPosition();
		}

		void RigidBody::SetLinearVelocity(const Math::Vector3& f3Velocity)
		{
			m_pImpl->SetLinearVelocity(f3Velocity);
		}

		void RigidBody::SetDamping(float fLinearDamping, float fAngularDamping)
		{
			m_pImpl->SetDamping(fLinearDamping, fAngularDamping);
		}

		void RigidBody::SetDeactivationTime(float fTime)
		{
			m_pImpl->SetDeactivationTime(fTime);
		}

		void RigidBody::SetSleepingThresholds(float fLinear, float fAngular)
		{
			m_pImpl->SetSleepingThresholds(fLinear, fAngular);
		}

		void RigidBody::SetAngularFactor(float fFactor)
		{
			m_pImpl->SetAngularFactor(fFactor);
		}

		void RigidBody::SetActiveState(EmActiveState::Type emActiveState)
		{
			m_pImpl->SetActiveState(emActiveState);
		}

		void RigidBody::SetGravity(bool isEnable)
		{
			m_pImpl->SetGravity(isEnable);
		}

		void RigidBody::SetGravity(const Math::Vector3& f3Gravity)
		{
			m_pImpl->SetGravity(f3Gravity);
		}

		const Collision::AABB& RigidBody::GetAABB() const
		{
			return m_pImpl->GetAABB();
		}

		const Collision::Sphere& RigidBody::GetBoundingSphere() const
		{
			return m_pImpl->GetBoundingSphere();
		}

		const Collision::OBB& RigidBody::GetOBB() const
		{
			return m_pImpl->GetOBB();
		}

		btRigidBody* RigidBody::GetInterface()
		{
			return m_pImpl->GetInterface();
		}
		
		bool RigidBody::Initislize(const RigidBodyProperty& physicsProperty)
		{
			return m_pImpl->Initislize(physicsProperty);
		}
	}
}