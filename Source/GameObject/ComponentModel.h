#pragma once

#include "Model/ModelInterface.h"
#include "Model/ModelLoader.h"
#include "Model/MotionLoader.h"
#include "ComponentInterface.h"

namespace eastengine
{
	namespace gameobject
	{
		class ComponentModel : public IComponent
		{
		public:
			ComponentModel(IActor* pOwner);
			virtual ~ComponentModel();

		public:
			void Initialize(graphics::IModelInstance* pModelInst);
			void Initialize(const graphics::ModelLoader* pLoader);

		public:
			virtual void Update(float elapsedTime) override;

			virtual bool LoadFile(file::Stream& file);
			virtual bool SaveFile(file::Stream& file);

		public:
			graphics::IModelInstance* GetModelInstance() { return m_pModelInst; }
			graphics::IModel* GetModel();
			bool IsLoadComplete();

		public:
			bool PlayMotion(graphics::MotionLayers emLayer, graphics::IMotion* pMotion, const graphics::MotionPlaybackInfo* pMotionState = nullptr);
			bool PlayMotion(graphics::MotionLayers emLayer, const graphics::MotionLoader& loader, const graphics::MotionPlaybackInfo* pMotionState = nullptr);
			void StopMotion(graphics::MotionLayers emLayer, float fStopTime);

		private:
			graphics::IModelInstance* m_pModelInst;
		};
	}
}