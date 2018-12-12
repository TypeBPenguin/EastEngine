#include "stdafx.h"
#include "ModelManager.h"

#include "CommonLib/Performance.h"

#include "CommonLib/ThreadPool.h"
#include "CommonLib/plf_colony.h"
#include "CommonLib/Timer.h"

#include "Model.h"
#include "ModelInstance.h"
#include "ModelNodeStatic.h"
#include "GeometryModel.h"

#include "Motion.h"

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
			void Cleanup(float elapsedTime);

		public:
			void AsyncLoadModel(IModel* pModel, const ModelLoader& loader);

			IModel* AllocateModel(const IModel::Key& key);
			IModelInstance* AllocateModelInstance(Model* pModel);
			bool DestroyModelInstance(ModelInstance** ppModelInstance);

			IModel* GetModel(const IModel::Key& key) const;

		public:
			// FilePath or ModelName
			IMotion* AllocateMotion(const IMotion::Key& key);

			// FilePath or ModelName
			IMotion* GetMotion(const IMotion::Key& key);
			IMotion* GetMotion(const size_t nIndex);

			size_t GetMotionCount() const;

		public:
			bool LoadModelFBX(Model* pModel, const char* strFilePath, float fScale, bool isFlipZ);
			bool LoadMotionFBX(Motion* pMotion, const char* strFilePath, float fScale);

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

			std::unique_ptr<FBXImport> m_pFBXImport;

			plf::colony<Model> m_clnModel;
			plf::colony<ModelInstance> m_clnModelInstance;

			plf::colony<Motion> m_clnMotions;

			tsl::robin_map<IModel::Key, Model*> m_umapModelCaching;
			tsl::robin_map<IMotion::Key, Motion*> m_umapMotions;

			Concurrency::concurrent_queue<RequestLoadModelInfo> m_conQueueRequestModelLoader;
			Concurrency::concurrent_queue<ResultLoadModelInfo> m_conFuncLoadCompleteCallback;
		};

		ModelManager::Impl::Impl()
		{
			if (geometry::Initialize() == false)
			{
				assert(false);
				return;
			}

			m_clnModel.reserve(128);
			m_clnModelInstance.reserve(1024);

			ObjImporter::GetInstance();

			m_pFBXImport = std::make_unique<FBXImport>();
		}

		ModelManager::Impl::~Impl()
		{
			SafeRelease(m_pFBXImport);

			ObjImporter::GetInstance()->ClearData();
			ObjImporter::DestroyInstance();

			m_clnModel.clear();
			m_clnModelInstance.clear();

			geometry::Release();
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
				mModelInstance.UpdateModel();
			});
			TRACER_ENDEVENT();
		}

		void ModelManager::Impl::Cleanup(float elapsedTime)
		{
			TRACER_EVENT("ModelManager::Flush");

			m_fTime += elapsedTime;

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

		IMotion* ModelManager::Impl::AllocateMotion(const IMotion::Key& key)
		{
			auto iter_find = m_umapMotions.find(key);
			if (iter_find != m_umapMotions.end())
			{
				assert(false);
				return iter_find->second;
			}

			auto iter = m_clnMotions.emplace(key);
			if (iter != m_clnMotions.end())
			{
				Motion* pMotion = &(*iter);
				m_umapMotions.emplace(key, pMotion);

				return pMotion;
			}

			return nullptr;
		}

		IMotion* ModelManager::Impl::GetMotion(const IMotion::Key& key)
		{
			auto iter = m_umapMotions.find(key);
			if (iter != m_umapMotions.end())
				return iter->second;

			return nullptr;
		}

		IMotion* ModelManager::Impl::GetMotion(const size_t nIndex)
		{
			auto iter = m_clnMotions.begin();
			m_clnMotions.advance(iter, nIndex);

			if (iter != m_clnMotions.end())
				return &(*iter);

			return nullptr;
		}

		size_t ModelManager::Impl::GetMotionCount() const
		{
			return m_clnMotions.size();
		}

		bool ModelManager::Impl::LoadModelFBX(Model* pModel, const char* strFilePath, float fScale, bool isFlipZ)
		{
			return m_pFBXImport->LoadModel(pModel, strFilePath, fScale, isFlipZ);
		}

		bool ModelManager::Impl::LoadMotionFBX(Motion* pMotion, const char* strFilePath, float fScale)
		{
			return m_pFBXImport->LoadMotion(pMotion, strFilePath, fScale);
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

		void ModelManager::Cleanup(float elapsedTime)
		{
			m_pImpl->Cleanup(elapsedTime);
		}

		void ModelManager::AsyncLoadModel(IModel* pModel, const ModelLoader& loader)
		{
			m_pImpl->AsyncLoadModel(pModel, loader);
		}

		IModel* ModelManager::AllocateModel(const string::StringID& strKey)
		{
			IModel::Key key(strKey);
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

		IModel* ModelManager::GetModel(const string::StringID& strKey) const
		{
			IModel::Key key(strKey);
			return m_pImpl->GetModel(key);
		}

		IMotion* ModelManager::AllocateMotion(const string::StringID& strKey)
		{
			IMotion::Key key(strKey);
			return m_pImpl->AllocateMotion(key);
		}

		IMotion* ModelManager::GetMotion(const string::StringID& strKey)
		{
			IMotion::Key key(strKey);
			return m_pImpl->GetMotion(key);
		}

		IMotion* ModelManager::GetMotion(const size_t nIndex)
		{
			return m_pImpl->GetMotion(nIndex);
		}

		size_t ModelManager::GetMotionCount() const
		{
			return m_pImpl->GetMotionCount();
		}

		bool ModelManager::LoadModelFBX(Model* pModel, const char* strFilePath, float fScale, bool isFlipZ)
		{
			return m_pImpl->LoadModelFBX(pModel, strFilePath, fScale, isFlipZ);
		}

		bool ModelManager::LoadMotionFBX(Motion* pMotion, const char* strFilePath, float fScale)
		{
			return m_pImpl->LoadMotionFBX(pMotion, strFilePath, fScale);
		}
	}
}