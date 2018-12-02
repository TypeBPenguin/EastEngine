#include "stdafx.h"
#include "ComponentPhysics.h"

#include "Model/ModelInterface.h"
#include "Model/ModelLoader.h"

#include "RagDoll.h"

#include "GameObject.h"

namespace eastengine
{
	namespace gameobject
	{
		PhysicsNode::PhysicsNode(const math::Matrix* pMatWorld, physics::RigidBody* pRigidBody, graphics::IModelInstance* pPhysicsModelInstance, graphics::IModelInstance* pModelInstance)
			: pMatWorld(pMatWorld)
			, pRigidBody(pRigidBody)
			, pPhysicsModelInstance(pPhysicsModelInstance)
			, pModelInstance(pModelInstance)
		{
		}

		ComponentPhysics::ComponentPhysics(IActor* pOwner)
			: IComponent(pOwner, IComponent::ePhysics)
			, m_isInit(false)
			, m_isCollisionModelVisible(false)
			, m_emRigidBodyType(RigidBodyType::eNone)
			, m_pRagDoll(new RagDoll)
		{
		}

		ComponentPhysics::~ComponentPhysics()
		{
			SafeDelete(m_pRagDoll);

			std::for_each(m_umapPhysicsNode.begin(), m_umapPhysicsNode.end(), [](std::pair<string::StringID, PhysicsNode> key)
			{
				SafeDelete(key.second.pRigidBody);
				graphics::IModel::DestroyInstance(&key.second.pPhysicsModelInstance);
			});
			m_umapPhysicsNode.clear();
		}

		void ComponentPhysics::Init(const physics::RigidBodyProperty& rigidBodyProperty, bool isCollisionModelVisible)
		{
			m_emRigidBodyType = RigidBodyType::eBasic;

			BasicRigidBody& basicRigidBody = m_rigidBodyElements.emplace<BasicRigidBody>();
			basicRigidBody.Set(rigidBodyProperty);

			m_isCollisionModelVisible = isCollisionModelVisible;
		}

		void ComponentPhysics::Init(graphics::IModelInstance* pModelInstance, const physics::RigidBodyProperty& rigidBodyProperty, uint32_t nTargetLod, bool isCollisionModelVisible)
		{
			m_emRigidBodyType = RigidBodyType::eModel;

			ModelRigidBody& modelRigidBody = m_rigidBodyElements.emplace<ModelRigidBody>();
			modelRigidBody.Set(pModelInstance, rigidBodyProperty, nTargetLod);

			m_isCollisionModelVisible = isCollisionModelVisible;
		}

		void ComponentPhysics::Init(const string::StringID& strID, const graphics::IVertexBuffer* pVertexBuffer, const graphics::IIndexBuffer* pIndexBuffer, math::Matrix* pMatWorld, const physics::RigidBodyProperty& rigidBodyProperty, bool isCollisionModelVisible)
		{
			m_emRigidBodyType = RigidBodyType::eCustom;

			CustomRigidBody& customRigidBody = m_rigidBodyElements.emplace<CustomRigidBody>();
			customRigidBody.Set(strID, pVertexBuffer, pIndexBuffer, pMatWorld, rigidBodyProperty);

			if (pVertexBuffer != nullptr && pIndexBuffer != nullptr)
			{
				// 고치시오
				assert(false);

				//const math::float3* pVertexPos = reinterpret_cast<const math::float3*>(pVertexBuffer->GetVertexPosPtr());
				//uint32_t nVertexCount = pVertexBuffer->GetVertexNum();
				//
				//const uint32_t* pIndices = static_cast<const uint32_t*>(pIndexBuffer->GetRawValuePtr());
				//uint32_t nIndexCount = pIndexBuffer->GetIndexNum();
				//
				//customRigidBody.rigidBodyProperty.shapeInfo.SetTriangleMesh(pVertexPos, nVertexCount, pIndices, nIndexCount);
			}

			m_isCollisionModelVisible = isCollisionModelVisible;
		}

		void ComponentPhysics::Update(float elapsedTime)
		{
			if (m_isInit == false)
			{
				switch (m_emRigidBodyType)
				{
				case RigidBodyType::eBasic:
				{
					BasicRigidBody& basicRigidBody = std::get<BasicRigidBody>(m_rigidBodyElements);
					initPhysics(basicRigidBody.rigidBodyProperty.strName, basicRigidBody.rigidBodyProperty, nullptr, nullptr);

					m_isInit = true;
				}
				break;
				case RigidBodyType::eModel:
				{
					ModelRigidBody& modelRigidBody = std::get<ModelRigidBody>(m_rigidBodyElements);

					if (modelRigidBody.pModelInstance != nullptr && modelRigidBody.pModelInstance->IsLoadComplete())
					{
						graphics::IModel* pModel = modelRigidBody.pModelInstance->GetModel();
						const size_t nNodeCount = pModel->GetNodeCount();
						m_umapPhysicsNode.reserve(nNodeCount);

						for (uint32_t i = 0; i < nNodeCount; ++i)
						{
							// 고치시오
							assert(false);

							//graphics::IModelNode* pModelNode = pModel->GetNode(i);
							//
							//modelRigidBody.rigidBodyProperty.shapeInfo.SetTriangleMesh(
							//	reinterpret_cast<const math::float3*>(pModelNode->GetVertexBuffer()->GetVertexPosPtr()), pModelNode->GetVertexBuffer()->GetVertexNum(),
							//	pModelNode->GetIndexBuffer()->GetRawValuePtr(), pModelNode->GetIndexBuffer()->GetIndexNum());
							//
							//string::StringID strName;
							//strName.Format("%s_%s", modelRigidBody.rigidBodyProperty.strName.c_str(), pModelNode->GetName().c_str());
							//
							//initPhysics(strName, modelRigidBody.rigidBodyProperty, nullptr, nullptr);
						}

						m_isInit = true;
					}
				}
				break;
				case RigidBodyType::eCustom:
				{
					CustomRigidBody& customRigidBody = std::get<CustomRigidBody>(m_rigidBodyElements);

					std::string strName = customRigidBody.rigidBodyProperty.strName.c_str();
					strName.append("_");
					strName.append(customRigidBody.strID.c_str());

					initPhysics(strName.c_str(), customRigidBody.rigidBodyProperty, customRigidBody.pMatWorld, nullptr);

					m_isInit = true;
				}
				break;
				}
			}

			if (m_pRagDoll != nullptr)
			{
				m_pRagDoll->Update(elapsedTime);
			}

			for (auto iter = m_umapPhysicsNode.begin(); iter != m_umapPhysicsNode.end(); ++iter)
			{
				if (iter->second.pRigidBody == nullptr)
					continue;

				iter->second.pRigidBody->Update(elapsedTime);

				math::Matrix matWorld = iter->second.pRigidBody->GetWorldMatrix();

				math::float3 f3Pos, f3Scale;
				math::Quaternion quat;
				matWorld.Decompose(f3Scale, quat, f3Pos);

				if (m_pOwner != nullptr)
				{
					m_pOwner->SetPosition(f3Pos);
					m_pOwner->SetRotation(quat);
					m_pOwner->CalcWorldMatrix();

					math::float3 f3Velocity = m_pOwner->GetVelocity();
					iter->second.pRigidBody->SetLinearVelocity(f3Velocity);
				}

				if (iter->second.pPhysicsModelInstance != nullptr)
				{
					iter->second.pPhysicsModelInstance->SetVisible(m_isCollisionModelVisible);
					iter->second.pPhysicsModelInstance->Update(elapsedTime, m_pOwner->GetWorldMatrix());
				}
			}
		}

		void ComponentPhysics::SetActiveState(physics::EmActiveState::Type emActiveState)
		{
			for (auto& iter : m_umapPhysicsNode)
			{
				iter.second.pRigidBody->SetActiveState(emActiveState);
			}
		}

		void ComponentPhysics::initPhysics(const string::StringID& strID, const physics::RigidBodyProperty& rigidBodyProperty, const math::Matrix* pMatWorld, graphics::IModelInstance* pModelInstance)
		{
			graphics::MaterialInfo materialInfo;
			materialInfo.name = strID;
			materialInfo.colorAlbedo = math::Color::Red;
			materialInfo.emRasterizerState = graphics::EmRasterizerState::eWireframeCullNone;

			physics::RigidBody* pRigidBody = nullptr;
			graphics::IModelInstance* pPhysicsModelInst = nullptr;

			switch (rigidBodyProperty.shapeInfo.emPhysicsShapeType)
			{
			case physics::EmPhysicsShape::eBox:
			{
				pRigidBody = physics::RigidBody::Create(rigidBodyProperty);

				const physics::Shape::Box* pBox = std::get_if<physics::Shape::Box>(&rigidBodyProperty.shapeInfo.element);
				if (pBox == nullptr)
					return;

				graphics::ModelLoader modelLoader;
				modelLoader.InitBox(strID, &materialInfo, pBox->f3Size);
				pPhysicsModelInst = graphics::IModel::CreateInstance(modelLoader);
			}
			break;
			case physics::EmPhysicsShape::eSphere:
			{
				pRigidBody = physics::RigidBody::Create(rigidBodyProperty);

				const physics::Shape::Sphere* pSphere = std::get_if<physics::Shape::Sphere>(&rigidBodyProperty.shapeInfo.element);
				if (pSphere == nullptr)
					return;

				graphics::ModelLoader modelLoader;
				modelLoader.InitSphere(strID, &materialInfo, pSphere->fRadius);
				pPhysicsModelInst = graphics::IModel::CreateInstance(modelLoader);
			}
			break;
			case physics::EmPhysicsShape::eCylinder:
				break;
			case physics::EmPhysicsShape::eCylinder_X:
				break;
			case physics::EmPhysicsShape::eCylinder_Z:
				break;
			case physics::EmPhysicsShape::eCapsule:
			{
				pRigidBody = physics::RigidBody::Create(rigidBodyProperty);

				const physics::Shape::Capsule* pCapsule = std::get_if<physics::Shape::Capsule>(&rigidBodyProperty.shapeInfo.element);
				if (pCapsule == nullptr)
					return;

				graphics::ModelLoader modelLoader;
				modelLoader.InitCapsule(strID, &materialInfo, pCapsule->fRadius, pCapsule->fHeight);
				pPhysicsModelInst = graphics::IModel::CreateInstance(modelLoader);
			}
			break;
			case physics::EmPhysicsShape::eCapsule_X:
				break;
			case physics::EmPhysicsShape::eCapsule_Z:
				break;
			case physics::EmPhysicsShape::eCone:
			{
				pRigidBody = physics::RigidBody::Create(rigidBodyProperty);

				const physics::Shape::Cone* pCone = std::get_if<physics::Shape::Cone>(&rigidBodyProperty.shapeInfo.element);
				if (pCone == nullptr)
					return;

				graphics::ModelLoader modelLoader;
				modelLoader.InitCone(strID, &materialInfo, pCone->fRadius, pCone->fHeight);
				pPhysicsModelInst = graphics::IModel::CreateInstance(modelLoader);
			}
			break;
			case physics::EmPhysicsShape::eCone_X:
				break;
			case physics::EmPhysicsShape::eCone_Z:
				break;
			case physics::EmPhysicsShape::eHull:
			{
				pRigidBody = physics::RigidBody::Create(rigidBodyProperty);
			}
			break;
			case physics::EmPhysicsShape::eTriangleMesh:
			{
				pRigidBody = physics::RigidBody::Create(rigidBodyProperty);
			}
			break;
			}

			if (pRigidBody == nullptr)
				return;

			m_umapPhysicsNode.emplace(strID, PhysicsNode(pMatWorld, pRigidBody, pPhysicsModelInst, pModelInstance));
		}
	}
}