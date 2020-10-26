#include "stdafx.h"
#include "ModelManager.h"

#include "CommonLib/Performance.h"

#include "CommonLib/ObjectPool.h"
#include "CommonLib/ThreadPool.h"
#include "CommonLib/Timer.h"

#include "Model.h"
#include "ModelInstance.h"
#include "ModelNodeStatic.h"
#include "GeometryModel.h"

#include "Motion.h"

#include "FbxImporter.h"
#include "ObjImporter.h"

namespace est
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
			void AsyncLoadModel(IModel* pModel, const ModelLoader& loader) { m_conQueueRequestModelLoader.push({ loader, pModel }); }

			IModel* AllocateModel(const IModel::Key& key);
			IModelInstance* AllocateModelInstance(Model* pModel);
			bool DestroyModelInstance(IModelInstance** ppModelInstance);

			IModel* GetModel(const IModel::Key& key) const;

		public:
			// FilePath or ModelName
			MotionPtr AllocateMotion(const IMotion::Key& key);
			bool DestroyMotion(MotionPtr* ppMotion);

			// FilePath or ModelName
			MotionPtr GetMotion(const IMotion::Key& key);
			MotionPtr GetMotion(const size_t index);

			size_t GetMotionCount() const;

		public:
			bool LoadModelFBX(Model* pModel, const wchar_t* filePath, float scale, bool isFlipZ);
			bool LoadMotionFBX(Motion* pMotion, const wchar_t* filePath, float scale);

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
			void Optimize();

		private:
			bool m_isLoading{ false };
			bool m_isDirty_model{ false };
			bool m_isDirty_modelInstance{ false };

			float m_time{ 0.f };
			float m_optimizeTime{ 0.f };

			std::unique_ptr<FBXImport> m_pFBXImport;

			memory::ObjectPool<Model, 256> m_poolModel;
			std::vector<Model*> m_models;
			tsl::robin_map<IModel::Key, Model*> m_umapModelCaching;

			memory::ObjectPool<ModelInstance, 1024> m_poolModelInstance;
			std::vector<ModelInstance*> m_modelInstances;

			struct MotionData
			{
				double destroyWaitTime{ 0.0 };
				MotionPtr pMotion;
			};
			memory::ObjectPool<Motion, 512> m_poolMotion;
			tsl::robin_map<IMotion::Key, MotionData> m_umapMotions;

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

			ObjImporter::GetInstance();
			m_pFBXImport = std::make_unique<FBXImport>();
		}

		ModelManager::Impl::~Impl()
		{
			SafeRelease(m_pFBXImport);

			ObjImporter::GetInstance()->ClearData();
			ObjImporter::DestroyInstance();

			for (auto& pModel : m_models)
			{
				m_poolModel.Destroy(pModel);
			}
			m_models.clear();

			for (auto& pModelInstance : m_modelInstances)
			{
				m_poolModelInstance.Destroy(pModelInstance);
			}
			m_modelInstances.clear();

			geometry::Release();
		}

		void ModelManager::Impl::Update()
		{
			TRACER_EVENT(__FUNCTIONW__);

			{
				TRACER_EVENT(L"Model");
				for (auto& pModel : m_models)
				{
					pModel->Ready();
				}
			}

			{
				TRACER_EVENT(L"ModelInstance");
				jobsystem::ParallelFor(m_modelInstances.size(), [&](size_t index)
					{
						if (m_modelInstances[index]->IsAttachment() == false)
						{
							m_modelInstances[index]->UpdateModel();
						}
					});
			}
		}

		void ModelManager::Impl::Cleanup(float elapsedTime)
		{
			TRACER_EVENT(L"ModelManager::Flush");

			m_time += elapsedTime;

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

					pModel->LoadCompleteCallback(true);
				}
				else
				{
					pModel->SetState(IResource::eInvalid);

					pModel->LoadCompleteCallback(false);
				}
			}

			constexpr float OptimizeInterval = 10.f;
			if (m_optimizeTime >= OptimizeInterval)
			{
				m_optimizeTime = 0.f;

				Optimize();
			}

			constexpr float CleanupInterval = 10.f;
			if (m_time >= CleanupInterval)
			{
				m_time = 0.f;

				const double gameTime = Timer::GetInstance()->GetGameTime();

				m_models.erase(std::remove_if(m_models.begin(), m_models.end(), [&](Model* pModel)
					{
						if (pModel->GetState() == IResource::eReady ||
							pModel->GetState() == IResource::eLoading)
						{
							return false;
						}

						if (pModel->IsReadyToDestroy(gameTime) == true)
						{
							auto iter_find = m_umapModelCaching.find(pModel->GetKey());
							if (iter_find != m_umapModelCaching.end())
							{
								m_umapModelCaching.erase(iter_find);
							}
							m_poolModel.Destroy(pModel);
							return true;
						}

						return false;
					}), m_models.end());

				for (auto iter = m_umapMotions.begin(); iter != m_umapMotions.end();)
				{
					MotionData& motionData = iter.value();

					if (motionData.pMotion->GetState() == IResource::eReady ||
						motionData.pMotion->GetState() == IResource::eLoading)
					{
						++iter;
						continue;
					}

					if (motionData.pMotion.use_count() <= 1)
					{
						if (math::IsZero(motionData.destroyWaitTime) == true)
						{
							motionData.destroyWaitTime = gameTime + 120.0;
						}

						if (motionData.destroyWaitTime < gameTime)
						{
							iter = m_umapMotions.erase(iter);
						}
						else
						{
							++iter;
						}
					}
					else
					{
						++iter;
					}
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

		void ModelManager::Impl::Optimize()
		{
			if (m_isDirty_model == true)
			{
				std::sort(m_models.begin(), m_models.end());
				m_isDirty_model = false;
			}

			if (m_isDirty_modelInstance == true)
			{
				std::sort(m_modelInstances.begin(), m_modelInstances.end());
				m_isDirty_modelInstance = false;
			}
		}

		IModel* ModelManager::Impl::AllocateModel(const IModel::Key& key)
		{
			auto iter_find = m_umapModelCaching.find(key);
			if (iter_find != m_umapModelCaching.end())
			{
				assert(false);
				return iter_find->second;
			}

			Model* pModel = m_poolModel.Allocate(key);
			m_models.emplace_back(pModel);
			m_umapModelCaching.emplace(key, pModel);

			m_isDirty_model = true;

			return pModel;
		}

		IModelInstance* ModelManager::Impl::AllocateModelInstance(Model* pModel)
		{
			ModelInstance* pModelInstance = m_poolModelInstance.Allocate(pModel);
			m_modelInstances.emplace_back(pModelInstance);

			pModel->AddInstance(pModelInstance);

			m_isDirty_modelInstance = true;

			return pModelInstance;
		}

		bool ModelManager::Impl::DestroyModelInstance(IModelInstance** ppModelInstance)
		{
			if (ppModelInstance == nullptr || *ppModelInstance == nullptr)
				return true;

			ModelInstance* pModelInstance = static_cast<ModelInstance*>(*ppModelInstance);
			Model* pModel = static_cast<Model*>((*ppModelInstance)->GetModel());

			auto iter = std::find(m_modelInstances.begin(), m_modelInstances.end(), pModelInstance);
			if (iter == m_modelInstances.end())
				return false;

			if (pModel->RemoveInstance(pModelInstance) == false)
			{
				return false;
			}

			m_modelInstances.erase(iter);
			m_poolModelInstance.Destroy(pModelInstance);
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

		MotionPtr ModelManager::Impl::AllocateMotion(const IMotion::Key& key)
		{
			auto iter_find = m_umapMotions.find(key);
			if (iter_find != m_umapMotions.end())
				return iter_find->second.pMotion;

			MotionData motionData;
			motionData.pMotion = std::shared_ptr<Motion>(m_poolMotion.Allocate(key), [&](Motion* pMotion)
				{
					m_poolMotion.Destroy(pMotion);
				});

			auto iter_result = m_umapMotions.emplace(key, motionData);
			if (iter_result.second == true)
				return iter_result.first->second.pMotion;

			return nullptr;
		}

		bool ModelManager::Impl::DestroyMotion(MotionPtr* ppMotion)
		{
			if (ppMotion == nullptr || *ppMotion == nullptr)
				return true;

			auto iter_find = m_umapMotions.find((*ppMotion)->GetKey());
			if (iter_find == m_umapMotions.end())
				return false;

			m_umapMotions.erase(iter_find);
			*ppMotion = nullptr;
			return true;
		}

		MotionPtr ModelManager::Impl::GetMotion(const IMotion::Key& key)
		{
			auto iter = m_umapMotions.find(key);
			if (iter != m_umapMotions.end())
			{
				iter.value().destroyWaitTime = 0.0;
				return iter->second.pMotion;
			}

			return nullptr;
		}

		MotionPtr ModelManager::Impl::GetMotion(const size_t index)
		{
			auto iter = m_umapMotions.begin();
			std::advance(iter, index);

			if (iter != m_umapMotions.end())
			{
				iter.value().destroyWaitTime = 0.0;
				return iter->second.pMotion;
			}

			return nullptr;
		}

		size_t ModelManager::Impl::GetMotionCount() const
		{
			return m_umapMotions.size();
		}

		bool ModelManager::Impl::LoadModelFBX(Model* pModel, const wchar_t* filePath, float scale, bool isFlipZ)
		{
			return m_pFBXImport->LoadModel(pModel, filePath, scale, isFlipZ);
		}

		bool ModelManager::Impl::LoadMotionFBX(Motion* pMotion, const wchar_t* filePath, float scale)
		{
			return m_pFBXImport->LoadMotion(pMotion, filePath, scale);
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

		bool ModelManager::DestroyModelInstance(IModelInstance** ppModelInstance)
		{
			return m_pImpl->DestroyModelInstance(ppModelInstance);
		}

		IModel* ModelManager::GetModel(const string::StringID& strKey) const
		{
			IModel::Key key(strKey);
			return m_pImpl->GetModel(key);
		}

		MotionPtr ModelManager::AllocateMotion(const string::StringID& strKey)
		{
			IMotion::Key key(strKey);
			return m_pImpl->AllocateMotion(key);
		}

		bool ModelManager::DestroyMotion(MotionPtr* ppMotion)
		{
			return m_pImpl->DestroyMotion(ppMotion);
		}

		MotionPtr ModelManager::GetMotion(const string::StringID& strKey)
		{
			IMotion::Key key(strKey);
			return m_pImpl->GetMotion(key);
		}

		MotionPtr ModelManager::GetMotion(const size_t index)
		{
			return m_pImpl->GetMotion(index);
		}

		size_t ModelManager::GetMotionCount() const
		{
			return m_pImpl->GetMotionCount();
		}

		bool ModelManager::LoadModelFBX(Model* pModel, const wchar_t* filePath, float scale, bool isFlipZ)
		{
			return m_pImpl->LoadModelFBX(pModel, filePath, scale, isFlipZ);
		}

		bool ModelManager::LoadMotionFBX(Motion* pMotion, const wchar_t* filePath, float scale)
		{
			return m_pImpl->LoadMotionFBX(pMotion, filePath, scale);
		}
	}
}