#include "stdafx.h"
#include "ComponentModel.h"

#include "../CommonLib/FileStream.h"

#include "../Model/MotionSystem.h"
#include "../Model/MotionManager.h"
#include "../Model/Model.h"

#include "ActorInterface.h"

namespace EastEngine
{
	namespace GameObject
	{
		ComponentModel::ComponentModel(IActor* pOwner)
			: IComponent(pOwner, EmComponent::eModel)
		{
		}

		ComponentModel::~ComponentModel()
		{
			Graphics::IModel::DestroyInstance(&m_pModelInst);
		}

		void ComponentModel::Init(Graphics::IModelInstance* pModelInst)
		{
			m_pModelInst = pModelInst;
		}

		void ComponentModel::Init(Graphics::ModelLoader* pLoader)
		{
			if (pLoader != nullptr)
			{
				m_pModelInst = Graphics::IModel::CreateInstance(*pLoader, pLoader->IsEnableThreadLoad());
			}
		}

		void ComponentModel::Update(float fElapsedTime)
		{
			if (m_pModelInst != nullptr)
			{
				m_pModelInst->Update(fElapsedTime, m_pOwner->GetWorldMatrix());
			}

			for (auto iter : m_umapChild)
			{
				iter.second->Update(fElapsedTime);
			}
		}

		bool ComponentModel::LoadToFile(File::FileStream& file)
		{
			std::string strBuf;
			file >> strBuf;

			std::string strFullPath(file.GetPath());
			strFullPath.append(strBuf);
			strFullPath.append(".emod");

			Graphics::ModelLoader loader;
			loader.InitEast(strBuf.c_str(), strFullPath.c_str());
			//loader.SetEnableThreadLoad(true);

			Init(&loader);

			return true;
		}

		bool ComponentModel::SaveToFile(File::FileStream& file)
		{
			Graphics::IModel* pModel = m_pModelInst->GetModel();

			file << pModel->GetName().c_str();

			std::string strFullPath(file.GetPath());
			strFullPath.append(pModel->GetName().c_str());
			strFullPath.append(".emod");

			Graphics::IModel::SaveToFile(pModel, strFullPath.c_str());

			return true;
		}

		Graphics::IModel* ComponentModel::GetModel()
		{
			if (m_pModelInst == nullptr)
				return nullptr;

			if (m_pModelInst->GetModel() != nullptr)
				return m_pModelInst->GetModel();

			return nullptr;
		}

		bool ComponentModel::IsLoadComplete()
		{
			if (m_pModelInst == nullptr)
				return false;

			return m_pModelInst->IsLoadComplete();
		}

		bool ComponentModel::PlayMotion(Graphics::IMotion* pMotion, const Graphics::MotionState* pMotionState)
		{
			m_pModelInst->PlayMotion(pMotion, pMotionState);

			return true;
		}

		bool ComponentModel::PlayMotion(const Graphics::MotionLoader& loader, const Graphics::MotionState* pMotionState)
		{
			Graphics::IMotion* pMotion = Graphics::IMotion::Create(loader);
			if (pMotion == nullptr)
				return false;

			m_pModelInst->PlayMotion(pMotion, pMotionState);

			return true;
		}

		void ComponentModel::StopMotion(float fStopTime)
		{
			m_pModelInst->StopMotion(fStopTime);
		}

		Graphics::IMotion* ComponentModel::GetMotioin()
		{
			return m_pModelInst->GetMotion();
		}
	}
}