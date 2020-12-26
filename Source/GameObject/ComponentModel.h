#pragma once

#include "Graphics/Model/ModelInterface.h"
#include "ComponentInterface.h"

namespace est
{
	namespace gameobject
	{
		class ComponentModel : public IComponent
		{
		public:
			ComponentModel(IActor* pOwner);
			virtual ~ComponentModel();

		public:
			virtual void Update(float elapsedTime, float lodThreshold) override;

		public:
			void Add(uint32_t id, graphics::ModelInstancePtr pModelInstnace);
			void Add(uint32_t id, const graphics::ModelLoader& loader);

			void Remove(uint32_t id);

		public:
			graphics::IModelInstance* GetModelInstance(uint32_t id) const;
			bool IsLoadComplete(uint32_t id) const;

		public:
			bool PlayMotion(uint32_t id, graphics::MotionLayers emLayer, const graphics::MotionPtr& pMotion, const graphics::MotionPlaybackInfo* pMotionState = nullptr);
			bool PlayMotion(uint32_t id, graphics::MotionLayers emLayer, const graphics::MotionLoader& loader, const graphics::MotionPlaybackInfo* pMotionState = nullptr);
			void StopMotion(uint32_t id, graphics::MotionLayers emLayer, float fStopTime);

		private:
			struct ModelData
			{
				uint32_t id{ 0 };
				graphics::ModelInstancePtr pModelInstnace{ nullptr };

				ModelData(uint32_t id, graphics::ModelInstancePtr pModelInstnace)
					: id(id)
					, pModelInstnace(std::move(pModelInstnace))
				{
				}
			};
			std::vector<ModelData> m_models;

			float m_lodUpdateTime{ 0.f };
		};
	}
}