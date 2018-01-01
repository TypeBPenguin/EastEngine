#include "stdafx.h"
#include "Model.h"

#include "CommonLib/FileStream.h"
#include "CommonLib/FileUtil.h"

#include "ModelManager.h"

#include "ModelNodeStatic.h"
#include "ModelNodeSkinned.h"

#include "ModelInstance.h"

#include "GeometryModel.h"

#include "FbxImporter.h"
#include "ObjImporter.h"

#include "Skeleton.h"

namespace StrID
{
	RegisterStringID(EastEngine_CustomStaticModel);
	RegisterStringID(EastEngine_Cube);
	RegisterStringID(EastEngine_Box);
	RegisterStringID(EastEngine_Sphere);
	RegisterStringID(EastEngine_GeoSphere);
	RegisterStringID(EastEngine_Cylinder);
	RegisterStringID(EastEngine_Cone);
	RegisterStringID(EastEngine_Torus);
	RegisterStringID(EastEngine_Tetrahedron);
	RegisterStringID(EastEngine_Octahedron);
	RegisterStringID(EastEngine_Dodecahedron);
	RegisterStringID(EastEngine_Icosahedron);
	RegisterStringID(EastEngine_Teapot);
	RegisterStringID(EastEngine_Hexagon);
	RegisterStringID(EastEngine_Capsule);
	RegisterStringID(EastEngine_Grid);
	RegisterStringID(EastEngine_Plane);
};

namespace EastEngine
{
	namespace Graphics
	{
		const String::StringID& GetGeometryTypeName(EmModelLoader::GeometryType emGeometryType)
		{
			static String::StringID strGeometryType[] =
			{
				StrID::EastEngine_CustomStaticModel,
				StrID::EastEngine_Cube,
				StrID::EastEngine_Box,
				StrID::EastEngine_Sphere,
				StrID::EastEngine_GeoSphere,
				StrID::EastEngine_Cylinder,
				StrID::EastEngine_Cone,
				StrID::EastEngine_Torus,
				StrID::EastEngine_Tetrahedron,
				StrID::EastEngine_Octahedron,
				StrID::EastEngine_Dodecahedron,
				StrID::EastEngine_Icosahedron,
				StrID::EastEngine_Teapot,
				StrID::EastEngine_Hexagon,
				StrID::EastEngine_Capsule,
				StrID::EastEngine_Grid,
				StrID::EastEngine_Plane,
			};

			return strGeometryType[emGeometryType];
		}

		Model::Model(uint32_t nReserveInstance)
			: m_isVisible(true)
			, m_isDirtyLocalMatrix(false)
			, m_nReferenceCount(0)
			, m_pSkeleton(nullptr)
			, m_f3Scale(Math::Vector3::One)
		{
			m_clnModelInstance.reserve(nReserveInstance);
		}

		Model::~Model()
		{
			m_clnModelInstance.clear();
			ISkeleton::Destroy(&m_pSkeleton);

			std::for_each(m_vecHierarchyModelNodes.begin(), m_vecHierarchyModelNodes.end(), [](IModelNode* pNode)
			{
				ModelNode* pModelNode = static_cast<ModelNode*>(pNode);
				SafeDelete(pModelNode);
			});
			m_vecHierarchyModelNodes.clear();

			m_vecModelNodes.clear();
		}

		IModelInstance* Model::CreateInstance()
		{
			auto iter = m_clnModelInstance.emplace(this);
			if (iter == m_clnModelInstance.end())
				return nullptr;

			ModelInstance* pModelInstance = &(*iter);
			IncreaseReference();

			if (GetLoadState() == EmLoadState::eComplete)
			{
				pModelInstance->LoadCompleteCallback(true);
			}
			else if (GetLoadState() == EmLoadState::eInvalid)
			{
				pModelInstance->LoadCompleteCallback(false);
			}

			return pModelInstance;
		}

		void Model::DestroyInstance(IModelInstance** ppModelInstance)
		{
			if (ppModelInstance == nullptr || *ppModelInstance == nullptr)
				return;

			auto iter = std::find_if(m_clnModelInstance.begin(), m_clnModelInstance.end(), [ppModelInstance](ModelInstance& instance)
			{
				return &instance == *ppModelInstance;
			});

			if (iter == m_clnModelInstance.end())
				return;

			m_clnModelInstance.erase(iter);
			*ppModelInstance = nullptr;

			DecreaseReference();
		}

		void Model::Process()
		{
			std::for_each(m_clnModelInstance.begin(), m_clnModelInstance.end(), [](ModelInstance& instance)
			{
				if (instance.IsAttachment() == false)
				{
					instance.Process();
				}
			});
		}
		
		void Model::Update(float fElapsedTime, const Math::Matrix& matParent, ISkeletonInstance* pSkeletonInstance, IMaterialInstance* pMaterialInstance)
		{
			if (m_isDirtyLocalMatrix == true)
			{
				m_matLocal = Math::Matrix::Compose(m_f3Scale, m_quat, m_f3Pos);
				m_isDirtyLocalMatrix = false;
			}

			Math::Matrix matModel = m_matLocal * matParent;

			std::for_each(m_vecHierarchyModelNodes.begin(), m_vecHierarchyModelNodes.end(), [&](IModelNode* pModelNode)
			{
				pModelNode->Update(fElapsedTime, matModel, pSkeletonInstance, pMaterialInstance, m_isVisible);
			});
		}

		IModelNode* Model::GetNode(const String::StringID& strName) const
		{
			auto iter = std::find_if(m_vecModelNodes.begin(), m_vecModelNodes.end(), [&](IModelNode* pNode)
			{
				return pNode->GetName() == strName;
			});

			if (iter != m_vecModelNodes.end())
				return *iter;

			return nullptr;
		}

		void Model::LoadCompleteCallback(bool isSuccess)
		{
			SetLoadState(isSuccess == true ? EmLoadState::eComplete : EmLoadState::eInvalid);

			std::for_each(m_clnModelInstance.begin(), m_clnModelInstance.end(), [isSuccess](ModelInstance& instance)
			{
				instance.LoadCompleteCallback(isSuccess);
			});
		}

		void Model::ChangeName(const String::StringID& strName)
		{
			if (ModelManager::GetInstance()->ChangeName(this, strName) == true)
			{
				SetName(strName);
			}
		}

		void Model::AddNode(IModelNode* pNode, const String::StringID& strNodeName, bool isRootNode)
		{
			if (isRootNode == true)
			{
				ModelNode* pModelNode = static_cast<ModelNode*>(pNode);
				pModelNode->SetNodeName(strNodeName);
				m_vecHierarchyModelNodes.emplace_back(pNode);
			}

			if (GetNode(strNodeName) == nullptr)
			{
				m_vecModelNodes.emplace_back(pNode);
			}
		}

		bool Model::Load(const ModelLoader& loader)
		{
			static std::mutex mutex;

			bool isSuccess = false;

			switch (loader.GetLoadModelType())
			{
			case EmModelLoader::eFbx:
			{
				std::unique_lock<std::mutex> lock(mutex);

				isSuccess = FBXImport::GetInstance()->LoadModel(this, loader.GetFilePath().c_str(), loader.GetScaleFactor());
				if (isSuccess == false)
				{
					PRINT_LOG("Can't load Model[FBX] : %s", loader.GetFilePath().c_str());
				}
			}
			break;
			case EmModelLoader::eObj:
			{
				std::unique_lock<std::mutex> lock(mutex);

				isSuccess = FBXImport::GetInstance()->LoadModel(this, loader.GetFilePath().c_str(), loader.GetScaleFactor());
				//isSuccess = SObjImporter::GetInstance()->LoadModel(this, loader.GetFilePath().c_str(), loader.GetScaleFactor(), loader.GetLodMax(), &loader.GetLODReductionRate());
				if (isSuccess == false)
				{
					PRINT_LOG("Can't load Model[Obj] : %s", loader.GetFilePath().c_str());
				}
			}
			break;
			case EmModelLoader::eEast:
			{
				isSuccess = LoadToFile(loader.GetFilePath().c_str());
				if (isSuccess == false)
				{
					PRINT_LOG("Can't load Model[East] : %s", loader.GetFilePath().c_str());
				}
			}
			break;
			case EmModelLoader::eGeometry:
			{
				IVertexBuffer* pVertexBuffer = nullptr;
				IIndexBuffer* pIndexBuffer = nullptr;

				Collision::AABB aabb;

				auto SetModelNode = [&](const String::StringID& strNodeName, const Collision::AABB& aabb)
				{
					ModelNodeStatic* pModelStatic = new ModelNodeStatic;
					pModelStatic->SetVertexBuffer(pVertexBuffer);
					pModelStatic->SetIndexBuffer(pIndexBuffer);
					pModelStatic->BuildBoundingBox(aabb);

					IMaterial* pMaterial = IMaterial::Create(&loader.GetMaterial());
					pModelStatic->AddMaterial(pMaterial);

					ModelSubset modelSubset;
					modelSubset.nMaterialID = 0;
					modelSubset.nStartIndex = 0;
					modelSubset.nIndexCount = pIndexBuffer->GetIndexNum();
					pModelStatic->AddModelSubset(modelSubset);

					this->AddNode(pModelStatic, strNodeName, true);
				};

				switch (loader.GetLoadGeometryType())
				{
				case EmModelLoader::eCube:
				{
					const LoadInfoCube* pLoadInfo = loader.GetCubeInfo();
					if (pLoadInfo != nullptr)
					{
						isSuccess = GeometryModel::CreateCube(&pVertexBuffer, &pIndexBuffer, pLoadInfo->fSize, loader.IsRightHandCoords());

						aabb.Extents = Math::Vector3(pLoadInfo->fSize);
					}
				}
				break;
				case EmModelLoader::eBox:
				{
					const LoadInfoBox* pLoadInfo = loader.GetBoxInfo();
					if (pLoadInfo != nullptr)
					{
						isSuccess = GeometryModel::CreateBox(&pVertexBuffer, &pIndexBuffer, pLoadInfo->f3Size, loader.IsRightHandCoords(), loader.IsInvertNormal());

						aabb.Extents = pLoadInfo->f3Size;
					}
				}
				break;
				case EmModelLoader::eSphere:
				{
					const LoadInfoSphere* pLoadInfo = loader.GetSphereInfo();
					if (pLoadInfo != nullptr)
					{
						isSuccess = GeometryModel::CreateSphere(&pVertexBuffer, &pIndexBuffer, pLoadInfo->fDiameter, pLoadInfo->nTessellation, loader.IsRightHandCoords(), loader.IsInvertNormal());

						aabb.Extents = Math::Vector3(pLoadInfo->fDiameter * 0.5f);
					}
				}
				break;
				case EmModelLoader::eGeoSphere:
				{
					const LoadInfoGeoSphere* pLoadInfo = loader.GetGeoSphereInfo();
					if (pLoadInfo != nullptr)
					{
						isSuccess = GeometryModel::CreateGeoSphere(&pVertexBuffer, &pIndexBuffer, pLoadInfo->fDiameter, pLoadInfo->nTessellation, loader.IsRightHandCoords());

						aabb.Extents = Math::Vector3(pLoadInfo->fDiameter * 0.5f);
					}
				}
				break;
				case EmModelLoader::eCylinder:
				{
					const LoadInfoCylinder* pLoadInfo = loader.GetCylinderInfo();
					if (pLoadInfo != nullptr)
					{
						isSuccess = GeometryModel::CreateCylinder(&pVertexBuffer, &pIndexBuffer, pLoadInfo->fHeight, pLoadInfo->fDiameter, pLoadInfo->nTessellation, loader.IsRightHandCoords());

						aabb.Extents = Math::Vector3(pLoadInfo->fDiameter * 0.5f, pLoadInfo->fHeight, pLoadInfo->fDiameter * 0.5f);
					}
				}
				break;
				case EmModelLoader::eCone:
				{
					const LoadInfoCone* pLoadInfo = loader.GetConeInfo();
					if (pLoadInfo != nullptr)
					{
						isSuccess = GeometryModel::CreateCone(&pVertexBuffer, &pIndexBuffer, pLoadInfo->fDiameter, pLoadInfo->fHeight, pLoadInfo->nTessellation, loader.IsRightHandCoords());

						aabb.Extents = Math::Vector3(pLoadInfo->fDiameter * 0.5f, pLoadInfo->fHeight, pLoadInfo->fDiameter * 0.5f);
					}
				}
				break;
				case EmModelLoader::eTorus:
				{
					const LoadInfoTorus* pLoadInfo = loader.GetTorusInfo();
					if (pLoadInfo != nullptr)
					{
						isSuccess = GeometryModel::CreateTorus(&pVertexBuffer, &pIndexBuffer, pLoadInfo->fDiameter, pLoadInfo->fThickness, pLoadInfo->nTessellation, loader.IsRightHandCoords());

						aabb.Extents = Math::Vector3(pLoadInfo->fDiameter * 0.5f, pLoadInfo->fThickness, pLoadInfo->fDiameter * 0.5f);
					}
				}
				break;
				case EmModelLoader::eTetrahedron:
				{
					const LoadInfoTetrahedron* pLoadInfo = loader.GetTetrahedronInfo();
					if (pLoadInfo != nullptr)
					{
						isSuccess = GeometryModel::CreateTetrahedron(&pVertexBuffer, &pIndexBuffer, pLoadInfo->fSize, loader.IsRightHandCoords());

						aabb.Extents = Math::Vector3(pLoadInfo->fSize);
					}
				}
				break;
				case EmModelLoader::eOctahedron:
				{
					const LoadInfoOctahedron* pLoadInfo = loader.GetOctahedronInfo();
					if (pLoadInfo != nullptr)
					{
						isSuccess = GeometryModel::CreateOctahedron(&pVertexBuffer, &pIndexBuffer, pLoadInfo->fSize, loader.IsRightHandCoords());

						aabb.Extents = Math::Vector3(pLoadInfo->fSize);
					}
				}
				break;
				case EmModelLoader::eDodecahedron:
				{
					const LoadInfoDodecahedron* pLoadInfo = loader.GetDodecahedronInfo();
					if (pLoadInfo != nullptr)
					{
						isSuccess = GeometryModel::CreateDodecahedron(&pVertexBuffer, &pIndexBuffer, pLoadInfo->fSize, loader.IsRightHandCoords());

						aabb.Extents = Math::Vector3(pLoadInfo->fSize);
					}
				}
				break;
				case EmModelLoader::eIcosahedron:
				{
					const LoadInfoIcosahedron* pLoadInfo = loader.GetIcosahedronInfo();
					if (pLoadInfo != nullptr)
					{
						isSuccess = GeometryModel::CreateIcosahedron(&pVertexBuffer, &pIndexBuffer, pLoadInfo->fSize, loader.IsRightHandCoords());

						aabb.Extents = Math::Vector3(pLoadInfo->fSize);
					}
				}
				break;
				case EmModelLoader::eTeapot:
				{
					const LoadInfoTeapot* pLoadInfo = loader.GetTeapotInfo();
					if (pLoadInfo != nullptr)
					{
						isSuccess = GeometryModel::CreateTeapot(&pVertexBuffer, &pIndexBuffer, pLoadInfo->fSize, pLoadInfo->nTessellation, loader.IsRightHandCoords());

						aabb.Extents = Math::Vector3(pLoadInfo->fSize);
					}
				}
				break;
				case EmModelLoader::eHexagon:
				{
					const LoadInfoHexagon* pLoadInfo = loader.GetHexagonInfo();
					if (pLoadInfo != nullptr)
					{
						isSuccess = GeometryModel::CreateHexagon(&pVertexBuffer, &pIndexBuffer, pLoadInfo->fRadius, loader.IsRightHandCoords());

						aabb.Extents = Math::Vector3(pLoadInfo->fRadius);
					}
				}
				break;
				case EmModelLoader::eCapsule:
				{
					const LoadInfoCapsule* pLoadInfo = loader.GetCapsuleInfo();
					if (pLoadInfo != nullptr)
					{
						isSuccess = GeometryModel::CreateCapsule(&pVertexBuffer, &pIndexBuffer, pLoadInfo->fRadius, pLoadInfo->fHeight, pLoadInfo->nSubdivisionHeight, pLoadInfo->nSegments);

						aabb.Extents = Math::Vector3(pLoadInfo->fRadius, pLoadInfo->fHeight, pLoadInfo->fRadius);
					}
				}
				break;
				case EmModelLoader::eGrid:
				{
					const LoadInfoGrid* pLoadInfo = loader.GetGridInfo();
					if (pLoadInfo != nullptr)
					{
						isSuccess = GeometryModel::CreateGrid(&pVertexBuffer, &pIndexBuffer, pLoadInfo->fGridSizeX, pLoadInfo->fGridSizeZ, pLoadInfo->nBlockCountWidth, pLoadInfo->nBlockCountLength);

						aabb.Extents = Math::Vector3(pLoadInfo->fGridSizeX, 0.1f, pLoadInfo->fGridSizeZ);
					}
				}
				break;
				case EmModelLoader::ePlane:
				{
					const LoadInfoPlane* pLoadInfo = loader.GetPlaneInfo();
					if (pLoadInfo != nullptr)
					{
						isSuccess = GeometryModel::CreatePlane(&pVertexBuffer, &pIndexBuffer, pLoadInfo->fEachLengthX, pLoadInfo->fEachLengthZ, pLoadInfo->nTotalCountX, pLoadInfo->nTotalCountZ);

						aabb.Extents = Math::Vector3(pLoadInfo->fEachLengthX * pLoadInfo->nTotalCountX, 0.1f, pLoadInfo->fEachLengthZ * pLoadInfo->nTotalCountZ);
					}
				}
				break;
				case EmModelLoader::eCustomStaticModel:
					break;
				}

				if (isSuccess == true)
				{
					SetModelNode(GetGeometryTypeName(loader.GetLoadGeometryType()), aabb);
				}
				else
				{
					PRINT_LOG("Can't load Model, GeometryType : %s", GetGeometryTypeName(loader.GetLoadGeometryType()).c_str());
				}
			}
			break;
			default:
				return false;
			}

			return isSuccess;
		}

		bool Model::LoadToFile(const char* strFilePath)
		{
			// 좀 더 구조적으로 쉽고 간편한 Save Load 방식이 필요함
			// FileStream 은 빨라서 좋지만, 데이터 규격이 달라지면 기존 데이터를 사용할 수 없게됨
			// 또는 확실한 버전 관리로, 버전별 Save Load 로직을 구현한다면 FileStream 으로도 문제없음
			File::FileStream file;
			if (file.Open(strFilePath, File::EmState::eRead | File::EmState::eBinary) == false)
			{
				PRINT_LOG("Can't open to file : %s", strFilePath);
				return false;
			}

			// Common
			std::string strBuf;
			Math::Vector3 f3Buf;
			Math::Quaternion quatBuf;
			Math::Matrix matBuf;

			file >> strBuf;
			SetName(strBuf.c_str());

			file.Read(&f3Buf.x, 3);
			SetLocalPosition(f3Buf);

			file.Read(&f3Buf.x, 3);
			SetLocalScale(f3Buf);

			file.Read(&quatBuf.x, 4);
			SetLocalRotation(quatBuf);

			// Node
			uint32_t nNodeCount = 0;
			file >> nNodeCount;

			std::map<std::string, std::vector<IModelNode*>> mapChildNodes;

			for (uint32_t i = 0; i < nNodeCount; ++i)
			{
				ModelNode* pNode = nullptr;

				int nType = 0;
				file >> nType;

				switch (nType)
				{
				case EmModelNode::eStatic:
					pNode = new ModelNodeStatic;
					break;
				case EmModelNode::eSkinned:
					pNode = new ModelNodeSkinned;
					break;
				default:
					PRINT_LOG("잘못된타입임돠, 데이터 포맷이 바뀐 듯 함돠.");
					break;
				}
				
				file >> strBuf;
				pNode->SetNodeName(strBuf.c_str());

				file >> strBuf;
				if (strBuf == "NoParent")
				{
					AddNode(pNode, pNode->GetName(), true);
				}
				else
				{
					auto iter = mapChildNodes.find(strBuf);
					if (iter != mapChildNodes.end())
					{
						iter->second.emplace_back(pNode);
					}
					else
					{
						auto iter_result = mapChildNodes.emplace(strBuf, std::vector<IModelNode*>());
						iter_result.first->second.emplace_back(pNode);
					}

					AddNode(pNode, pNode->GetName(), false);
				}

				bool isVisible = true;
				file >> isVisible;

				pNode->SetVisible(isVisible);

				// 쓸모없는 데이터들
				file.Read(&matBuf._11, 16);
				file.Read(&matBuf._11, 16);

				uint32_t nSubsetCount = 0;
				file >> nSubsetCount;
				
				std::vector<ModelSubset> vecSubsets;
				vecSubsets.resize(nSubsetCount);
				for (uint32_t j = 0; j < nSubsetCount; ++j)
				{
					file >> vecSubsets[j].nStartIndex;
					file >> vecSubsets[j].nIndexCount;
					file >> vecSubsets[j].nMaterialID;
				}

				pNode->AddModelSubsets(vecSubsets);

				int nVertexCount = 0;
				file >> nVertexCount;

				IVertexBuffer* pVertexBuffer = nullptr;
				if (nType == EmModelNode::eStatic)
				{
					std::vector<VertexPosTexNor> vecVertices;
					vecVertices.resize(nVertexCount);

					for (int j = 0; j < nVertexCount; ++j)
					{
						VertexPosTexNor& vertex = vecVertices[j];
						file.Read(&vertex.pos.x, 3);
						file.Read(&vertex.uv.x, 2);
						file.Read(&vertex.normal.x, 3);
					}

					pVertexBuffer = IVertexBuffer::Create(VertexPosTexNor::Format(), vecVertices.size(), &vecVertices.front(), D3D11_USAGE_DYNAMIC, IVertexBuffer::eSaveVertexPos);
					if (pVertexBuffer == nullptr)
					{
						PRINT_LOG("버텍스 버퍼 생성 실패했슴돠");
						return false;
					}
				}
				else if (nType == EmModelNode::eSkinned)
				{
					std::vector<VertexPosTexNorWeiIdx> vecVertices;
					vecVertices.resize(nVertexCount);

					for (int j = 0; j < nVertexCount; ++j)
					{
						VertexPosTexNorWeiIdx& vertex = vecVertices[j];
						file.Read(&vertex.pos.x, 3);
						file.Read(&vertex.uv.x, 2);
						file.Read(&vertex.normal.x, 3);
						file.Read(&vertex.boneWeight.x, 3);

						file >> vertex.boneIndices.v;
					}

					pVertexBuffer = IVertexBuffer::Create(VertexPosTexNorWeiIdx::Format(), vecVertices.size(), &vecVertices.front(), D3D11_USAGE_DYNAMIC, IVertexBuffer::eSaveVertexPos);
					if (pVertexBuffer == nullptr)
					{
						PRINT_LOG("버텍스 버퍼 생성 실패했슴돠");
						return false;
					}
				}

				pNode->SetVertexBuffer(pVertexBuffer);

				int nIndexCount = 0;
				file >> nIndexCount;

				std::vector<uint32_t> vecIndices;
				vecIndices.resize(nIndexCount);

				for (int j = 0; j < nIndexCount; ++j)
				{
					file >> vecIndices[j];
				}

				IIndexBuffer* pIndexBuffer = IIndexBuffer::Create(vecIndices.size(), &vecIndices.front(), D3D11_USAGE_DYNAMIC, IIndexBuffer::eSaveRawValue);
				if (pIndexBuffer == nullptr)
				{
					PRINT_LOG("인덱스 버퍼 생성 실패했슴돠");
					return false;
				}

				pNode->SetIndexBuffer(pIndexBuffer);

				uint32_t nMaterialCount = 0;
				file >> nMaterialCount;

				for (uint32_t j = 0; j < nMaterialCount; ++j)
				{
					file >> strBuf;
					
					strBuf.append(".emtl");

					pNode->AddMaterial(IMaterial::Create(strBuf.c_str(), File::GetFilePath(strFilePath).c_str()));
				}

				if (nType == EmModelNode::eSkinned)
				{
					ModelNodeSkinned* pSkinned = static_cast<ModelNodeSkinned*>(pNode);

					uint32_t nBoneCount = 0;
					file >> nBoneCount;

					std::vector<String::StringID> vecBones;
					vecBones.resize(nBoneCount);

					for (uint32_t j = 0; j < nBoneCount; ++j)
					{
						file >> strBuf;
						vecBones[j] = strBuf.c_str();
					}

					pSkinned->SetBoneNameList(vecBones);
				}
			}

			// Skeleton
			bool isHasSkeleton = false;
			file >> isHasSkeleton;

			Skeleton* pSkeleton = nullptr;
			if (isHasSkeleton == true)
			{
				pSkeleton = static_cast<Skeleton*>(ISkeleton::Create());
				SetSkeleton(pSkeleton);

				uint32_t nBoneCount = 0;
				file >> nBoneCount;
				
				for (uint32_t i = 0; i < nBoneCount; ++i)
				{
					std::string strName;
					file >> strName;

					std::string strParentName;
					file >> strParentName;

					Math::Matrix matTransformation;
					file.Read(&matTransformation._11, 16);

					Math::Matrix matMotionOffset;
					file.Read(&matMotionOffset._11, 16);

					if (strParentName == "NoParent")
					{
						pSkeleton->CreateBone(strName.c_str(), matMotionOffset, matTransformation);
					}
					else
					{
						pSkeleton->CreateBone(strParentName.c_str(), strName.c_str(), matMotionOffset, matTransformation);
					}
				}
			}

			for (const auto& pNode : m_vecModelNodes)
			{
				uint32_t nMaterialCount = pNode->GetMaterialCount();
				for (uint32_t i = 0; i < nMaterialCount; ++i)
				{
					IMaterial* pMaterial = pNode->GetMaterial(i);
					pMaterial->LoadTexture();
				}

				if (pSkeleton != nullptr)
				{
					if (pNode->GetType() != EmModelNode::eSkinned)
						continue;

					ModelNodeSkinned* pNodeSkinned = static_cast<ModelNodeSkinned*>(pNode);

					uint32_t nBoneCount = pNodeSkinned->GetBoneCount();

					std::vector<String::StringID> vecBoneNames;
					vecBoneNames.resize(nBoneCount);

					for (uint32_t j = 0; j < nBoneCount; ++j)
					{
						vecBoneNames[j] = pNodeSkinned->GetBoneName(j);
					}

					pSkeleton->SetSkinnedList(pNodeSkinned->GetName(), &vecBoneNames.front(), vecBoneNames.size());
				}
			}

			return true;
		}
	}
}