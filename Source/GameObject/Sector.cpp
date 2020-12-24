#include "stdafx.h"
#include "SectorMgr.h"
#include "Sector.h"

#include "GameObject.h"
#include "ComponentModel.h"
#include "ComponentPhysics.h"

#include "Graphics/Model/ModelLoader.h"

namespace sid
{
	RegisterStringID(Sector);
	RegisterStringID(Sector_RigidBody);
}

namespace est
{
	namespace gameobject
	{
		Sector::Sector(SectorMgr* pSectorMgr, float fRadius, int nCoordinateX, int nCoordinateY)
			: m_pSectorMgr(pSectorMgr)
			, m_fRadius(fRadius)
			, m_n2Coordinate(nCoordinateX, nCoordinateY)
			, m_isVisibleTile(true)
		{
			const math::float3 position((float)(nCoordinateX)* fRadius * 1.5f * 1.05f, 1.f, ((float)(nCoordinateY)+(float)(nCoordinateX) * -0.5f) * fRadius * 2.f * 1.05f);

			const string::StringID name(string::Format(L"%s_%d/%d", sid::Sector.c_str(), nCoordinateX, nCoordinateY));

			m_pActor = CreateActor(name);
			m_pActor->SetPosition(position);

			ComponentModel* pCompModel = static_cast<ComponentModel*>(m_pActor->CreateComponent(IComponent::eModel));

			if (pCompModel != nullptr)
			{
				graphics::IMaterial::Data materialData;
				materialData.name = sid::Sector;
				//materialData.rasterizerStateKey = graphics::Device::GetInstance()->GetRasterizerStateKey(graphics::RasterizerState::eWireFrame);

				graphics::ModelLoader loader;
				loader.InitHexagon(sid::Sector, &materialData, fRadius);

				graphics::ModelInstancePtr pModelInst = graphics::CreateModelInstance(loader);
				pCompModel->Add(0, std::move(pModelInst));

				ComponentPhysics* pCompPhysics = static_cast<ComponentPhysics*>(m_pActor->CreateComponent(IComponent::ePhysics));
				if (pCompPhysics != nullptr)
				{
					graphics::IModelNode* pModelNode = pModelInst->GetModel()->GetNode(0);

					const graphics::VertexPos* pVertices = nullptr;
					size_t numVertices = 0;
					pModelNode->GetRawVertices(&pVertices, numVertices);

					const uint32_t* pIndices = nullptr;
					size_t numIndices = 0;
					pModelNode->GetRawIndices(&pIndices, numIndices);

					physics::RigidActorProperty rigidActorProperty;
					rigidActorProperty.material.restitution = 0.75f;
					rigidActorProperty.shape.SetTriangleMeshGeometry(math::float3::One, math::Quaternion::Identity, reinterpret_cast<const math::float3*>(pVertices), static_cast<uint32_t>(numVertices), pIndices, static_cast<uint32_t>(numIndices), physics::IGeometry::MeshFlag::eNone);

					rigidActorProperty.rigidAcotr.name = sid::Sector_RigidBody;
					rigidActorProperty.rigidAcotr.type = physics::IActor::eRigidStatic;
					rigidActorProperty.rigidAcotr.globalTransform.position = position;

					pCompPhysics->CreateRigidActor(rigidActorProperty);
				}
			}

			m_nearSector.resize(EmSector::eCount);
		}

		Sector::~Sector()
		{
			m_pActor.reset();
			m_umapActor.clear();

			m_nearSector.clear();
		}

		void Sector::Enter(IActor* pActor)
		{
			m_umapActor.emplace(pActor->GetHandle(), pActor);
		}

		void Sector::Leave(IActor* pActor)
		{
			auto iter = m_umapActor.find(pActor->GetHandle());
			if (iter == m_umapActor.end())
				return;

			m_umapActor.erase(iter);
		}

		void Sector::Update(float elapsedTime)
		{
			if (m_isVisibleTile)
			{
				m_pActor->Update(elapsedTime);
			}

			for (auto& iter : m_umapActor)
			{
				iter.second->Update(elapsedTime);
			}
		}

		bool Sector::MoveToNearSector(EmSector::Dir emSectorDir, IActor* pActor)
		{
			if (EmSector::eCount >= emSectorDir)
				return false;

			Sector* pTarget = m_nearSector[emSectorDir];
			if (pTarget == nullptr)
				return false;

			EnterLeaveSectorInfo info;
			info.pEnterSector = pTarget;
			info.pLeaveSector = this;
			info.pActor = pActor;

			m_pSectorMgr->EnterLeaveSector(info);

			return true;
		}
	}
}