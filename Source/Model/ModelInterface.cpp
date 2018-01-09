#include "stdafx.h"
#include "ModelInterface.h"

#include "CommonLib/FileUtil.h"
#include "CommonLib/FileStream.h"

#include "ModelManager.h"
#include "Model.h"

#include "ModelNodeSkinned.h"

#include "Skeleton.h"

#include "Motion.h"
#include "MotionManager.h"
#include "MotionSystem.h"

#include "FbxImporter.h"

namespace EastEngine
{
	namespace Graphics
	{
		static boost::object_pool<Motion> s_poolMotion;
		static boost::object_pool<MotionSystem> s_poolMotionSystem;

		static boost::object_pool<Skeleton> s_poolSkeleton;

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
			IMotion* pIMotion = MotionManager::GetInstance()->GetMotion(loader.GetName());
			if (pIMotion != nullptr)
				return pIMotion;

			switch (loader.GetLoadMotionType())
			{
			case EmMotionLoader::eFbx:
			{
				String::StringID strMotionName = File::GetFileNameWithoutExtension(loader.GetFilePath().c_str()).c_str();

				Motion* pMotion = s_poolMotion.construct(strMotionName, loader.GetFilePath().c_str());

				if (FBXImport::GetInstance()->LoadMotion(pMotion, loader.GetFilePath().c_str(), loader.GetScaleFactor()) == false)
				{
					pMotion->SetLoadState(EmLoadState::eInvalid);
					return nullptr;
				}

				pMotion->SetLoadState(EmLoadState::eComplete);

				MotionManager::GetInstance()->AddMotion(loader.GetName(), pMotion);

				return pMotion;
			}
			break;
			case EmMotionLoader::eEast:
			{
				File::FileStream file;
				if (file.Open(loader.GetFilePath().c_str(), File::EmState::eRead | File::EmState::eBinary) == false)
				{
					LOG_WARNING("Can't open to file : %s", loader.GetFilePath().c_str());
					return false;
				}

				std::string strBuf;
				file >> strBuf;

				Motion* pMotion = s_poolMotion.construct(strBuf.c_str(), loader.GetFilePath().c_str());

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
						file.Read(&vecKeyframes[j].f3Pos.x, 3);
						file.Read(&vecKeyframes[j].f3Scale.x, 3);
						file.Read(&vecKeyframes[j].quatRotation.x, 4);
					}

					pMotion->AddBoneKeyframes(strBuf.c_str(), vecKeyframes);
				}

				pMotion->SetLoadState(EmLoadState::eComplete);

				MotionManager::GetInstance()->AddMotion(loader.GetName(), pMotion);

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

			Motion* pMotion = static_cast<Motion*>(*ppMotion);
			s_poolMotion.destroy(pMotion);

			*ppMotion = nullptr;
		}

		bool IMotion::SaveToFile(IMotion* pMotion, const char* strFilePath)
		{
			File::FileStream file;
			if (file.Open(strFilePath, File::EmState::eWrite | File::EmState::eBinary) == false)
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
					file.Write(&pKeyframe->f3Pos.x, 3);
					file.Write(&pKeyframe->f3Scale.x, 3);
					file.Write(&pKeyframe->quatRotation.x, 4);
				}
			}

			file.Close();

			return true;
		}

		IMotionSystem* IMotionSystem::Create(ISkeletonInstance* pSkeletonInstance)
		{
			if (pSkeletonInstance == nullptr)
				return nullptr;

			return s_poolMotionSystem.construct(pSkeletonInstance);
		}

		void IMotionSystem::Destroy(IMotionSystem** ppMotionSystem)
		{
			if (ppMotionSystem == nullptr || *ppMotionSystem == nullptr)
				return;

			MotionSystem* pMotionSystem = static_cast<MotionSystem*>(*ppMotionSystem);

			s_poolMotionSystem.destroy(pMotionSystem);
			*ppMotionSystem = nullptr;
		}

		IModel* IModel::Create(const ModelLoader& loader, bool isThreadLoad, size_t nReserveInstance)
		{
			IModel* pIModel = ModelManager::GetInstance()->GetModel(loader.GetModelName());
			if (pIModel != nullptr)
				return pIModel;

			Model* pModel = static_cast<Model*>(ModelManager::GetInstance()->AllocateModel(nReserveInstance));
			pModel->SetName(loader.GetModelName());
			pModel->SetFilePath(loader.GetFilePath());
			pModel->SetLocalPosition(loader.GetLocalPosition());
			pModel->SetLocalRotation(loader.GetLocalRotation());
			pModel->SetLocalScale(loader.GetLocalScale());
			pModel->SetLoadState(EmLoadState::eReady);

			if (isThreadLoad == true)
			{
				ModelManager::GetInstance()->LoadModelSync(pModel, loader);
			}
			else
			{
				if (pModel->Load(loader) == true)
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

			return pModel;
		}

		void IModel::Destroy(IModel** ppModel)
		{
			if (ppModel == nullptr || *ppModel == nullptr)
				return;

			ModelManager::GetInstance()->DestroyModel(ppModel);
		}

		IModelInstance* IModel::CreateInstance(const ModelLoader& loader, bool isThreadLoad)
		{
			Model* pModel = static_cast<Model*>(IModel::Create(loader, isThreadLoad));
			if (pModel == nullptr)
				return nullptr;

			return pModel->CreateInstance();
		}

		IModelInstance* IModel::CreateInstance(IModel* pModel)
		{
			if (pModel == nullptr)
				return nullptr;

			Model* pRealModel = static_cast<Model*>(pModel);
			return pRealModel->CreateInstance();
		}

		void IModel::DestroyInstance(IModelInstance** ppModelInst)
		{
			if (ppModelInst == nullptr || *ppModelInst == nullptr)
				return;

			Model* pModel = static_cast<Model*>((*ppModelInst)->GetModel());
			if (pModel == nullptr)
				return;

			pModel->DestroyInstance(ppModelInst);
		}

		bool IModel::SaveToFile(IModel* pModel, const char* strFilePath)
		{
			if (strFilePath == nullptr)
				return false;

			Model* pRealModel = static_cast<Model*>(pModel);
			if (pRealModel == nullptr)
				return false;

			// 좀 더 구조적으로 쉽고 간편한 Save Load 방식이 필요함
			// FileStream 은 빨라서 좋지만, 데이터 규격이 달라지면 기존 데이터를 사용할 수 없게됨
			// 또는 확실한 버전 관리로, 버전별 Save Load 로직을 구별한다면 FileStream 으로도 문제없음
			File::FileStream file;
			if (file.Open(strFilePath, File::EmState::eWrite | File::EmState::eBinary) == false)
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
					ModelSubset* pModelSubset = pNode->GetModelSubset(j);
					file << pModelSubset->strName.c_str();
					file << pModelSubset->nStartIndex;
					file << pModelSubset->nIndexCount;
					file << pModelSubset->nMaterialID;
					file << pModelSubset->emPrimitiveType;
				}

				IVertexBuffer* pVertexBuffer = pNode->GetVertexBuffer();

				void* pData = nullptr;
				if (pVertexBuffer->Map(0, D3D11_MAP_WRITE_NO_OVERWRITE, &pData) == false)
				{
					LOG_ERROR("Can't map vertexbuffer");
					return false;
				}

				uint32_t nVertexCount = pVertexBuffer->GetVertexNum();
				file << nVertexCount;

				if (pNode->GetType() == EmModelNode::eStatic)
				{
					VertexPosTexNor* pVertices = reinterpret_cast<VertexPosTexNor*>(pData);
					for (uint32_t j = 0; j < nVertexCount; ++j)
					{
						file.Write(&pVertices[j].pos.x, 3);
						file.Write(&pVertices[j].uv.x, 2);
						file.Write(&pVertices[j].normal.x, 3);
					}
				}
				else if (pNode->GetType() == EmModelNode::eSkinned)
				{
					VertexPosTexNorWeiIdx* pVertices = reinterpret_cast<VertexPosTexNorWeiIdx*>(pData);
					for (uint32_t j = 0; j < nVertexCount; ++j)
					{
						file.Write(&pVertices[j].pos.x, 3);
						file.Write(&pVertices[j].uv.x, 2);
						file.Write(&pVertices[j].normal.x, 3);
						file.Write(&pVertices[j].boneWeight.x, 3);
						file.Write(&pVertices[j].boneIndices[0], 4);
					}
				}

				pVertexBuffer->Unmap(0);

				IIndexBuffer* pIndexBuffer = pNode->GetIndexBuffer();

				pData = nullptr;
				if (pIndexBuffer->Map(0, D3D11_MAP_WRITE_NO_OVERWRITE, &pData) == false)
				{
					LOG_ERROR("Can't map indexbuffer");
					return false;
				}

				uint32_t nIndexCount = pIndexBuffer->GetIndexNum();
				file << nIndexCount;
				uint32_t* pIndices = reinterpret_cast<uint32_t*>(pData);
				for (uint32_t j = 0; j < nIndexCount; ++j)
				{
					file << pIndices[j];
				}

				pIndexBuffer->Unmap(0);

				uint32_t nMaterialCount = pNode->GetMaterialCount();
				file << nMaterialCount;

				std::string strPath = File::GetFilePath(strFilePath);
				for (uint32_t j = 0; j < nMaterialCount; ++j)
				{
					IMaterial* pMaterial = pNode->GetMaterial(j);
					file << pMaterial->GetName().c_str();

					IMaterial::SaveToFile(pMaterial, strPath.c_str());
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

				file << pSkeleton->GetBoneCount();

				ISkeleton::IBone* pRootBone = pSkeleton->GetRootBone();

				std::function<void(ISkeleton::IBone*)> WriteBone = [&](ISkeleton::IBone* pBone)
				{
					file << pBone->GetName().c_str();

					if (pBone->GetParent() != nullptr && pBone->GetParent()->IsRootBone() == false)
					{
						file << pBone->GetParent()->GetName().c_str();
					}
					else
					{
						file << "NoParent";
					}

					file.Write(&pBone->GetMotionOffsetMatrix()._11, 16);
					file.Write(&pBone->GetDefaultMotionData()._11, 16);

					uint32_t nChildBoneCount = pBone->GetChildBoneCount();
					for (uint32_t i = 0; i < nChildBoneCount; ++i)
					{
						ISkeleton::IBone* pChildBone = pBone->GetChildBone(i);
						WriteBone(pChildBone);
					}
				};

				uint32_t nChildBoneCount = pRootBone->GetChildBoneCount();
				for (uint32_t i = 0; i < nChildBoneCount; ++i)
				{
					ISkeleton::IBone* pChildBone = pRootBone->GetChildBone(i);
					WriteBone(pChildBone);
				}
			}
			else
			{
				file << false;
			}

			return true;
		}

		ISkeleton* ISkeleton::Create()
		{
			return s_poolSkeleton.construct();
		}

		void ISkeleton::Destroy(ISkeleton** ppSkeleton)
		{
			if (ppSkeleton == nullptr || *ppSkeleton == nullptr)
				return;

			Skeleton* pSkeleton = static_cast<Skeleton*>(*ppSkeleton);
			s_poolSkeleton.destroy(pSkeleton);

			*ppSkeleton = nullptr;
		}

		ISkeletonInstance* ISkeleton::CreateInstance(ISkeleton* pSkeleton)
		{
			if (pSkeleton == nullptr)
				return nullptr;

			Skeleton* pRealSkeleton = static_cast<Skeleton*>(pSkeleton);

			return pRealSkeleton->CreateInstance();
		}

		void ISkeleton::DestroyInstance(ISkeletonInstance** ppSkeletonInstance)
		{
			if (ppSkeletonInstance == nullptr || *ppSkeletonInstance == nullptr)
				return;

			Skeleton* pRealSkeleton = static_cast<Skeleton*>((*ppSkeletonInstance)->GetSkeleton());
			pRealSkeleton->DestroyInstance(ppSkeletonInstance);
		}
	}
}