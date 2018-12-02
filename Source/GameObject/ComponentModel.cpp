#include "stdafx.h"
#include "ComponentModel.h"

#include "CommonLib/FileStream.h"
#include "CommonLib/FileUtil.h"

#include "Model/MotionSystem.h"
#include "Model/Model.h"
#include "Model/ModelLoader.h"

#include "GameObject.h"

namespace eastengine
{
	namespace gameobject
	{
		ComponentModel::ComponentModel(IActor* pOwner)
			: IComponent(pOwner, IComponent::eModel)
		{
		}

		ComponentModel::~ComponentModel()
		{
			graphics::IModel::DestroyInstance(&m_pModelInst);
		}

		void ComponentModel::Initialize(graphics::IModelInstance* pModelInst)
		{
			m_pModelInst = pModelInst;
		}

		void ComponentModel::Initialize(const graphics::ModelLoader* pLoader)
		{
			if (pLoader != nullptr)
			{
				m_pModelInst = graphics::IModel::CreateInstance(*pLoader, pLoader->IsEnableThreadLoad());
			}
		}

		void ComponentModel::Update(float elapsedTime)
		{
			if (m_pModelInst != nullptr)
			{
				m_pModelInst->Update(elapsedTime, m_pOwner->GetWorldMatrix());
			}

			for (auto iter : m_umapChild)
			{
				iter.second->Update(elapsedTime);
			}
		}

		bool ComponentModel::LoadFile(file::Stream& file)
		{
			std::string strBuf;
			file >> strBuf;

			std::string strFullPath(file::GetFilePath(file.GetFilePath()));
			strFullPath.append(strBuf);
			strFullPath.append(".emod");

			graphics::ModelLoader loader;
			loader.InitEast(strBuf.c_str(), strFullPath.c_str());
			//loader.SetEnableThreadLoad(true);

			Initialize(&loader);

			return true;
		}

		bool ComponentModel::SaveFile(file::Stream& file)
		{
			graphics::IModel* pModel = m_pModelInst->GetModel();

			file << pModel->GetName().c_str();

			std::string strFullPath(file::GetFilePath(file.GetFilePath()));
			strFullPath.append(pModel->GetName().c_str());
			strFullPath.append(".emod");

			graphics::IModel::SaveFile(pModel, strFullPath.c_str());

			return true;
		}

		graphics::IModel* ComponentModel::GetModel()
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

		bool ComponentModel::PlayMotion(graphics::MotionLayers emLayer, graphics::IMotion* pMotion, const graphics::MotionPlaybackInfo* pPlayback)
		{
			if (m_pModelInst == nullptr || m_pModelInst->GetMotionSystem() == nullptr)
				return false;

			m_pModelInst->GetMotionSystem()->Play(emLayer, pMotion, pPlayback);

			return true;
		}

		bool ComponentModel::PlayMotion(graphics::MotionLayers emLayer, const graphics::MotionLoader& loader, const graphics::MotionPlaybackInfo* pPlayback)
		{
			graphics::IMotion* pMotion = graphics::IMotion::Create(loader);
			if (pMotion == nullptr)
				return false;

			return PlayMotion(emLayer, pMotion, pPlayback);
		}

		void ComponentModel::StopMotion(graphics::MotionLayers emLayer, float fStopTime)
		{
			if (m_pModelInst == nullptr || m_pModelInst->GetMotionSystem() == nullptr)
				return;

			m_pModelInst->GetMotionSystem()->Stop(emLayer, fStopTime);
		}
	}
}