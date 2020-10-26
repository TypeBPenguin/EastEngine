#include "stdafx.h"
#include "ComponentModel.h"

#include "GameObject.h"

namespace est
{
	namespace gameobject
	{
		ComponentModel::ComponentModel(IActor* pOwner)
			: IComponent(pOwner, IComponent::eModel)
		{
		}

		ComponentModel::~ComponentModel()
		{
			m_models.clear();
		}

		void ComponentModel::Update(float elapsedTime)
		{
			for (auto& modelData : m_models)
			{
				modelData.pModelInstnace->Update(elapsedTime, m_pOwner->GetWorldMatrix());
			}
		}

		void ComponentModel::Add(uint32_t id, graphics::ModelInstancePtr pModelInstnace)
		{
			Remove(id);

			m_models.emplace_back(id, std::move(pModelInstnace));
		}

		void ComponentModel::Add(uint32_t id, const graphics::ModelLoader& loader)
		{
			Add(id, graphics::CreateModelInstance(loader, loader.IsEnableThreadLoad()));
		}

		void ComponentModel::Remove(uint32_t id)
		{
			auto iter = std::find_if(m_models.begin(), m_models.end(), [id](const ModelData& modelData)
			{
				return modelData.id == id;
			});

			if (iter != m_models.end())
			{
				m_models.erase(iter);
			}
		}

		graphics::IModelInstance* ComponentModel::GetModelInstance(uint32_t id) const
		{
			auto iter = std::find_if(m_models.begin(), m_models.end(), [id](const ModelData& modelData)
			{
				return modelData.id == id;
			});

			if (iter != m_models.end())
				return iter->pModelInstnace.get();

			return nullptr;
		}

		bool ComponentModel::IsLoadComplete(uint32_t id) const
		{
			graphics::IModelInstance* ModelInstance = GetModelInstance(id);
			if (ModelInstance == nullptr)
				return false;

			return ModelInstance->IsLoadComplete();
		}

		bool ComponentModel::PlayMotion(uint32_t id, graphics::MotionLayers emLayer, const graphics::MotionPtr& pMotion, const graphics::MotionPlaybackInfo* pPlayback)
		{
			graphics::IModelInstance* ModelInstance = GetModelInstance(id);
			if (ModelInstance == nullptr)
				return false;

			if (ModelInstance->GetMotionSystem() == nullptr)
				return false;

			ModelInstance->GetMotionSystem()->Play(emLayer, pMotion, pPlayback);

			return true;
		}

		bool ComponentModel::PlayMotion(uint32_t id, graphics::MotionLayers emLayer, const graphics::MotionLoader& loader, const graphics::MotionPlaybackInfo* pPlayback)
		{
			graphics::IModelInstance* ModelInstance = GetModelInstance(id);
			if (ModelInstance == nullptr)
				return false;

			graphics::MotionPtr pMotion = graphics::CreateMotion(loader);
			if (pMotion == nullptr)
				return false;

			return PlayMotion(id, emLayer, pMotion, pPlayback);
		}

		void ComponentModel::StopMotion(uint32_t id, graphics::MotionLayers emLayer, float fStopTime)
		{
			graphics::IModelInstance* ModelInstance = GetModelInstance(id);
			if (ModelInstance == nullptr)
				return;

			if (ModelInstance->GetMotionSystem() == nullptr)
				return;

			ModelInstance->GetMotionSystem()->Stop(emLayer, fStopTime);
		}
	}
}