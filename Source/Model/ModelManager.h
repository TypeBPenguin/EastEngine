#pragma once

#include "CommonLib/Singleton.h"
#include "CommonLib/plf_colony.h"

#include "ModelLoader.h"

namespace EastEngine
{
	namespace Graphics
	{
		class Model;
		class ModelInstance;
		class IModel;
		class ModelLoader;

		class ModelManager : public Singleton<ModelManager>
		{
			friend Singleton<ModelManager>;
		private:
			ModelManager();
			virtual ~ModelManager();

			struct RequestLoadModelInfo
			{
				ModelLoader loader;
				IModel* pModel_out;

				RequestLoadModelInfo();
				RequestLoadModelInfo(const ModelLoader& loader, IModel* pModel_out);
				RequestLoadModelInfo(const RequestLoadModelInfo& source);
			};

			struct ResultLoadModelInfo
			{
				bool isSuccess = false;
				IModel* pModel_out;

				ResultLoadModelInfo();
				ResultLoadModelInfo(bool isSuccess, IModel* pModel_out);
			};

		public:
			bool Init();
			void Release();

			void Update();
			void Flush();

			void ProcessRequestModelLoader(const RequestLoadModelInfo& loader);

			void LoadModelSync(IModel* pModel, const ModelLoader& loader);

			IModel* AllocateModel(uint32_t nReserveInstance);
			void DestroyModel(IModel** ppModel);

			bool ChangeName(IModel* pModel, const String::StringID& strName);
			IModel* GetModel(const String::StringID& strModelName);

		public:
			void PushJobUpdateModels(ModelInstance* pModelInstance) { m_vecJobUpdateModels.emplace_back(pModelInstance); }

		private:
			bool m_isInit;
			bool m_isLoading;

			plf::colony<Model> m_clnModel;
			std::vector<ModelInstance*> m_vecJobUpdateModels;

			Concurrency::concurrent_queue<RequestLoadModelInfo> m_conQueueRequestModelLoader;
			Concurrency::concurrent_queue<ResultLoadModelInfo> m_conFuncLoadCompleteCallback;
		};
	}
}