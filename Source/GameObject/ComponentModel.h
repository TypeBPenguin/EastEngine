#pragma once

#include "Model/ModelInterface.h"
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
			void Init(graphics::IModelInstance* pModelInst);
			void Init(const graphics::ModelLoader* pLoader);
			virtual void Update(float fElapsedTime) override;

			virtual bool LoadToFile(file::Stream& file);
			virtual bool SaveToFile(file::Stream& file);

		public:
			graphics::IModelInstance* GetModelInstance() { return m_pModelInst; }
			graphics::IModel* GetModel();
			bool IsLoadComplete();

		public:
			bool PlayMotion(graphics::EmMotion::Layers emLayer, graphics::IMotion* pMotion, const graphics::MotionPlaybackInfo* pMotionState = nullptr);
			bool PlayMotion(graphics::EmMotion::Layers emLayer, const graphics::MotionLoader& loader, const graphics::MotionPlaybackInfo* pMotionState = nullptr);
			void StopMotion(graphics::EmMotion::Layers emLayer, float fStopTime);

		private:
			graphics::IModelInstance* m_pModelInst;
		};
	}
}