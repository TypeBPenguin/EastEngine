#include "stdafx.h"
#include "SectorMgr.h"
#include "Sector.h"

#include "ActorInterface.h"
#include "ComponentModel.h"
#include "ComponentPhysics.h"

#include "../DirectX/Device.h"

#include "../Model/ModelManager.h"

namespace StrID
{
	RegisterStringID(Sector);
	RegisterStringID(Sector_RigidBody);
}

namespace EastEngine
{
	namespace GameObject
	{
		Sector::Sector(SectorMgr* pSectorMgr, float fRadius, int nCoordinateX, int nCoordinateY)
			: m_pSectorMgr(pSectorMgr)
			, m_fRadius(fRadius)
			, m_n2Coordinate(nCoordinateX, nCoordinateY)
			, m_isVisibleTile(true)
		{
			Math::Vector3 f3Pos((float)(nCoordinateX)* fRadius * 1.5f * 1.05f, 1.f, ((float)(nCoordinateY)+(float)(nCoordinateX) * -0.5f) * fRadius * 2.f * 1.05f);

			String::StringID strName;
			strName.Format("%s_%d/%d", StrID::Sector.c_str(), nCoordinateX, nCoordinateY);

			m_pActor = IActor::Create(strName);
			m_pActor->SetPosition(f3Pos);

			ComponentModel* pCompModel = static_cast<ComponentModel*>(m_pActor->CreateComponent(EmComponent::eModel));

			if (pCompModel != nullptr)
			{
				Graphics::MaterialInfo materialInfo;
				materialInfo.strName = StrID::Sector;
				//materialInfo.rasterizerStateKey = Graphics::Device::GetInstance()->GetRasterizerStateKey(Graphics::EmRasterizerState::eWireFrame);

				Graphics::ModelLoader loader;
				loader.InitHexagon(StrID::Sector, &materialInfo, fRadius);

				auto pModelInst = Graphics::IModel::CreateInstance(loader);
				pCompModel->Init(pModelInst);

				ComponentPhysics* pCompPhysics = static_cast<ComponentPhysics*>(m_pActor->CreateComponent(EmComponent::ePhysics));

				if (pCompPhysics != nullptr)
				{
					Physics::RigidBodyProperty prop;
					prop.fRestitution = 0.75f;
					prop.strName = StrID::Sector_RigidBody;
					prop.fMass = 0.f;
					prop.nCollisionFlag = Physics::EmCollision::eStaticObject;
					prop.shapeInfo.SetTriangleMesh();
					prop.f3OriginPos = f3Pos;
					pCompPhysics->Init(pModelInst, prop);
				}
			}

			m_vecNearSector.resize(EmSector::eCount);
		}

		Sector::~Sector()
		{
			IActor::Destroy(&m_pActor);

			m_umapActor.clear();

			m_vecNearSector.clear();
		}

		void Sector::Enter(IActor* pActor)
		{
			m_umapActor.emplace(pActor->GetActorID(), pActor);
		}

		void Sector::Leave(IActor* pActor)
		{
			auto iter = m_umapActor.find(pActor->GetActorID());
			if (iter == m_umapActor.end())
				return;

			m_umapActor.erase(iter);
		}

		void Sector::Update(float fElapsedTime)
		{
			if (m_isVisibleTile)
			{
				m_pActor->Update(fElapsedTime);
			}

			for (auto& iter : m_umapActor)
			{
				iter.second->Update(fElapsedTime);
			}
		}

		bool Sector::MoveToNearSector(EmSector::Dir emSectorDir, IActor* pActor)
		{
			if (EmSector::eCount >= emSectorDir)
				return false;

			Sector* pTarget = m_vecNearSector[emSectorDir];
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