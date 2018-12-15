#include "stdafx.h"
#include "ModelInterface.h"

#include "CommonLib/FileUtil.h"
#include "CommonLib/FileStream.h"
#include "CommonLib/Timer.h"

#include "ModelManager.h"

#include "Model.h"
#include "ModelInstance.h"

#include "ModelNodeSkinned.h"

#include "Skeleton.h"

#include "Motion.h"
#include "MotionSystem.h"

#include "XpsImporter.h"

namespace eastengine
{
	namespace graphics
	{
		LODReductionRate::LODReductionRate()
		{
			levels[0] = 1.f;
			levels[1] = 0.775f;
			levels[2] = 0.55f;
			levels[3] = 0.325f;
			levels[4] = 0.1f;
		}

		LODReductionRate::LODReductionRate(float lv0, float lv1, float lv2, float lv3, float lv4)
		{
			levels[0] = lv0;
			levels[1] = lv1;
			levels[2] = lv2;
			levels[3] = lv3;
			levels[4] = lv4;
		}

		IMotionEvent::IMotionEvent(int nID)
			: nID(nID)
		{
		}

		IMotionEvent::~IMotionEvent()
		{
		}

		IMotion* IMotion::Create(const MotionLoader& loader)
		{
			string::StringID strKey = loader.GetFilePath().c_str();

			IMotion* pIMotion = ModelManager::GetInstance()->GetMotion(strKey);
			if (pIMotion != nullptr)
				return pIMotion;

			Motion* pMotion = static_cast<Motion*>(ModelManager::GetInstance()->AllocateMotion(strKey));
			pMotion->SetName(loader.GetName());
			pMotion->SetFilePath(loader.GetFilePath());

			switch (loader.GetLoadMotionType())
			{
			case EmMotionLoader::eFbx:
			{
				string::StringID strMotionName = file::GetFileNameWithoutExtension(loader.GetFilePath().c_str()).c_str();

				Stopwatch sw;
				sw.Start();
				if (ModelManager::GetInstance()->LoadMotionFBX(pMotion, loader.GetFilePath().c_str(), loader.GetScaleFactor()) == false)
				{
					pMotion->SetState(IResource::eInvalid);
					return nullptr;
				}
				sw.Stop();
				LOG_MESSAGE("FBX Motion Load Complete : %lf[%s]", sw.Elapsed(), loader.GetFilePath().c_str());

				pMotion->SetState(IResource::eComplete);

				return pMotion;
			}
			break;
			case EmMotionLoader::eXps:
			{
				string::StringID strMotionName = file::GetFileNameWithoutExtension(loader.GetFilePath().c_str()).c_str();

				Stopwatch sw;
				sw.Start();
				if (XPSImport::LoadMotion(pMotion, loader.GetFilePath().c_str()) == false)
				{
					pMotion->SetState(IResource::eInvalid);
					return nullptr;
				}
				sw.Stop();
				LOG_MESSAGE("Xps Motion Load Complete : %lf[%s]", sw.Elapsed(), loader.GetFilePath().c_str());

				pMotion->SetState(IResource::eComplete);

				return pMotion;
			}
			break;
			case EmMotionLoader::eEast:
			{
				Stopwatch sw;
				sw.Start();
				if (pMotion->LoadFile(loader.GetFilePath().c_str()) == false)
				{
					pMotion->SetState(IResource::eInvalid);
					return nullptr;
				}
				sw.Stop();
				LOG_MESSAGE("Emot Motion Load Complete : %lf[%s]", sw.Elapsed(), loader.GetFilePath().c_str());
				
				pMotion->SetState(IResource::eComplete);
				
				return pMotion;
			}
			break;
			default:
				return nullptr;
			}

			return nullptr;
		}

		void IMotion::Destroy(IMotion** ppMotion)
		{
			if (ppMotion == nullptr || *ppMotion == nullptr)
				return;

			// 모션 리소스는 매니저의 Flush 함수에서 자원해제 있는데, 여기서도 뭔가 해줄수있는게 있는지 생각해보자
			*ppMotion = nullptr;
		}

		bool IMotion::SaveFile(IMotion* pMotion, const char* strFilePath)
		{
			file::Stream file;
			if (file.Open(strFilePath, file::eWriteBinary) == false)
			{
				LOG_WARNING("Can't save to file : %s", strFilePath);
				return false;
			}

			file << pMotion->GetName().c_str();

			file << pMotion->GetStartTime();
			file << pMotion->GetEndTime();
			file << pMotion->GetFrameInterval();

			const uint32_t nBoneCount = pMotion->GetBoneCount();
			file << nBoneCount;

			for (uint32_t i = 0; i < nBoneCount; ++i)
			{
				const IMotion::IBone* pBone = pMotion->GetBone(i);

				file << pBone->GetName().c_str();

				uint32_t nKeyframeCount = pBone->GetKeyframeCount();
				file << nKeyframeCount;

				for (uint32_t j = 0; j < nKeyframeCount; ++j)
				{
					const Keyframe* pKeyframe = pBone->GetKeyframe(j);

					file << pKeyframe->fTime;
					file.Write(&pKeyframe->transform.scale.x, 3);
					file.Write(&pKeyframe->transform.rotation.x, 4);
					file.Write(&pKeyframe->transform.position.x, 3);
				}
			}

			file.Close();

			return true;
		}

		IModel* IModel::Create(const ModelLoader& loader, bool isThreadLoad)
		{
			string::StringID strKey;
			if (loader.GetLoadModelType() == ModelLoader::LoadType::eGeometry)
			{
				strKey = loader.GetModelName();
			}
			else
			{
				strKey = loader.GetFilePath().c_str();
			}

			IModel* pIModel = ModelManager::GetInstance()->GetModel(strKey);
			if (pIModel != nullptr)
				return pIModel;

			Model* pModel = static_cast<Model*>(ModelManager::GetInstance()->AllocateModel(strKey));
			pModel->SetName(loader.GetModelName());
			pModel->SetFilePath(loader.GetFilePath());
			pModel->SetLocalPosition(loader.GetLocalPosition());
			pModel->SetLocalRotation(loader.GetLocalRotation());
			pModel->SetLocalScale(loader.GetLocalScale());
			pModel->SetState(IResource::eReady);

			if (isThreadLoad == true)
			{
				ModelManager::GetInstance()->AsyncLoadModel(pModel, loader);
			}
			else
			{
				if (pModel->Load(loader) == true)
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

			return pModel;
		}

		IModelInstance* IModel::CreateInstance(const ModelLoader& loader, bool isThreadLoad)
		{
			Model* pModel = static_cast<Model*>(IModel::Create(loader, isThreadLoad));
			if (pModel == nullptr)
				return nullptr;

			return CreateInstance(pModel);
		}

		IModelInstance* IModel::CreateInstance(IModel* pIModel)
		{
			if (pIModel == nullptr)
				return nullptr;

			Model* pModel = static_cast<Model*>(pIModel);
			return ModelManager::GetInstance()->AllocateModelInstance(pModel);
		}

		void IModel::DestroyInstance(IModelInstance** ppModelInst)
		{
			if (ppModelInst == nullptr || *ppModelInst == nullptr)
				return;

			ModelInstance* pModelInstance = static_cast<ModelInstance*>(*ppModelInst);
			if (ModelManager::GetInstance()->DestroyModelInstance(&pModelInstance) == true)
			{
				*ppModelInst = nullptr;
			}
		}

		bool IModel::SaveFile(IModel* pModel, const char* strFilePath)
		{
			if (strFilePath == nullptr)
				return false;

			Model* pRealModel = static_cast<Model*>(pModel);
			if (pRealModel == nullptr)
				return false;

			// 좀 더 구조적으로 쉽고 간편한 Save Load 방식이 필요함
			// Stream 은 빨라서 좋지만, 데이터 규격이 달라지면 기존 데이터를 사용할 수 없게됨
			// 또는 확실한 버전 관리로, 버전별 Save Load 로직을 구별한다면 Stream 으로도 문제없음
			file::Stream file;
			if (file.Open(strFilePath, file::eWriteBinary) == false)
			{
				LOG_WARNING("Can't save to file : %s", strFilePath);
				return false;
			}

			// Common
			file << pModel->GetName().c_str();

			file.Write(&pModel->GetLocalPosition().x, 3);
			file.Write(&pModel->GetLocalScale().x, 3);
			file.Write(&pModel->GetLocalRotation().x, 4);

			// Node
			uint32_t nNodeCount = pModel->GetNodeCount();
			file << nNodeCount;

			for (uint32_t i = 0; i < nNodeCount; ++i)
			{
				IModelNode* pNode = pModel->GetNode(i);

				file << pNode->GetType();

				file << pNode->GetName().c_str();

				file << pNode->GetParentName().c_str();
				file << pNode->GetAttachedBoneName().c_str();

				const collision::AABB& aabb = pNode->GetOriginAABB();
				file.Write(&aabb.Center.x, 3);
				file.Write(&aabb.Extents.x, 3);

				file << pNode->IsVisible();

				uint32_t nSubsetCount = pNode->GetModelSubsetCount();
				file << nSubsetCount;

				for (uint32_t j = 0; j < nSubsetCount; ++j)
				{
					const ModelSubset* pModelSubset = pNode->GetModelSubset(j);
					file << pModelSubset->strName.c_str();
					file << pModelSubset->nStartIndex;
					file << pModelSubset->nIndexCount;
					file << pModelSubset->nMaterialID;
					file << pModelSubset->emPrimitiveType;
				}

				IVertexBuffer* pVertexBuffer = pNode->GetVertexBuffer();

				void* pData = nullptr;
				if (pVertexBuffer->Map(&pData) == false)
				{
					LOG_ERROR("Can't map vertexbuffer");
					return false;
				}

				uint32_t nVertexCount = pVertexBuffer->GetVertexCount();
				file << nVertexCount;

				if (pNode->GetType() == IModelNode::eStatic)
				{
					const VertexPosTexNor* pVertices = reinterpret_cast<const VertexPosTexNor*>(pData);
					for (uint32_t j = 0; j < nVertexCount; ++j)
					{
						file.Write(&pVertices[j].pos.x, 3);
						file.Write(&pVertices[j].uv.x, 2);
						file.Write(&pVertices[j].normal.x, 3);
					}
				}
				else if (pNode->GetType() == IModelNode::eSkinned)
				{
					const VertexPosTexNorWeiIdx* pVertices = reinterpret_cast<const VertexPosTexNorWeiIdx*>(pData);
					for (uint32_t j = 0; j < nVertexCount; ++j)
					{
						file.Write(&pVertices[j].pos.x, 3);
						file.Write(&pVertices[j].uv.x, 2);
						file.Write(&pVertices[j].normal.x, 3);
						file.Write(&pVertices[j].boneWeight.x, 3);
						file.Write(&pVertices[j].boneIndices[0], 4);
					}
				}

				pVertexBuffer->Unmap();

				IIndexBuffer* pIndexBuffer = pNode->GetIndexBuffer();

				pData = nullptr;
				if (pIndexBuffer->Map(&pData) == false)
				{
					LOG_ERROR("Can't map indexbuffer");
					return false;
				}

				uint32_t nIndexCount = pIndexBuffer->GetIndexCount();
				file << nIndexCount;

				const uint32_t* pIndices = reinterpret_cast<const uint32_t*>(pData);
				for (uint32_t j = 0; j < nIndexCount; ++j)
				{
					file << pIndices[j];
				}

				pIndexBuffer->Unmap();

				uint32_t nMaterialCount = pNode->GetMaterialCount();
				file << nMaterialCount;

				std::string strPath = file::GetFilePath(strFilePath);
				for (uint32_t j = 0; j < nMaterialCount; ++j)
				{
					IMaterial* pMaterial = pNode->GetMaterial(j);
					file << pMaterial->GetName().c_str();

					pMaterial->SaveFile(strPath.c_str());
				}

				if (pNode->GetType() == IModelNode::eSkinned)
				{
					ModelNodeSkinned* pSkinned = static_cast<ModelNodeSkinned*>(pNode);
					uint32_t nBoneCount = pSkinned->GetBoneCount();

					file << nBoneCount;
					
					for (uint32_t j = 0; j < nBoneCount; ++j)
					{
						file << pSkinned->GetBoneName(j).c_str();
					}
				}
			}

			// Skeleton
			ISkeleton* pSkeleton = pModel->GetSkeleton();
			if (pSkeleton != nullptr)
			{
				file << true;

				const uint32_t nBoneCount = pSkeleton->GetBoneCount();

				file << nBoneCount;

				for (uint32_t i = 0; i < nBoneCount; ++i)
				{
					ISkeleton::IBone* pBone = pSkeleton->GetBone(i);

					file << pBone->GetName().c_str();

					if (pBone->GetParentIndex() != ISkeleton::eInvalidBoneIndex)
					{
						ISkeleton::IBone* pParentBone = pSkeleton->GetBone(pBone->GetParentIndex());
						file << pParentBone->GetName().c_str();
					}
					else
					{
						file << "NoParent";
					}

					file.Write(&pBone->GetMotionOffsetMatrix()._11, 16);
					file.Write(&pBone->GetDefaultMotionData()._11, 16);
				}
			}
			else
			{
				file << false;
			}

			return true;
		}
	}
}