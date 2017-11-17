#pragma once

#include "ComponentInterface.h"

namespace EastEngine
{
	namespace Graphics
	{
		class IModel;
		class IModelInstance;
		class IMotion;
		class ModelLoader;
		class MotionLoader;

		struct MotionState;
	}

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

			virtual bool LoadToFile(File::FileStream& file);
			virtual bool SaveToFile(File::FileStream& file);

		public:
			Graphics::IModelInstance* GetModelInstance() { return m_pModelInst; }
			Graphics::IModel* GetModel();
			bool IsLoadComplete();

		public:
			bool PlayMotion(Graphics::IMotion* pMotion, const Graphics::MotionState* pMotionState = nullptr);
			bool PlayMotion(const Graphics::MotionLoader& loader, const Graphics::MotionState* pMotionState = nullptr);
			void StopMotion(float fStopTime);
			Graphics::IMotion* GetMotioin();

		private:
			Graphics::IModelInstance* m_pModelInst;
		};
	}
}