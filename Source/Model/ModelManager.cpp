#include "stdafx.h"
#include "ModelManager.h"

#include "CommonLib/ThreadPool.h"

#include "Model.h"
#include "ModelInstance.h"

#include "ModelNodeStatic.h"
#include "GeometryModel.h"

#include "FbxImporter.h"
#include "ObjImporter.h"

namespace EastEngine
{
	namespace Graphics
	{
		ModelManager::RequestLoadModelInfo::RequestLoadModelInfo()
			: pModel_out(nullptr)
		{
		}

		ModelManager::RequestLoadModelInfo::RequestLoadModelInfo(const ModelLoader& loader, IModel* pModel_out)
			: loader(loader)
			, pModel_out(pModel_out)
		{
		}

		ModelManager::RequestLoadModelInfo::RequestLoadModelInfo(const RequestLoadModelInfo& source)
			: loader(source.loader)
			, pModel_out(source.pModel_out)
		{
		}

		ModelManager::ResultLoadModelInfo::ResultLoadModelInfo()
			: isSuccess(false)
		{
		}

		ModelManager::ResultLoadModelInfo::ResultLoadModelInfo(bool isSuccess, IModel* pModel_out)
			: isSuccess(isSuccess)
			, pModel_out(pModel_out)
		{
		}

		ModelManager::ModelManager()
			: m_isInit(false)
			, m_isLoading(false)
		{
		}

		ModelManager::~ModelManager()
		{
			Release();
		}

		bool ModelManager::Init()
		{
			if (m_isInit == true)
				return true;

			m_isInit = true;

			m_clnModel.reserve(128);
			m_vecJobUpdateTransformations.reserve(1024);
			m_vecJobUpdateModels.reserve(1024);

			if (GeometryModel::Initialize() == false)
			{
				Release();
				return false;
			}

			SObjImporter::GetInstance();

			return true;
		}

		void ModelManager::Release()
		{
			if (m_isInit == false)
				return;

			FBXImport::GetInstance()->Release();
			FBXImport::DestroyInstance();

			SObjImporter::GetInstance()->ClearData();
			SObjImporter::DestroyInstance();

			m_clnModel.clear();

			GeometryModel::Release();

			m_isInit = false;
		}

		void ModelManager::Update()
		{
			if (m_conQueueRequestModelLoader.empty() == false && m_isLoading == false)
			{
				RequestLoadModelInfo loader;
				if (m_conQueueRequestModelLoader.try_pop(loader) == true)
				{
					if (loader.pModel_out != nullptr)
					{
						m_isLoading = true;
						loader.pModel_out->SetLoadState(EmLoadState::eLoading);

						Thread::CreateTask([this, loader]() { this->ProcessRequestModelLoader(loader); });
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
					pModel->SetLoadState(EmLoadState::eComplete);
					pModel->SetAlive(true);

					pModel->LoadCompleteCallback(true);
				}
				else
				{
					pModel->SetLoadState(EmLoadState::eInvalid);
					pModel->SetAlive(false);

					pModel->LoadCompleteCallback(false);
				}
			}

			std::for_each(m_clnModel.begin(), m_clnModel.end(), [](Model& model)
			{
				model.Ready();
			});

			Concurrency::parallel_for_each(m_vecJobUpdateTransformations.begin(), m_vecJobUpdateTransformations.end(), [](ModelInstance* pModelInstance)
			{
				pModelInstance->UpdateTransformations();
			});
			m_vecJobUpdateTransformations.clear();

			Concurrency::parallel_for_each(m_vecJobUpdateModels.begin(), m_vecJobUpdateModels.end(), [](ModelInstance* pModelInstance)
			{
				pModelInstance->UpdateModel();
			});
			m_vecJobUpdateModels.clear();
		}

		void ModelManager::Flush()
		{
			auto iter = m_clnModel.begin();
			while (iter != m_clnModel.end())
			{
				Model& model = *iter;

				if (model.GetLoadState() == EmLoadState::eReady ||
					model.GetLoadState() == EmLoadState::eLoading)
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
					iter = m_clnModel.erase(iter);
					continue;
				}

				model.SubtractLife();
				++iter;
			}
		}

		void ModelManager::ProcessRequestModelLoader(const RequestLoadModelInfo& loader)
		{
			Model* pModel = static_cast<Model*>(loader.pModel_out);
			bool isSuccess = pModel->Load(loader.loader);

			m_conFuncLoadCompleteCallback.push(ResultLoadModelInfo(isSuccess, loader.pModel_out));

			m_isLoading = false;
		}

		void ModelManager::LoadModelSync(IModel* pModel, const ModelLoader& loader)
		{
			pModel->SetAlive(true);

			m_conQueueRequestModelLoader.push(RequestLoadModelInfo(loader, pModel));
		}

		IModel* ModelManager::AllocateModel(uint32_t nReserveInstance)
		{
			auto iter = m_clnModel.emplace(nReserveInstance);
			if (iter != m_clnModel.end())
				return &(*iter);

			return nullptr;
		}

		void ModelManager::DestroyModel(IModel** ppModel)
		{
			if (ppModel == nullptr || *ppModel == nullptr)
				return;

			auto iter = std::find_if(m_clnModel.begin(), m_clnModel.end(), [ppModel](Model& model)
			{
				return &model == *ppModel;
			});

			if (iter == m_clnModel.end())
				return;

			m_clnModel.erase(iter);
			*ppModel = nullptr;
		}

		bool ModelManager::ChangeName(IModel* pModel, const String::StringID& strName)
		{
			auto iter = std::find_if(m_clnModel.begin(), m_clnModel.end(), [strName](Model& model)
			{
				return model.GetName() == strName;
			});

			if (iter == m_clnModel.end())
				return false;

			Model& model = *iter;
			model.SetName(strName);

			return true;
		}

		IModel* ModelManager::GetModel(const String::StringID& strModelName)
		{
			auto iter = std::find_if(m_clnModel.begin(), m_clnModel.end(), [strModelName](Model& model)
			{
				return model.GetName() == strModelName;
			});

			if (iter != m_clnModel.end())
				return &(*iter);

			return nullptr;
		}
	}
}