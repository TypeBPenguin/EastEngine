#pragma once

#include "Physics/RigidBody.h"
#include "Physics/Constraint.h"
#include "Physics/GhostObject.h"
#include "Physics/KinematicCharacterController.h"

#include "Model/ModelInterface.h"

#include "GameObject/RagDoll.h"

#include "ComponentInterface.h"

namespace eastengine
{
	namespace graphics
	{
		class IVertexBuffer;
		class IIndexBuffer;
		class IModelInstance;
		class IModelNode;
	}

	namespace gameobject
	{
		class RagDoll;

		struct PhysicsNode
		{
			const math::Matrix* pMatWorld = nullptr;
			Physics::RigidBody* pRigidBody = nullptr;
			graphics::IModelInstance* pPhysicsModelInstance = nullptr;
			graphics::IModelInstance* pModelInstance = nullptr;

			PhysicsNode(const math::Matrix* pMatWorld, Physics::RigidBody* pRigidBody, graphics::IModelInstance* pPhysicsModelInstance, graphics::IModelInstance* pModelInstance);
		};

		class ComponentPhysics : public IComponent
		{
		public:
			ComponentPhysics(IActor* pOwner);
			virtual ~ComponentPhysics();

			RagDoll* m_pRagDoll;

		public:
			void Init(const Physics::RigidBodyProperty& rigidBodyProperty, bool isCollisionModelVisible = false);
			void Init(graphics::IModelInstance* pModelInstance, const Physics::RigidBodyProperty& rigidBodyProperty, uint32_t nTargetLod = 0, bool isCollisionModelVisible = false);
			void Init(const String::StringID& strID, const graphics::IVertexBuffer* pVertexBuffer, const graphics::IIndexBuffer* pIndexBuffer, math::Matrix* pMatWorld, const Physics::RigidBodyProperty& rigidBodyProperty, bool isCollisionModelVisible = false);
			virtual void Update(float fElapsedTime) override;

		public:
			void SetActiveState(Physics::EmActiveState::Type emActiveState);

			const PhysicsNode* GetPhysicsNode(const String::StringID& strName)
			{
				auto iter = m_umapPhysicsNode.find(strName);
				if (iter != m_umapPhysicsNode.end())
					return &iter->second;

				return nullptr;
			}

		private:
			void initPhysics(const String::StringID& strID, const Physics::RigidBodyProperty& rigidBodyProperty, const math::Matrix* pMatWorld, graphics::IModelInstance* pModelInstance);

		private:
			bool m_isInit;
			bool m_isCollisionModelVisible;

			std::unordered_map<String::StringID, PhysicsNode> m_umapPhysicsNode;

			enum RigidBodyType
			{
				eNone = 0,
				eBasic,
				eModel,
				eCustom,
			};

			struct BasicRigidBody
			{
				Physics::RigidBodyProperty rigidBodyProperty;

				void Set(const Physics::RigidBodyProperty& _rigidBodyProperty)
				{
					rigidBodyProperty = _rigidBodyProperty;
				}
			};

			struct ModelRigidBody
			{
				graphics::IModelInstance* pModelInstance = nullptr;
				Physics::RigidBodyProperty rigidBodyProperty;
				uint32_t nTargetLod = 0;

				void Set(graphics::IModelInstance* _pModelInstance, const Physics::RigidBodyProperty& _rigidBodyProperty, uint32_t _nTargetLod)
				{
					pModelInstance = _pModelInstance;
					rigidBodyProperty = _rigidBodyProperty;
					nTargetLod = _nTargetLod;
				}
			};

			struct CustomRigidBody
			{
				String::StringID strID;
				const graphics::IVertexBuffer* pVertexBuffer = nullptr;
				const graphics::IIndexBuffer* pIndexBuffer = nullptr;
				math::Matrix* pMatWorld = nullptr;
				Physics::RigidBodyProperty rigidBodyProperty;

				void Set(const String::StringID& _strID, const graphics::IVertexBuffer* _pVertexBuffer, const graphics::IIndexBuffer* _pIndexBuffer, math::Matrix* _pMatWorld, const Physics::RigidBodyProperty& _rigidBodyProperty)
				{
					strID = _strID;
					pVertexBuffer = _pVertexBuffer;
					pIndexBuffer = _pIndexBuffer;
					pMatWorld = _pMatWorld;
					rigidBodyProperty = _rigidBodyProperty;
				}
			};

			RigidBodyType m_emRigidBodyType;
			std::variant<BasicRigidBody, ModelRigidBody, CustomRigidBody> m_rigidBodyElements;
		};
	}
}