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
			//, m_pRagDoll(nullptr)
		{
		}

		ComponentPhysics::~ComponentPhysics()
		{
			//SafeDelete(m_pRagDoll);

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

		void ComponentPhysics::Init(String::StringID strID, const Graphics::IVertexBuffer* pVertexBuffer, const Graphics::IIndexBuffer* pIndexBuffer, Math::Matrix* pMatWorld, const Physics::RigidBodyProperty& rigidBodyProperty, bool isCollisionModelVisible)
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
					initPhysics(nullptr, m_loader.byBasic.rigidBodyProperty.strName, nullptr, nullptr, nullptr, m_loader.byBasic.rigidBodyProperty);

					m_isInit = true;
				}
				break;
				case Type::eModel:
				{
					if (m_loader.byModel.pModelInst != nullptr && m_loader.byModel.pModelInst->IsLoadComplete())
					{
						Graphics::IModel* pModel = m_loader.byModel.pModelInst->GetModel();
						uint32_t nNodeCount = pModel->GetNodeCount();
						m_umapPhysicsNode.reserve(nNodeCount);

						for (uint32_t i = 0; i < nNodeCount; ++i)
						{
							Graphics::IModelNode* pModelNode = pModel->GetNode(i);
							initPhysics(pModelNode, m_loader.byModel.rigidBodyProperty, m_loader.byModel.nTargetLod);
						}

						m_isInit = true;
					}
				}
				break;
				case Type::eCustom:
				{
					std::string strName = m_loader.byCustom.rigidBodyProperty.strName.c_str();
					strName.append("_");
					strName.append(m_loader.byCustom.strID.c_str());

					initPhysics(nullptr, strName.c_str(), m_loader.byCustom.pVertexBuffer, m_loader.byCustom.pIndexBuffer,
						m_loader.byCustom.pMatWorld, m_loader.byCustom.rigidBodyProperty);

					m_isInit = true;
				}
				break;
				}
			}

			Graphics::Camera* pCamera = Graphics::CameraManager::GetInstance()->GetMainCamera();
			const Math::Vector3& f3CameraPos = pCamera->GetPosition();

			//if (m_pRagDoll != nullptr)
			//{
			//	m_pRagDoll->Update(fElapsedTime);
			//}

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
				
				if (iter->second.pModelNode != nullptr)
				{
					Collision::AABB aabb = iter->second.pRigidBody->GetAABB();
					aabb.Transform(aabb, matWorld.Invert());

					iter->second.pModelNode->BuildBoundingBox(aabb);

					float fDist = Math::Vector3::DistanceSquared(iter->second.pRigidBody->GetAABB().Center, f3CameraPos);
					iter->second.pModelNode->SetDistanceFromCamera(fDist);

					//Math::Vector3 f3Dir = iter->second.pRigidBody->GetAABB().Center - f3CameraPos;
					//f3Dir.Normalize();
					//
					//float fDist = 0.f;
					//iter->second.pRigidBody->GetAABB().Intersects(f3CameraPos, f3Dir, fDist);
					//iter->second.pModelNode->SetDistanceFromCamera(fDist);

					//const Math::Vector3& f3Pos = pCamera->GetPosition();
					//
					//Math::Vector3 f3Dir = iter->second.pRigidBody->GetAABB().Center - f3Pos;
					//f3Dir.Normalize();
					//
					//float fDist = 0.f;
					//iter->second.pRigidBody->GetAABB().Intersects(f3Pos, f3Dir, fDist);
					//
					//PRINT_LOG("%f", fDist);
					//
					//EmLOD emLod = EmLOD::Lv0;
					//if (fDist < 40.f)
					//{
					//	emLod = EmLOD::Lv0;
					//}
					//else if (fDist < 60.f)
					//{
					//	emLod = EmLOD::Lv1;
					//}
					//else if (fDist < 80.f)
					//{
					//	emLod = EmLOD::Lv2;
					//}
					//else if (fDist < 100.f)
					//{
					//	emLod = EmLOD::Lv3;
					//}
					//else
					//{
					//	emLod = EmLOD::Lv4;
					//}
					//
					//iter->second.pModelNode->SetLOD(emLod);
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

		void ComponentPhysics::initPhysics(Graphics::IModelNode* pModelNode, Physics::RigidBodyProperty& rigidBodyProperty, uint32_t nTargetLod)
		{
			using namespace Physics;

			std::string strName = rigidBodyProperty.strName.c_str();
			strName.append("_");
			strName.append(pModelNode->GetName().c_str());

			initPhysics(pModelNode, strName.c_str(), pModelNode->GetVertexBuffer(nTargetLod), pModelNode->GetIndexBuffer(nTargetLod), pModelNode->GetWorldMatrixPtr(), rigidBodyProperty);
		}

		void ComponentPhysics::initPhysics(Graphics::IModelNode* pModelNode, const String::StringID& strID, const Graphics::IVertexBuffer* pVertexBuffer, const Graphics::IIndexBuffer* pIndexBuffer, const Math::Matrix* pMatWorld, Physics::RigidBodyProperty& rigidBodyProperty)
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
				if (pModelNode != nullptr)
				{
					const Math::Vector3* pVertexPos = reinterpret_cast<const Math::Vector3*>(pModelNode->GetVertexBuffer()->GetVertexPosPtr());
					uint32_t nVertexCount = pModelNode->GetVertexBuffer()->GetVertexNum();

					const uint32_t* pIndices = static_cast<const uint32_t*>(pModelNode->GetIndexBuffer()->GetRawValuePtr());
					uint32_t nIndexCount = pModelNode->GetIndexBuffer()->GetIndexNum();

					rigidBodyProperty.shapeInfo.SetHull(pVertexPos, nVertexCount, pIndices, nIndexCount);

					pRigidBody = Physics::RigidBody::Create(rigidBodyProperty);
				}
			}
			break;
			case Physics::EmPhysicsShape::eTriangleMesh:
			{
				if (pModelNode != nullptr)
				{
					const Math::Vector3* pVertexPos = reinterpret_cast<const Math::Vector3*>(pModelNode->GetVertexBuffer()->GetVertexPosPtr());
					uint32_t nVertexCount = pModelNode->GetVertexBuffer()->GetVertexNum();

					const uint32_t* pIndices = static_cast<const uint32_t*>(pModelNode->GetIndexBuffer()->GetRawValuePtr());
					uint32_t nIndexCount = pModelNode->GetIndexBuffer()->GetIndexNum();

					rigidBodyProperty.shapeInfo.SetTriangleMesh(pVertexPos, nVertexCount, pIndices, nIndexCount);

					pRigidBody = Physics::RigidBody::Create(rigidBodyProperty);
				}
			}
			break;
			}

			if (pRigidBody == nullptr)
				return;

			m_umapPhysicsNode.emplace(strID, PhysicsNode(pMatWorld, pRigidBody, pModelNode, pPhysicsModelInst));
		}
	}
}