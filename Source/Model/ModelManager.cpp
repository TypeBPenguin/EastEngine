#include "stdafx.h"
#include "ModelManager.h"

#include "CommonLib/Performance.h"

#include "CommonLib/ThreadPool.h"
#include "CommonLib/plf_colony.h"

#include "Model.h"
#include "ModelInstance.h"
#include "ModelNodeStatic.h"
#include "GeometryModel.h"

#include "FbxImporter.h"
#include "ObjImporter.h"

namespace eastengine
{
	namespace graphics
	{
		class ModelManager::Impl
		{
		public:
			Impl();
			~Impl();

		public:
			void Update();
			void Flush(float fElapsedTime);

		public:
			void AsyncLoadModel(IModel* pModel, const ModelLoader& loader);

			IModel* AllocateModel(const IModel::Key& key);
			IModelInstance* AllocateModelInstance(Model* pModel);
			bool DestroyModelInstance(ModelInstance** ppModelInstance);

			IModel* GetModel(const IModel::Key& key) const;

		private:
			struct RequestLoadModelInfo
			{
				ModelLoader loader;
				IModel* pModel_out{ nullptr };
			};

			struct ResultLoadModelInfo
			{
				bool isSuccess = false;
				IModel* pModel_out = nullptr;
			};

			void ProcessRequestModelLoader(const RequestLoadModelInfo& loader);

		private:
			bool m_isLoading{ false };
			float m_fTime{ 0.f };

			plf::colony<Model> m_clnModel;
			plf::colony<ModelInstance> m_clnModelInstance;

			std::unordered_map<Model::Key, Model*> m_umapModelCaching;

			Concurrency::concurrent_queue<RequestLoadModelInfo> m_conQueueRequestModelLoader;
			Concurrency::concurrent_queue<ResultLoadModelInfo> m_conFuncLoadCompleteCallback;
		};

		ModelManager::Impl::Impl()
		{
			if (GeometryModel::Initialize() == false)
			{
				assert(false);
				return;
			}

			m_clnModel.reserve(128);
			m_clnModelInstance.reserve(1024);

			ObjImporter::GetInstance();
		}

		ModelManager::Impl::~Impl()
		{
			FBXImport::GetInstance()->Release();
			FBXImport::DestroyInstance();

			ObjImporter::GetInstance()->ClearData();
			ObjImporter::DestroyInstance();

			m_clnModel.clear();
			m_clnModelInstance.clear();

			GeometryModel::Release();
		}

		void ModelManager::Impl::Update()
		{
			TRACER_EVENT("ModelManager::Update");

			TRACER_BEGINEVENT("Model");
			std::for_each(m_clnModel.begin(), m_clnModel.end(), [](Model& model)
			{
				model.Ready();
			});
			TRACER_ENDEVENT();

			TRACER_BEGINEVENT("ModelInstance");
			Concurrency::parallel_for_each(m_clnModelInstance.begin(), m_clnModelInstance.end(), [](ModelInstance& mModelInstance)
			{
				if (mModelInstance.GetSkeleton() != nullptr)
				{
					mModelInstance.UpdateTransformations();
				}
				mModelInstance.UpdateModel();
			});
			TRACER_ENDEVENT();
		}

		void ModelManager::Impl::Flush(float fElapsedTime)
		{
			m_fTime += fElapsedTime;

			TRACER_EVENT("ModelManager::Flush");
			if (m_conQueueRequestModelLoader.empty() == false && m_isLoading == false)
			{
				RequestLoadModelInfo loader;
				if (m_conQueueRequestModelLoader.try_pop(loader) == true)
				{
					if (loader.pModel_out != nullptr)
					{
						m_isLoading = true;
						loader.pModel_out->SetState(IResource::eLoading);

						thread::CreateTask([this, loader]() { this->ProcessRequestModelLoader(loader); });
					}
				}
			}

			while (m_conFuncLoadCompleteCallback.empty() == false)
			{
				ResultLoadModelInfo result;
				if (m_conFuncLoadCompleteCallback.try_pop(result) == false)
					break;

				Model* pModel = static_cast<Model*>(result.pModel_out);
				if (result.isSuccess == true)
				{
					pModel->SetState(IResource::eComplete);
					pModel->SetAlive(true);

					pModel->LoadCompleteCallback(true);
				}
				else
				{
					pModel->SetState(IResource::eInvalid);
					pModel->SetAlive(false);

					pModel->LoadCompleteCallback(false);
				}
			}

			if (m_fTime >= 10.f)
			{
				m_fTime = 0.f;

				auto iter = m_clnModel.begin();
				while (iter != m_clnModel.end())
				{
					Model& model = *iter;

					if (model.GetState() == IResource::eReady ||
						model.GetState() == IResource::eLoading)
					{
						++iter;
						continue;
					}

					if (model.GetReferenceCount() > 0)
					{
						model.SetAlive(true);
						++iter;
						continue;
					}

					if (model.IsAlive() == false)
					{
						auto iter_find = m_umapModelCaching.find(model.GetKey());
						if (iter_find != m_umapModelCaching.end())
						{
							m_umapModelCaching.erase(iter_find);
						}

						iter = m_clnModel.erase(iter);
						continue;
					}

					model.SubtractLife();
					++iter;
				}
			}
		}

		void ModelManager::Impl::ProcessRequestModelLoader(const RequestLoadModelInfo& loader)
		{
			Model* pModel = static_cast<Model*>(loader.pModel_out);
			bool isSuccess = pModel->Load(loader.loader);

			m_conFuncLoadCompleteCallback.push({ isSuccess, loader.pModel_out });

			m_isLoading = false;
		}

		void ModelManager::Impl::AsyncLoadModel(IModel* pModel, const ModelLoader& loader)
		{
			pModel->SetAlive(true);

			m_conQueueRequestModelLoader.push({ loader, pModel });
		}

		IModel* ModelManager::Impl::AllocateModel(const IModel::Key& key)
		{
			auto iter_find = m_umapModelCaching.find(key);
			if (iter_find != m_umapModelCaching.end())
			{
				assert(false);
				return iter_find->second;
			}

			auto iter = m_clnModel.emplace(key);
			if (iter != m_clnModel.end())
			{
				Model* pModel = &(*iter);
				m_umapModelCaching.emplace(key, pModel);

				return pModel;
			}

			return nullptr;
		}

		IModelInstance* ModelManager::Impl::AllocateModelInstance(Model* pModel)
		{
			auto iter = m_clnModelInstance.emplace(pModel);
			if (iter == m_clnModelInstance.end())
				return nullptr;

			ModelInstance* pModelInstance = &(*iter);

			pModel->AddInstance(pModelInstance);

			return pModelInstance;
		}

		bool ModelManager::Impl::DestroyModelInstance(ModelInstance** ppModelInstance)
		{
			if (ppModelInstance == nullptr || *ppModelInstance == nullptr)
				return false;

			Model* pModel = static_cast<Model*>((*ppModelInstance)->GetModel());

			auto iter = std::find_if(m_clnModelInstance.begin(), m_clnModelInstance.end(), [ppModelInstance](ModelInstance& modelInstance)
			{
				return &modelInstance == *ppModelInstance;
			});

			if (iter == m_clnModelInstance.end())
				return false;

			if (pModel->RemoveInstance(*ppModelInstance) == false)
				return false;

			m_clnModelInstance.erase(iter);
			*ppModelInstance = nullptr;

			return true;
		}

		IModel* ModelManager::Impl::GetModel(const IModel::Key& key) const
		{
			auto iter = m_umapModelCaching.find(key);
			if (iter != m_umapModelCaching.end())
				return iter->second;

			return nullptr;
		}

		ModelManager::ModelManager()
			: m_pImpl{ std::make_unique<Impl>() }
		{
		}

		ModelManager::~ModelManager()
		{
		}

		void ModelManager::Update()
		{
			m_pImpl->Update();
		}

		void ModelManager::Flush(float fElapsedTime)
		{
			m_pImpl->Flush(fElapsedTime);
		}

		void ModelManager::AsyncLoadModel(IModel* pModel, const ModelLoader& loader)
		{
			m_pImpl->AsyncLoadModel(pModel, loader);
		}

		IModel* ModelManager::AllocateModel(const String::StringID& strKey)
		{
			IModel::Key key(strKey.Key());
			return m_pImpl->AllocateModel(key);
		}

		IModelInstance* ModelManager::AllocateModelInstance(Model* pModel)
		{
			return m_pImpl->AllocateModelInstance(pModel);
		}

		bool ModelManager::DestroyModelInstance(ModelInstance** ppModelInstance)
		{
			return m_pImpl->DestroyModelInstance(ppModelInstance);
		}

		IModel* ModelManager::GetModel(const String::StringID& strKey) const
		{
			IModel::Key key(strKey.Key());
			return m_pImpl->GetModel(key);
		}
	}
}