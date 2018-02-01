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
#include "XpsImporter.h"

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

		Model::Model(Key key)
			: m_key(key)
			, m_isVisible(true)
			, m_isDirtyLocalMatrix(false)
			, m_nReferenceCount(0)
			, m_f3Scale(Math::Vector3::One)
		{
		}

		Model::~Model()
		{
			m_vecModelInstances.clear();

			std::for_each(m_vecHierarchyModelNodes.begin(), m_vecHierarchyModelNodes.end(), [](IModelNode* pNode)
			{
				ModelNode* pModelNode = static_cast<ModelNode*>(pNode);
				SafeDelete(pModelNode);
			});
			m_vecHierarchyModelNodes.clear();

			m_vecModelNodes.clear();
		}

		void Model::AddInstance(ModelInstance* pModelInstance)
		{
			m_vecModelInstances.emplace_back(pModelInstance);

			IncreaseReference();

			if (GetLoadState() == EmLoadState::eComplete)
			{
				pModelInstance->LoadCompleteCallback(true);
			}
			else if (GetLoadState() == EmLoadState::eInvalid)
			{
				pModelInstance->LoadCompleteCallback(false);
			}
		}

		bool Model::RemoveInstance(ModelInstance* pModelInstance)
		{
			auto iter = std::find(m_vecModelInstances.begin(), m_vecModelInstances.end(), pModelInstance);
			if (iter == m_vecModelInstances.end())
				return false;

			m_vecModelInstances.erase(iter);

			DecreaseReference();

			return true;
		}

		bool Model::IsHasInstance() const
		{
			return m_vecModelInstances.empty() == false;
		}

		void Model::Ready()
		{
			if (GetLoadState() != EmLoadState::eComplete)
				return;

			if (m_isDirtyLocalMatrix == true)
			{
				m_matLocal = Math::Matrix::Compose(m_f3Scale, m_quat, m_f3Pos);
				m_isDirtyLocalMatrix = false;
			}
		}

		void Model::Update(float fElapsedTime, const Math::Matrix& matParent, ISkeletonInstance* pSkeletonInstance, IMaterialInstance* pMaterialInstance)
		{
			std::for_each(m_vecHierarchyModelNodes.begin(), m_vecHierarchyModelNodes.end(), [&](IModelNode* pModelNode)
			{
				pModelNode->Update(fElapsedTime, matParent, pSkeletonInstance, pMaterialInstance, m_isVisible);
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

			std::for_each(m_vecModelInstances.begin(), m_vecModelInstances.end(), [isSuccess](ModelInstance* pInstance)
			{
				pInstance->LoadCompleteCallback(isSuccess);
			});
		}

		void Model::ChangeName(const String::StringID& strName)
		{
			SetName(strName);
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
			else
			{
				assert(false);
			}
		}

		bool Model::Load(const ModelLoader& loader)
		{
			bool isSuccess = false;

			switch (loader.GetLoadModelType())
			{
			case EmModelLoader::eFbx:
			{
				isSuccess = FBXImport::GetInstance()->LoadModel(this, loader.GetFilePath().c_str(), loader.GetScaleFactor(), loader.IsFlipZ());
				if (isSuccess == false)
				{
					LOG_ERROR("Can't load Model[FBX] : %s", loader.GetFilePath().c_str());
				}
			}
			break;
			case EmModelLoader::eObj:
			{
				isSuccess = FBXImport::GetInstance()->LoadModel(this, loader.GetFilePath().c_str(), loader.GetScaleFactor(), loader.IsFlipZ());
				if (isSuccess == false)
				{
					LOG_ERROR("Can't load Model[Obj] : %s", loader.GetFilePath().c_str());
				}
			}
			break;
			case EmModelLoader::eXps:
			{
				isSuccess = XPSImport::LoadModel(this, loader.GetFilePath().c_str(), loader.GetDevideKeywords(), loader.GetDevideKeywordCount());
				if (isSuccess == false)
				{
					LOG_ERROR("Can't load Model[XPS] : %s", loader.GetFilePath().c_str());
				}
			}
			break;
			case EmModelLoader::eEast:
			{
				isSuccess = LoadToFile(loader.GetFilePath().c_str());
				if (isSuccess == false)
				{
					LOG_ERROR("Can't load Model[East] : %s", loader.GetFilePath().c_str());
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
					pModelStatic->SetOriginAABB(aabb);

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
					LOG_ERROR("Can't load Model, GeometryType : %s", GetGeometryTypeName(loader.GetLoadGeometryType()).c_str());
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
			std::string strFileExtension = File::GetFileExtension(strFilePath);
			if (strFileExtension != "emod")
			{
				LOG_ERROR("Invalid Extension, Required[%s] != Request[%s]", "emod", strFileExtension.c_str());
				return false;
			}

			// �� �� ���������� ���� ������ Save Load ����� �ʿ���
			// FileStream �� ���� ������, ������ �԰��� �޶����� ���� �����͸� ����� �� ���Ե�
			// �Ǵ� Ȯ���� ���� ������, ������ Save Load ������ �����Ѵٸ� FileStream ���ε� ��������
			File::FileStream file;
			if (file.Open(strFilePath, File::EmState::eRead | File::EmState::eBinary) == false)
			{
				LOG_WARNING("Can't open to file : %s", strFilePath);
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
					LOG_WARNING("�߸���Ÿ���ӵ�, ������ ������ �ٲ� �� �Ե�.");
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
					ModelNode* pParentNode = static_cast<ModelNode*>(GetNode(strBuf.c_str()));
					assert(pParentNode != nullptr);
					
					pParentNode->AddChildNode(pNode);
					pNode->SetParentNode(pParentNode);

					AddNode(pNode, pNode->GetName(), false);
				}

				file >> strBuf;
				if (strBuf != "None")
				{
					pNode->SetAttachedBoneName(strBuf.c_str());
				}

				Collision::AABB aabb;
				file.Read(&aabb.Center.x, 3);
				file.Read(&aabb.Extents.x, 3);
				pNode->SetOriginAABB(aabb);

				bool isVisible = true;
				file >> isVisible;

				pNode->SetVisible(isVisible);

				uint32_t nSubsetCount = 0;
				file >> nSubsetCount;
				
				std::vector<ModelSubset> vecSubsets;
				vecSubsets.resize(nSubsetCount);
				for (uint32_t j = 0; j < nSubsetCount; ++j)
				{
					file >> strBuf;
					vecSubsets[j].strName = strBuf.c_str();

					file >> vecSubsets[j].nStartIndex;
					file >> vecSubsets[j].nIndexCount;
					file >> vecSubsets[j].nMaterialID;

					int nTemp = 0;
					file >> nTemp;
					vecSubsets[j].emPrimitiveType = static_cast<EmPrimitive::Type>(nTemp);
				}

				pNode->AddModelSubsets(vecSubsets);

				uint32_t nVertexCount = 0;
				file >> nVertexCount;

				IVertexBuffer* pVertexBuffer = nullptr;
				if (nType == EmModelNode::eStatic)
				{
					std::vector<VertexPosTexNor> vecVertices;
					vecVertices.resize(nVertexCount);

					for (uint32_t j = 0; j < nVertexCount; ++j)
					{
						VertexPosTexNor& vertex = vecVertices[j];
						file.Read(&vertex.pos.x, 3);
						file.Read(&vertex.uv.x, 2);
						file.Read(&vertex.normal.x, 3);
					}

					pVertexBuffer = IVertexBuffer::Create(VertexPosTexNor::Format(), vecVertices.size(), &vecVertices.front(), D3D11_USAGE_DYNAMIC, IVertexBuffer::eSaveVertexPos);
					if (pVertexBuffer == nullptr)
					{
						LOG_ERROR("���ؽ� ���� ���� �����߽���");
						return false;
					}
				}
				else if (nType == EmModelNode::eSkinned)
				{
					std::vector<VertexPosTexNorWeiIdx> vecVertices;
					vecVertices.resize(nVertexCount);

					for (uint32_t j = 0; j < nVertexCount; ++j)
					{
						VertexPosTexNorWeiIdx& vertex = vecVertices[j];
						file.Read(&vertex.pos.x, 3);
						file.Read(&vertex.uv.x, 2);
						file.Read(&vertex.normal.x, 3);
						file.Read(&vertex.boneWeight.x, 3);
						file.Read(&vertex.boneIndices[0], 4);
					}

					pVertexBuffer = IVertexBuffer::Create(VertexPosTexNorWeiIdx::Format(), vecVertices.size(), &vecVertices.front(), D3D11_USAGE_DYNAMIC, IVertexBuffer::eSaveVertexPos);
					if (pVertexBuffer == nullptr)
					{
						LOG_ERROR("���ؽ� ���� ���� �����߽���");
						return false;
					}
				}

				pNode->SetVertexBuffer(pVertexBuffer);

				uint32_t nIndexCount = 0;
				file >> nIndexCount;

				std::vector<uint32_t> vecIndices;
				vecIndices.resize(nIndexCount);

				for (uint32_t j = 0; j < nIndexCount; ++j)
				{
					file >> vecIndices[j];
				}

				IIndexBuffer* pIndexBuffer = IIndexBuffer::Create(vecIndices.size(), &vecIndices.front(), D3D11_USAGE_DYNAMIC, IIndexBuffer::eSaveRawValue);
				if (pIndexBuffer == nullptr)
				{
					LOG_ERROR("�ε��� ���� ���� �����߽���");
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

			if (isHasSkeleton == true)
			{
				uint32_t nBoneCount = 0;
				file >> nBoneCount;
				
				for (uint32_t i = 0; i < nBoneCount; ++i)
				{
					std::string strName;
					file >> strName;

					std::string strParentName;
					file >> strParentName;

					Math::Matrix matMotionOffset;
					file.Read(&matMotionOffset._11, 16);

					Math::Matrix matDefaultMotionData;
					file.Read(&matDefaultMotionData._11, 16);

					if (strParentName == "NoParent")
					{
						m_skeleton.CreateBone(strName.c_str(), matMotionOffset, matDefaultMotionData);
					}
					else
					{
						m_skeleton.CreateBone(strParentName.c_str(), strName.c_str(), matMotionOffset, matDefaultMotionData);
					}
				}
			}

			for (const auto& pNode : m_vecModelNodes)
			{
				const uint32_t nMaterialCount = pNode->GetMaterialCount();
				for (uint32_t i = 0; i < nMaterialCount; ++i)
				{
					IMaterial* pMaterial = pNode->GetMaterial(i);
					pMaterial->LoadTexture();
				}

				const uint32_t nBoneCount = m_skeleton.GetBoneCount();
				if (nBoneCount > 0)
				{
					if (pNode->GetType() != EmModelNode::eSkinned)
						continue;

					ModelNodeSkinned* pNodeSkinned = static_cast<ModelNodeSkinned*>(pNode);

					size_t nIncludBoneCount = pNodeSkinned->GetBoneCount();

					std::vector<String::StringID> vecBoneNames;
					vecBoneNames.resize(nIncludBoneCount);

					for (size_t j = 0; j < nIncludBoneCount; ++j)
					{
						vecBoneNames[j] = pNodeSkinned->GetBoneName(j);
					}

					m_skeleton.SetSkinnedList(pNodeSkinned->GetName(), &vecBoneNames.front(), vecBoneNames.size());
				}
			}

			return true;
		}
	}
}