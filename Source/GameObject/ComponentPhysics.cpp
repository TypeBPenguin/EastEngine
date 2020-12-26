#include "stdafx.h"
#include "ComponentPhysics.h"

#include "Graphics/Model/ModelInterface.h"

#include "GameObject.h"

namespace est
{
	namespace gameobject
	{
		ComponentPhysics::PhysicsNode::PhysicsNode(uint32_t id, std::unique_ptr<physics::IRigidActor> pRigidActor)
			: id(id)
			, pRigidActor(std::move(pRigidActor))
		{
		}

		ComponentPhysics::ComponentPhysics(IActor* pOwner)
			: IComponent(pOwner, IComponent::ePhysics)
		{
		}

		ComponentPhysics::~ComponentPhysics()
		{
		}

		void ComponentPhysics::Update(float elapsedTime, float lodThreshold)
		{
			for (auto& physicsNode : m_physicsNodes)
			{
				if (physicsNode.pRigidActor == nullptr)
					continue;

				switch (physicsNode.pRigidActor->GetType())
				{
				case physics::IActor::Type::eRigidStatic:
				{
					//physics::IRigidStatic* pRigidStatic = static_cast<physics::IRigidStatic*>(physicsNode.pRigidActor.get());
				}
				break;
				case physics::IActor::Type::eRigidDynamic:
				{
					//physics::IRigidDynamic* pRigidDynamic = static_cast<physics::IRigidDynamic*>(physicsNode.pRigidActor.get());
				}
				break;
				default:
					throw_line("unknown type");
					break;
				}

				if (physicsNode.id == eAttachToActor)
				{
					const physics::Transform globalTransform = physicsNode.pRigidActor->GetGlobalTransform();
					m_pOwner->SetPosition(globalTransform.position);
					m_pOwner->SetRotation(globalTransform.rotation);
				}
			}
		}

		physics::IRigidActor* ComponentPhysics::CreateRigidActor(const physics::RigidActorProperty& rigidActorProperty)
		{
			if (rigidActorProperty.shape.pGeometry == nullptr)
			{
				LOG_ERROR(L"failed to create rigid actor, uninitialized geometry");
				return nullptr;
			}

			auto iter = std::find_if(m_physicsNodes.begin(), m_physicsNodes.end(), [](const PhysicsNode& physicsNode)
			{
				return physicsNode.id == eAttachToActor;
			});

			if (iter != m_physicsNodes.end())
				return iter->pRigidActor.get();

			std::unique_ptr<physics::IRigidActor> pRigidActor = physics::CreateRigidActor(rigidActorProperty);
			pRigidActor->SetUserData(m_pOwner);

			physics::scene::AddActor(pRigidActor.get());
			PhysicsNode& physicsNode = m_physicsNodes.emplace_back(eAttachToActor, std::move(pRigidActor));
			return physicsNode.pRigidActor.get();
		}

		physics::IRigidActor* ComponentPhysics::CreateRigidActor(uint32_t id, const physics::RigidActorProperty& rigidActorProperty)
		{
			if (rigidActorProperty.shape.pGeometry == nullptr)
			{
				LOG_ERROR(L"failed to create rigid actor, uninitialized geometry");
				return nullptr;
			}

			if (id == eAttachToActor)
			{
				LOG_ERROR(L"failed to create rigid actor, because modelInstanceID == eRigidActor");
				return nullptr;
			}

			auto iter = std::find_if(m_physicsNodes.begin(), m_physicsNodes.end(), [id](const PhysicsNode& physicsNode)
			{
				return physicsNode.id == id;
			});

			if (iter != m_physicsNodes.end())
				return iter->pRigidActor.get();

			std::unique_ptr<physics::IRigidActor> pRigidActor = physics::CreateRigidActor(rigidActorProperty);
			pRigidActor->SetUserData(m_pOwner);

			physics::scene::AddActor(pRigidActor.get());
			PhysicsNode& physicsNode = m_physicsNodes.emplace_back(id, std::move(pRigidActor));
			return physicsNode.pRigidActor.get();
		}

		void ComponentPhysics::SetGlobalTransform(const math::float3& position, const math::Quaternion& rotation, bool isEnableAutoWake)
		{
			auto iter = std::find_if(m_physicsNodes.begin(), m_physicsNodes.end(), [](const PhysicsNode& physicsNode)
				{
					return physicsNode.id == eAttachToActor;
				});
			
			if (iter != m_physicsNodes.end())
			{
				physics::Transform transform;
				transform.position = position;
				transform.rotation = rotation;
				iter->pRigidActor->SetGlobalTransform(transform, isEnableAutoWake);
			}
		}

		void ComponentPhysics::SetGlobalPosition(const math::float3& position, bool isEnableAutoWake)
		{
			auto iter = std::find_if(m_physicsNodes.begin(), m_physicsNodes.end(), [](const PhysicsNode& physicsNode)
				{
					return physicsNode.id == eAttachToActor;
				});

			if (iter != m_physicsNodes.end())
			{
				physics::Transform transform = iter->pRigidActor->GetGlobalTransform();
				transform.position = position;
				iter->pRigidActor->SetGlobalTransform(transform, isEnableAutoWake);
			}
		}

		void ComponentPhysics::SetGlobalRotation(const math::Quaternion& rotation, bool isEnableAutoWake)
		{
			auto iter = std::find_if(m_physicsNodes.begin(), m_physicsNodes.end(), [](const PhysicsNode& physicsNode)
				{
					return physicsNode.id == eAttachToActor;
				});

			if (iter != m_physicsNodes.end())
			{
				physics::Transform transform = iter->pRigidActor->GetGlobalTransform();
				transform.rotation = rotation;
				iter->pRigidActor->SetGlobalTransform(transform, isEnableAutoWake);
			}
		}

		void ComponentPhysics::SetAngularVelocity(const math::float3& velocity)
		{
			for (auto& physicsNode : m_physicsNodes)
			{
				if (physicsNode.pRigidActor == nullptr)
					continue;

				if (physicsNode.pRigidActor->GetType() == physics::IActor::Type::eRigidDynamic)
				{
					physics::IRigidDynamic* pRigidStatic = static_cast<physics::IRigidDynamic*>(physicsNode.pRigidActor.get());
					pRigidStatic->SetAngularVelocity(velocity);
				}
			}
		}

		void ComponentPhysics::SetLinearVelocity(const math::float3& velocity)
		{
			for (auto& physicsNode : m_physicsNodes)
			{
				if (physicsNode.pRigidActor == nullptr)
					continue;

				if (physicsNode.pRigidActor->GetType() == physics::IActor::Type::eRigidDynamic)
				{
					physics::IRigidDynamic* pRigidStatic = static_cast<physics::IRigidDynamic*>(physicsNode.pRigidActor.get());
					pRigidStatic->SetLinearVelocity(velocity);
				}
			}
		}

		bool ComponentPhysics::Raycast(const collision::Ray& ray, float distance, physics::HitLocation* pHitLocation, physics::HitActorShape* pHitActorShape, const physics::QueryFilterData& queryFilterData)
		{
			for (auto& physicsNode : m_physicsNodes)
			{
				if (physicsNode.pRigidActor == nullptr)
					continue;

				physics::QueryCache queryCache;
				queryCache.pActor = physicsNode.pRigidActor.get();
				queryCache.pShape = physicsNode.pRigidActor->GetShape(0).get();

				if (physics::scene::Raycast(ray.position, ray.direction, distance, pHitLocation, pHitActorShape, queryFilterData, queryCache) == true)
					return true;
			}

			return false;
		}
	}
}