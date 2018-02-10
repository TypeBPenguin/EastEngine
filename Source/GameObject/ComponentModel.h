#pragma once

#include "Model/ModelInterface.h"
#include "ComponentInterface.h"

namespace EastEngine
{
	namespace GameObject
	{
		class ComponentModel : public IComponent
		{
		public:
			ComponentModel(IActor* pOwner);
			virtual ~ComponentModel();

		public:
			void Init(Graphics::IModelInstance* pModelInst);
			void Init(Graphics::ModelLoader* pLoader);
			virtual void Update(float fElapsedTime) override;

			virtual bool LoadToFile(File::Stream& file);
			virtual bool SaveToFile(File::Stream& file);

		public:
			Graphics::IModelInstance* GetModelInstance() { return m_pModelInst; }
			Graphics::IModel* GetModel();
			bool IsLoadComplete();

		public:
			bool PlayMotion(Graphics::EmMotion::Layers emLayer, Graphics::IMotion* pMotion, const Graphics::MotionPlaybackInfo* pMotionState = nullptr);
			bool PlayMotion(Graphics::EmMotion::Layers emLayer, const Graphics::MotionLoader& loader, const Graphics::MotionPlaybackInfo* pMotionState = nullptr);
			void StopMotion(Graphics::EmMotion::Layers emLayer, float fStopTime);

		private:
			Graphics::IModelInstance* m_pModelInst;
		};
	}
}