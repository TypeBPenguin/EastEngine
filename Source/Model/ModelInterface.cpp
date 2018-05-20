#include "stdafx.h"
#include "ModelInterface.h"

#include "CommonLib/FileUtil.h"
#include "CommonLib/FileStream.h"

#include "ModelManager.h"

#include "Model.h"
#include "ModelInstance.h"

#include "ModelNodeSkinned.h"

#include "Skeleton.h"

#include "Motion.h"
#include "MotionManager.h"
#include "MotionSystem.h"

#include "FbxImporter.h"
#include "XpsImporter.h"

namespace eastengine
{
	namespace graphics
	{
		LODReductionRate::LODReductionRate()
		{
			fLv[0] = 1.f;
			fLv[1] = 0.775f;
			fLv[2] = 0.55f;
			fLv[3] = 0.325f;
			fLv[4] = 0.1f;
		}

		LODReductionRate::LODReductionRate(float fLv0, float fLv1, float fLv2, float fLv3, float fLv4)
		{
			fLv[0] = fLv0;
			fLv[1] = fLv1;
			fLv[2] = fLv2;
			fLv[3] = fLv3;
			fLv[4] = fLv4;
		}

		IMotion* IMotion::Create(const MotionLoader& loader)
		{
			String::StringID strKey = loader.GetFilePath().c_str();

			IMotion* pIMotion = MotionManager::GetInstance()->GetMotion(strKey);
			if (pIMotion != nullptr)
				return pIMotion;

			Motion* pMotion = static_cast<Motion*>(MotionManager::GetInstance()->AllocateMotion(strKey));
			pMotion->SetName(loader.GetName());
			pMotion->SetFilePath(loader.GetFilePath());

			switch (loader.GetLoadMotionType())
			{
			case EmMotionLoader::eFbx:
			{
				String::StringID strMotionName = file::GetFileNameWithoutExtension(loader.GetFilePath().c_str()).c_str();

				if (FBXImport::GetInstance()->LoadMotion(pMotion, loader.GetFilePath().c_str(), loader.GetScaleFactor()) == false)
				{
					pMotion->SetState(IResource::eInvalid);
					return nullptr;
				}

				pMotion->SetState(IResource::eComplete);

				return pMotion;
			}
			break;
			case EmMotionLoader::eXps:
			{
				String::StringID strMotionName = file::GetFileNameWithoutExtension(loader.GetFilePath().c_str()).c_str();

				if (XPSImport::LoadMotion(pMotion, loader.GetFilePath().c_str()) == false)
				{
					pMotion->SetState(IResource::eInvalid);
					return nullptr;
				}

				pMotion->SetState(IResource::eComplete);

				return pMotion;
			}
			break;
			case EmMotionLoader::eEast:
			{
				file::Stream file;
				if (file.Open(loader.GetFilePath().c_str(), file::eRead | file::eBinary) == false)
				{
					LOG_WARNING("Can't open to file : %s", loader.GetFilePath().c_str());
					return false;
				}

				std::string strBuf;
				file >> strBuf;

				uint32_t nBoneCount = 0;
				file >> nBoneCount;

				for (uint32_t i = 0; i < nBoneCount; ++i)
				{
					file >> strBuf;

					uint32_t nKeyframe = 0;
					file >> nKeyframe;

					std::vector<Keyframe> vecKeyframes;
					vecKeyframes.resize(nKeyframe);

					for (uint32_t j = 0; j < nKeyframe; ++j)
					{
						file >> vecKeyframes[j].fTime;
						file.Read(&vecKeyframes[j].transform.position.x, 3);
						file.Read(&vecKeyframes[j].transform.scale.x, 3);
						file.Read(&vecKeyframes[j].transform.rotation.x, 4);
					}

					pMotion->AddBoneKeyframes(strBuf.c_str(), vecKeyframes);
				}

				pMotion->SetState(IResource::eComplete);

				file.Close();
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

		bool IMotion::SaveToFile(IMotion* pMotion, const char* strFilePath)
		{
			file::Stream file;
			if (file.Open(strFilePath, file::eWrite | file::eBinary) == false)
			{
				LOG_WARNING("Can't save to file : %s", strFilePath);
				return false;
			}

			file << pMotion->GetName().c_str();

			uint32_t nBoneCount = pMotion->GetBoneCount();

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
					file.Write(&pKeyframe->transform.position.x, 3);
					file.Write(&pKeyframe->transform.scale.x, 3);
					file.Write(&pKeyframe->transform.rotation.x, 4);
				}
			}

			file.Close();

			return true;
		}

		IModel* IModel::Create(const ModelLoader& loader, bool isThreadLoad)
		{
			String::StringID strKey;
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

		bool IModel::SaveToFile(IModel* pModel, const char* strFilePath)
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
			if (file.Open(strFilePath, file::eWrite | file::eBinary) == false)
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

				if (pNode->GetParentNode() != nullptr)
				{
					file << pNode->GetParentNode()->GetName().c_str();
				}
				else
				{
					file << "NoParent";
				}

				if (pNode->GetAttachedBoneName().empty() == false)
				{
					file << pNode->GetAttachedBoneName().c_str();
				}
				else
				{
					file << "None";
				}

				const Collision::AABB& aabb = pNode->GetOriginAABB();
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

				if (pNode->GetType() == EmModelNode::eStatic)
				{
					const VertexPosTexNor* pVertices = reinterpret_cast<const VertexPosTexNor*>(pData);
					for (uint32_t j = 0; j < nVertexCount; ++j)
					{
						file.Write(&pVertices[j].pos.x, 3);
						file.Write(&pVertices[j].uv.x, 2);
						file.Write(&pVertices[j].normal.x, 3);
					}
				}
				else if (pNode->GetType() == EmModelNode::eSkinned)
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

					pMaterial->SaveToFile(strPath.c_str());
				}

				if (pNode->GetType() == EmModelNode::eSkinned)
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