#pragma once

#include "Physics/PhysicsInterface.h"
#include "ComponentInterface.h"

namespace est
{
	namespace gameobject
	{
		enum : uint32_t
		{
			eAttachToActor = std::numeric_limits<uint32_t>::max(),
		};

		class ComponentPhysics : public IComponent
		{
		public:
			ComponentPhysics(IActor* pOwner);
			virtual ~ComponentPhysics();

		public:
			virtual void Update(float elapsedTime, float lodThreshold) override;

		public:
			physics::IRigidActor* CreateRigidActor(const physics::RigidActorProperty& rigidActorProperty);
			physics::IRigidActor* CreateRigidActor(uint32_t id, const physics::RigidActorProperty& rigidActorProperty);

		public:
			void SetGlobalTransform(const math::float3& position, const math::Quaternion& rotation, bool isEnableAutoWake = true);
			void SetGlobalPosition(const math::float3& position, bool isEnableAutoWake = true);
			void SetGlobalRotation(const math::Quaternion& rotation, bool isEnableAutoWake = true);

			void SetAngularVelocity(const math::float3& velocity);
			void SetLinearVelocity(const math::float3& velocity);

		public:
			bool Raycast(const collision::Ray& ray, float distance, physics::HitLocation* pHitLocation = nullptr, physics::HitActorShape* pHitActorShape = nullptr, const physics::QueryFilterData& queryFilterData = {});

		private:
			struct PhysicsNode
			{
				uint32_t id{ eAttachToActor };
				std::unique_ptr<physics::IRigidActor> pRigidActor;

				PhysicsNode(uint32_t id, std::unique_ptr<physics::IRigidActor> pRigidActor);
			};
			std::vector<PhysicsNode> m_physicsNodes;
		};
	}
}