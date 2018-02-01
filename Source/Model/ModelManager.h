#pragma once

#include "CommonLib/Singleton.h"

#include "ModelLoader.h"

namespace EastEngine
{
	namespace Graphics
	{
		class Model;
		class IModel;
		class ModelInstance;
		class IModelInstance;

		class ModelManager : public Singleton<ModelManager>
		{
			friend Singleton<ModelManager>;
		private:
			ModelManager();
			virtual ~ModelManager();

		public:
			void Update();
			void Flush();

		public:
			void AsyncLoadModel(IModel* pModel, const ModelLoader& loader);

			// FilePath or ModelName
			IModel* AllocateModel(const std::string& strKey);
			IModelInstance* AllocateModelInstance(Model* pModel);
			bool DestroyModelInstance(ModelInstance** ppModelInstance);

			// FilePath or ModelName
			IModel* GetModel(const std::string& strKey) const;

		private:
			class Impl;
			std::unique_ptr<Impl> m_pImpl;
		};
	}
}