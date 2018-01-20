#pragma once

#include "Physics/RigidBody.h"
#include "Physics/Constraint.h"
#include "Physics/GhostObject.h"
#include "Physics/KinematicCharacterController.h"

#include "Model/ModelInterface.h"

#include "GameObject/RagDoll.h"

#include "ComponentInterface.h"

namespace EastEngine
{
	namespace Graphics
	{
		class IVertexBuffer;
		class IIndexBuffer;
		class IModelInstance;
		class IModelNode;
	}

	namespace GameObject
	{
		class RagDoll;

		struct PhysicsNode
		{
			const Math::Matrix* pMatWorld = nullptr;
			Physics::RigidBody* pRigidBody = nullptr;
			Graphics::IModelInstance* pPhysicsModelInstance = nullptr;
			Graphics::IModelInstance* pModelInstance = nullptr;

			PhysicsNode(const Math::Matrix* pMatWorld, Physics::RigidBody* pRigidBody, Graphics::IModelInstance* pPhysicsModelInstance, Graphics::IModelInstance* pModelInstance);
		};

		class ComponentPhysics : public IComponent
		{
		public:
			ComponentPhysics(IActor* pOwner);
			virtual ~ComponentPhysics();

			RagDoll* m_pRagDoll;

		public:
			void Init(const Physics::RigidBodyProperty& rigidBodyProperty, bool isCollisionModelVisible = false);
			void Init(Graphics::IModelInstance* pModelInstance, const Physics::RigidBodyProperty& rigidBodyProperty, uint32_t nTargetLod = 0, bool isCollisionModelVisible = false);
			void Init(const String::StringID& strID, const Graphics::IVertexBuffer* pVertexBuffer, const Graphics::IIndexBuffer* pIndexBuffer, Math::Matrix* pMatWorld, const Physics::RigidBodyProperty& rigidBodyProperty, bool isCollisionModelVisible = false);
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
			void initPhysics(const String::StringID& strID, const Physics::RigidBodyProperty& rigidBodyProperty, const Math::Matrix* pMatWorld, Graphics::IModelInstance* pModelInstance);

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
				Graphics::IModelInstance* pModelInstance = nullptr;
				Physics::RigidBodyProperty rigidBodyProperty;
				uint32_t nTargetLod = 0;

				void Set(Graphics::IModelInstance* _pModelInstance, const Physics::RigidBodyProperty& _rigidBodyProperty, uint32_t _nTargetLod)
				{
					pModelInstance = _pModelInstance;
					rigidBodyProperty = _rigidBodyProperty;
					nTargetLod = _nTargetLod;
				}
			};

			struct CustomRigidBody
			{
				String::StringID strID;
				const Graphics::IVertexBuffer* pVertexBuffer = nullptr;
				const Graphics::IIndexBuffer* pIndexBuffer = nullptr;
				Math::Matrix* pMatWorld = nullptr;
				Physics::RigidBodyProperty rigidBodyProperty;

				void Set(const String::StringID& _strID, const Graphics::IVertexBuffer* _pVertexBuffer, const Graphics::IIndexBuffer* _pIndexBuffer, Math::Matrix* _pMatWorld, const Physics::RigidBodyProperty& _rigidBodyProperty)
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