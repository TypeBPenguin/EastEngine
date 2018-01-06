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
			Graphics::IModelInstance* pPhysicsModelInst = nullptr;

			PhysicsNode()
			{
			}

			PhysicsNode(const Math::Matrix* pMatWorld, Physics::RigidBody* pRigidBody, Graphics::IModelInstance* pPhysicsModelInst)
				: pMatWorld(pMatWorld)
				, pRigidBody(pRigidBody)
				, pPhysicsModelInst(pPhysicsModelInst)
			{
			}
		};

		class ComponentPhysics : public IComponent
		{
		public:
			ComponentPhysics(IActor* pOwner);
			virtual ~ComponentPhysics();

			RagDoll* m_pRagDoll;

		public:
			void Init(const Physics::RigidBodyProperty& rigidBodyProperty, bool isCollisionModelVisible = false);
			void Init(Graphics::IModelInstance* pModelInst, const Physics::RigidBodyProperty& rigidBodyProperty, uint32_t nTargetLod = 0, bool isCollisionModelVisible = false);
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
			void initPhysics(const String::StringID& strID, const Graphics::IVertexBuffer* pVertexBuffer, const Graphics::IIndexBuffer* pIndexBuffer, const Math::Matrix* pMatWorld, Physics::RigidBodyProperty& rigidBodyProperty);

		private:
			std::unordered_map<String::StringID, PhysicsNode> m_umapPhysicsNode;

			enum Type
			{
				eBasic = 0,
				eModel,
				eCustom,
			} m_emType;

			union PhysicsLoader
			{
				struct Basic
				{
					Physics::RigidBodyProperty rigidBodyProperty;

					void Set(const Physics::RigidBodyProperty& _rigidBodyProperty)
					{
						rigidBodyProperty = _rigidBodyProperty;
					}
				} byBasic;

				struct ByModel
				{
					Graphics::IModelInstance* pModelInst;
					Physics::RigidBodyProperty rigidBodyProperty;
					uint32_t nTargetLod;

					void Set(Graphics::IModelInstance* _pModelInst, const Physics::RigidBodyProperty& _rigidBodyProperty, uint32_t _nTargetLod)
					{
						pModelInst = _pModelInst;
						rigidBodyProperty = _rigidBodyProperty;
						nTargetLod = _nTargetLod;
					}
				} byModel;

				struct ByCustom
				{
					String::StringID strID;
					const Graphics::IVertexBuffer* pVertexBuffer;
					const Graphics::IIndexBuffer* pIndexBuffer;
					Math::Matrix* pMatWorld;
					Physics::RigidBodyProperty rigidBodyProperty;

					void Set(const String::StringID& _strID, const Graphics::IVertexBuffer* _pVertexBuffer, const Graphics::IIndexBuffer* _pIndexBuffer, Math::Matrix* _pMatWorld, const Physics::RigidBodyProperty& _rigidBodyProperty)
					{
						strID = _strID;
						pVertexBuffer = _pVertexBuffer;
						pIndexBuffer = _pIndexBuffer;
						pMatWorld = _pMatWorld;
						rigidBodyProperty = _rigidBodyProperty;
					}
				} byCustom;

				PhysicsLoader() {}
				~PhysicsLoader() {}
			} m_loader;

			bool m_isInit;
			bool m_isCollisionModelVisible;
		};
	}
}