#include "stdafx.h"
#include "ComponentPhysics.h"

#include "DirectX/CameraManager.h"

#include "Model/ModelInterface.h"
#include "Model/ModelLoader.h"

#include "RagDoll.h"

#include "GameObject.h"

namespace EastEngine
{
	namespace GameObject
	{
		ComponentPhysics::ComponentPhysics(IActor* pOwner)
			: IComponent(pOwner, EmComponent::ePhysics)
			, m_isInit(false)
			, m_isCollisionModelVisible(false)
			, m_pRagDoll(new RagDoll)
		{
		}

		ComponentPhysics::~ComponentPhysics()
		{
			SafeDelete(m_pRagDoll);

			if (m_emType == Type::eModel)
			{
				m_loader.byModel.pModelInst = nullptr;
			}

			std::for_each(m_umapPhysicsNode.begin(), m_umapPhysicsNode.end(), [](std::pair<String::StringID, PhysicsNode> key)
			{
				SafeDelete(key.second.pRigidBody);
				Graphics::IModel::DestroyInstance(&key.second.pPhysicsModelInst);
			});
			m_umapPhysicsNode.clear();
		}

		void ComponentPhysics::Init(const Physics::RigidBodyProperty& rigidBodyProperty, bool isCollisionModelVisible)
		{
			m_loader.byBasic.Set(rigidBodyProperty);
			m_emType = Type::eBasic;
			m_isCollisionModelVisible = isCollisionModelVisible;
		}

		void ComponentPhysics::Init(Graphics::IModelInstance* pModelInst, const Physics::RigidBodyProperty& rigidBodyProperty, uint32_t nTargetLod, bool isCollisionModelVisible)
		{
			m_loader.byModel.Set(pModelInst, rigidBodyProperty, nTargetLod);
			m_emType = Type::eModel;
			m_isCollisionModelVisible = isCollisionModelVisible;
		}

		void ComponentPhysics::Init(const String::StringID& strID, const Graphics::IVertexBuffer* pVertexBuffer, const Graphics::IIndexBuffer* pIndexBuffer, Math::Matrix* pMatWorld, const Physics::RigidBodyProperty& rigidBodyProperty, bool isCollisionModelVisible)
		{
			m_loader.byCustom.Set(strID, pVertexBuffer, pIndexBuffer, pMatWorld, rigidBodyProperty);
			m_emType = Type::eCustom;
			m_isCollisionModelVisible = isCollisionModelVisible;
		}

		void ComponentPhysics::Update(float fElapsedTime)
		{
			if (m_isInit == false)
			{
				switch (m_emType)
				{
				case Type::eBasic:
				{
					m_isInit = true;
				}
				break;
				case Type::eModel:
				{
				}
				break;
				case Type::eCustom:
				{
					std::string strName = m_loader.byCustom.rigidBodyProperty.strName.c_str();
					strName.append("_");
					strName.append(m_loader.byCustom.strID.c_str());

					m_isInit = true;
				}
				break;
				}
			}

			Graphics::Camera* pCamera = Graphics::CameraManager::GetInstance()->GetMainCamera();
			const Math::Vector3& f3CameraPos = pCamera->GetPosition();

			if (m_pRagDoll != nullptr)
			{
				m_pRagDoll->Update(fElapsedTime);
			}

			for (auto iter = m_umapPhysicsNode.begin(); iter != m_umapPhysicsNode.end(); ++iter)
			{
				if (iter->second.pRigidBody == nullptr)
					continue;

				iter->second.pRigidBody->Update(fElapsedTime);

				Math::Matrix matWorld = iter->second.pRigidBody->GetWorldMatrix();

				Math::Vector3 f3Pos, f3Scale;
				Math::Quaternion quat;
				matWorld.Decompose(f3Scale, quat, f3Pos);

				if (m_pOwner != nullptr)
				{
					m_pOwner->SetPosition(f3Pos);
					m_pOwner->SetRotation(quat);
					m_pOwner->CalcWorldMatrix();

					Math::Vector3 f3Velocity = m_pOwner->GetVelocity();
					iter->second.pRigidBody->SetLinearVelocity(f3Velocity);
				}
				
				if (iter->second.pPhysicsModelInst != nullptr)
				{
					iter->second.pPhysicsModelInst->SetVisible(m_isCollisionModelVisible);
					iter->second.pPhysicsModelInst->Update(fElapsedTime, m_pOwner->GetWorldMatrix());
				}
			}
		}

		void ComponentPhysics::SetActiveState(Physics::EmActiveState::Type emActiveState)
		{
			for (auto& iter : m_umapPhysicsNode)
			{
				iter.second.pRigidBody->SetActiveState(emActiveState);
			}
		}

		void ComponentPhysics::initPhysics(const String::StringID& strID, const Graphics::IVertexBuffer* pVertexBuffer, const Graphics::IIndexBuffer* pIndexBuffer, const Math::Matrix* pMatWorld, Physics::RigidBodyProperty& rigidBodyProperty)
		{
			Graphics::MaterialInfo materialInfo;
			materialInfo.strName = strID;
			materialInfo.colorAlbedo = Math::Color::Red;
			materialInfo.pRasterizerState = Graphics::GetDevice()->GetRasterizerState(Graphics::EmRasterizerState::eWireframeCullNone);
			
			Physics::RigidBody* pRigidBody = nullptr;
			Graphics::IModelInstance* pPhysicsModelInst = nullptr;

			switch (rigidBodyProperty.shapeInfo.emPhysicsShapeType)
			{
			case Physics::EmPhysicsShape::eBox:
			{
				pRigidBody = Physics::RigidBody::Create(rigidBodyProperty);

				const Physics::Shape::Box* pBox = std::get_if<Physics::Shape::Box>(&rigidBodyProperty.shapeInfo.element);
				if (pBox == nullptr)
					return;

				Graphics::ModelLoader modelLoader;
				modelLoader.InitBox(strID, &materialInfo, pBox->f3Size);
				pPhysicsModelInst = Graphics::IModel::CreateInstance(modelLoader);
			}
				break;
			case Physics::EmPhysicsShape::eSphere:
			{
				pRigidBody = Physics::RigidBody::Create(rigidBodyProperty);

				const Physics::Shape::Sphere* pSphere = std::get_if<Physics::Shape::Sphere>(&rigidBodyProperty.shapeInfo.element);
				if (pSphere == nullptr)
					return;

				Graphics::ModelLoader modelLoader;
				modelLoader.InitSphere(strID, &materialInfo, pSphere->fRadius);
				pPhysicsModelInst = Graphics::IModel::CreateInstance(modelLoader);
			}
				break;
			case Physics::EmPhysicsShape::eCylinder:
				break;
			case Physics::EmPhysicsShape::eCylinder_X:
				break;
			case Physics::EmPhysicsShape::eCylinder_Z:
				break;
			case Physics::EmPhysicsShape::eCapsule:
			{
				pRigidBody = Physics::RigidBody::Create(rigidBodyProperty);

				const Physics::Shape::Capsule* pCapsule = std::get_if<Physics::Shape::Capsule>(&rigidBodyProperty.shapeInfo.element);
				if (pCapsule == nullptr)
					return;

				Graphics::ModelLoader modelLoader;
				modelLoader.InitCapsule(strID, &materialInfo, pCapsule->fRadius, pCapsule->fHeight);
				pPhysicsModelInst = Graphics::IModel::CreateInstance(modelLoader);
			}
				break;
			case Physics::EmPhysicsShape::eCapsule_X:
				break;
			case Physics::EmPhysicsShape::eCapsule_Z:
				break;
			case Physics::EmPhysicsShape::eCone:
			{
				pRigidBody = Physics::RigidBody::Create(rigidBodyProperty);

				const Physics::Shape::Cone* pCone = std::get_if<Physics::Shape::Cone>(&rigidBodyProperty.shapeInfo.element);
				if (pCone == nullptr)
					return;

				Graphics::ModelLoader modelLoader;
				modelLoader.InitCone(strID, &materialInfo, pCone->fRadius, pCone->fHeight);
				pPhysicsModelInst = Graphics::IModel::CreateInstance(modelLoader);
			}
				break;
			case Physics::EmPhysicsShape::eCone_X:
				break;
			case Physics::EmPhysicsShape::eCone_Z:
				break;
			case Physics::EmPhysicsShape::eHull:
			{
				if (pVertexBuffer != nullptr && pIndexBuffer != nullptr)
				{
					const Math::Vector3* pVertexPos = reinterpret_cast<const Math::Vector3*>(pVertexBuffer->GetVertexPosPtr());
					uint32_t nVertexCount = pVertexBuffer->GetVertexNum();

					const uint32_t* pIndices = static_cast<const uint32_t*>(pIndexBuffer->GetRawValuePtr());
					uint32_t nIndexCount = pIndexBuffer->GetIndexNum();

					rigidBodyProperty.shapeInfo.SetHull(pVertexPos, nVertexCount, pIndices, nIndexCount);

					pRigidBody = Physics::RigidBody::Create(rigidBodyProperty);
				}
			}
			break;
			case Physics::EmPhysicsShape::eTriangleMesh:
			{
				if (pVertexBuffer != nullptr && pIndexBuffer != nullptr)
				{
					const Math::Vector3* pVertexPos = reinterpret_cast<const Math::Vector3*>(pVertexBuffer->GetVertexPosPtr());
					uint32_t nVertexCount = pVertexBuffer->GetVertexNum();

					const uint32_t* pIndices = static_cast<const uint32_t*>(pIndexBuffer->GetRawValuePtr());
					uint32_t nIndexCount = pIndexBuffer->GetIndexNum();

					rigidBodyProperty.shapeInfo.SetTriangleMesh(pVertexPos, nVertexCount, pIndices, nIndexCount);

					pRigidBody = Physics::RigidBody::Create(rigidBodyProperty);
				}
			}
			break;
			}

			if (pRigidBody == nullptr)
				return;

			m_umapPhysicsNode.emplace(strID, PhysicsNode(pMatWorld, pRigidBody, pPhysicsModelInst));
		}
	}
}