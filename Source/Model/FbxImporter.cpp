#include "StdAfx.h"
#include "FbxImporter.h"

#include "CommonLib/FileStream.h"
#include "CommonLib/FileUtil.h"

#include "DirectX/Vertex.h"

#include "Model.h"
#include "ModelNodeStatic.h"
#include "ModelNodeSkinned.h"
#include "Motion.h"

#include "Skeleton.h"

#include <fbxsdk.h>

namespace EastEngine
{
	namespace Graphics
	{
		namespace FbxUtil
		{
			void ConvertVector(Math::Vector3& d3dVector, fbxsdk::FbxVector4 fbxVector);
			void ConvertVector(Math::Vector4& d3dVector, fbxsdk::FbxVector4 fbxVector);
			void ConvertMatrix(Math::Matrix& d3dMatrix, fbxsdk::FbxAMatrix fbxMatrix);
			void ConvertMaterial(IMaterial* pMaterial, fbxsdk::FbxSurfaceMaterial* pFbxMaterial);;
			void ConvertColor(Math::Color& d3dColorValue, fbxsdk::FbxColor fbxColor, float fColorFactor);
			void LoadTextureName(FbxProperty Property, String::StringID& strTextureName);
			fbxsdk::FbxAMatrix GetGeometry(fbxsdk::FbxNode* pNode);
			fbxsdk::FbxAMatrix GetLocalTransform(fbxsdk::FbxNode* pNode);
			fbxsdk::FbxAMatrix GetLocalTransform(fbxsdk::FbxNode* pNode, const fbxsdk::FbxTime& pTime);
			void InsertTimeset(std::set<fbxsdk::FbxTime>& timeSet, fbxsdk::FbxAnimCurve* pCurve, fbxsdk::FbxTime& tStart, fbxsdk::FbxTime& tStop);

			void ConvertVector(Math::Vector3& d3dVector, fbxsdk::FbxVector4 fbxVector)
			{
				d3dVector.x = (float)fbxVector[0];
				d3dVector.y = (float)fbxVector[1];
				d3dVector.z = (float)fbxVector[2];
			}

			void ConvertVector(Math::Vector4& d3dVector, fbxsdk::FbxVector4 fbxVector)
			{
				d3dVector.x = (float)fbxVector[0];
				d3dVector.y = (float)fbxVector[1];
				d3dVector.z = (float)fbxVector[2];
				d3dVector.w = (float)fbxVector[3];
			}

			void ConvertMatrix(Math::Matrix& d3dMatrix, fbxsdk::FbxAMatrix fbxMatrix)
			{
				for (int i = 0; i < 4; i++)
				{
					for (int j = 0; j < 4; j++)
					{
						d3dMatrix.m[i][j] = (float)fbxMatrix.Get(i, j);
					}
				}
			}

			void ConvertMaterial(IMaterial* pMaterial, fbxsdk::FbxSurfaceMaterial* pFbxMaterial)
			{
				fbxsdk::FbxDouble3 fbxDouble3;
				fbxsdk::FbxColor fbxColor;

				float globalAmbientFactor = 0.4f;

				if (pFbxMaterial->GetClassId().Is(fbxsdk::FbxSurfaceLambert::ClassId))
				{
					fbxsdk::FbxSurfaceLambert *lambert = (fbxsdk::FbxSurfaceLambert*)pFbxMaterial;

					Math::Color color;

					fbxDouble3 = lambert->Ambient;
					float ambientFactor = (float)lambert->AmbientFactor.Get() * globalAmbientFactor;
					fbxColor.Set(fbxDouble3[0], fbxDouble3[1], fbxDouble3[2]);
					ConvertColor(color, fbxColor, ambientFactor);
					//pMaterial->SetAmbientColor(color);

					fbxDouble3 = lambert->Diffuse;
					float diffuseFactor = (float)lambert->DiffuseFactor.Get();
					fbxColor.Set(fbxDouble3[0], fbxDouble3[1], fbxDouble3[2]);
					ConvertColor(color, fbxColor, diffuseFactor);
					pMaterial->SetAlbedoColor(color);

					fbxDouble3 = lambert->Emissive;
					float emissiveFactor = (float)lambert->EmissiveFactor.Get();
					fbxColor.Set(fbxDouble3[0], fbxDouble3[1], fbxDouble3[2]);
					ConvertColor(color, fbxColor, emissiveFactor);
					pMaterial->SetEmissiveColor(color);
				}
				else if (pFbxMaterial->GetClassId().Is(fbxsdk::FbxSurfacePhong::ClassId))
				{
					fbxsdk::FbxSurfacePhong *phong = static_cast<fbxsdk::FbxSurfacePhong*>(pFbxMaterial);

					Math::Color color;

					fbxDouble3 = phong->Ambient;
					float ambientFactor = (float)phong->AmbientFactor.Get() * globalAmbientFactor;
					fbxColor.Set(fbxDouble3[0], fbxDouble3[1], fbxDouble3[2]);
					ConvertColor(color, fbxColor, ambientFactor);
					//pMaterial->SetAmbientColor(color);

					fbxDouble3 = phong->Diffuse;
					float diffuseFactor = (float)phong->DiffuseFactor.Get();
					fbxColor.Set(fbxDouble3[0], fbxDouble3[1], fbxDouble3[2]);
					ConvertColor(color, fbxColor, diffuseFactor);
					pMaterial->SetAlbedoColor(color);

					fbxDouble3 = phong->Specular;
					float specularFactor = (float)phong->SpecularFactor.Get();
					fbxColor.Set(fbxDouble3[0], fbxDouble3[1], fbxDouble3[2]);
					ConvertColor(color, fbxColor, specularFactor);
					//pMaterial->SetSpecularColor(color);

					// 스펙큘러 강도
					color.a = (float)phong->Shininess;

					fbxDouble3 = phong->Emissive;
					float emissiveFactor = (float)phong->EmissiveFactor.Get();
					fbxColor.Set(fbxDouble3[0], fbxDouble3[1], fbxDouble3[2]);
					ConvertColor(color, fbxColor, emissiveFactor);
					pMaterial->SetEmissiveColor(color);
				}
			}

			void ConvertColor(Math::Color& d3dColorValue,
				fbxsdk::FbxColor fbxColor,
				float fColorFactor)
			{
				d3dColorValue.r = (float)fbxColor.mRed * fColorFactor;
				d3dColorValue.g = (float)fbxColor.mGreen * fColorFactor;
				d3dColorValue.b = (float)fbxColor.mBlue * fColorFactor;
				d3dColorValue.a = 1.f;
			}

			void LoadTextureName(FbxProperty Property, String::StringID& strTextureName)
			{
				strTextureName.clear();

				if (Property.IsValid() == false)
					return;

				int nTextureNum = Property.GetSrcObjectCount<fbxsdk::FbxTexture>();

				for (int nTextureIndex = 0; nTextureIndex < nTextureNum; nTextureIndex++)
				{
					fbxsdk::FbxFileTexture* pTexture = Property.GetSrcObject<fbxsdk::FbxFileTexture>(nTextureIndex);

					if (pTexture == nullptr)
						continue;

					fbxsdk::FbxString FileName = pTexture->GetFileName();
					if (FileName.IsEmpty() == true)
						continue;

					strTextureName = File::GetFileName(FileName.Buffer()).c_str();
				}
			}

			fbxsdk::FbxAMatrix GetGeometry(fbxsdk::FbxNode* pNode)
			{
				fbxsdk::FbxVector4 vT, vR, vS;

				vT = pNode->GetGeometricTranslation(fbxsdk::FbxNode::eSourcePivot);
				vR = pNode->GetGeometricRotation(fbxsdk::FbxNode::eSourcePivot);
				vS = pNode->GetGeometricScaling(fbxsdk::FbxNode::eSourcePivot);

				return FbxAMatrix(vT, vR, vS);
			}

			fbxsdk::FbxAMatrix GetLocalTransform(fbxsdk::FbxNode* pNode)
			{
				return pNode->EvaluateLocalTransform();
			}

			fbxsdk::FbxAMatrix GetLocalTransform(fbxsdk::FbxNode* pNode, const fbxsdk::FbxTime& pTime)
			{
				return pNode->EvaluateLocalTransform(pTime);
			}

			void InsertTimeset(std::set<fbxsdk::FbxTime>& timeSet, fbxsdk::FbxAnimCurve* pCurve, fbxsdk::FbxTime& tStart, fbxsdk::FbxTime& tStop)
			{
				if (pCurve == nullptr)
					return;

				int nKeyCount = pCurve->KeyGetCount();

				for (int nCount = 0; nCount < nKeyCount; nCount++)
				{
					fbxsdk::FbxTime KeyTime = pCurve->KeyGetTime(nCount);

					if (KeyTime < tStart)
					{
						tStart = KeyTime;
					}

					if (KeyTime > tStop)
					{
						tStop = KeyTime;
					}

					timeSet.insert(KeyTime);
				}
			}
		}

		SFbxImporter::SFbxImporter()
			: m_isInit(false)
			, m_pSdkManager(nullptr)
			, m_pImporter(nullptr)
			, m_pScene(nullptr)
			, m_fScaleFactor(1.f)
		{
		}

		SFbxImporter::~SFbxImporter()
		{
			Release();
		}

		bool SFbxImporter::Init()
		{
			if (m_isInit == true)
				return true;

			m_isInit = true;

			return true;
		}

		void SFbxImporter::Release()
		{
			if (m_isInit == false)
				return;

			if (m_pScene != nullptr)
			{
				m_pScene->Destroy();
			}

			if (m_pImporter != nullptr)
			{
				m_pImporter->Destroy();
			}

			if (m_pSdkManager != nullptr)
			{
				m_pSdkManager->Destroy();
			}

			m_isInit = false;
		}

		bool SFbxImporter::LoadModel(IModel* pModel, const char* strFilePath, float fScaleFactor)
		{
			if (pModel == nullptr)
				return false;

			if (initSdkObjects() == false)
				return false;

			if (initImporter(strFilePath, fScaleFactor) == false)
				return false;

			m_umapOffsetMatrix.clear();

			createModel(m_pScene->GetRootNode(), pModel, nullptr);

			std::function<bool(fbxsdk::FbxNode*)> IsSkinningModel = [&](fbxsdk::FbxNode* pNode) -> bool
			{
				if (pNode == nullptr)
					return false;

				fbxsdk::FbxNodeAttribute* pNodeAttribute = pNode->GetNodeAttribute();
				if (pNodeAttribute != nullptr)
				{
					fbxsdk::FbxNodeAttribute::EType AttributeType = pNodeAttribute->GetAttributeType();
					if (AttributeType == fbxsdk::FbxNodeAttribute::eSkeleton)
						return true;
				}

				for (int i = 0; i < pNode->GetChildCount(); ++i)
				{
					if (IsSkinningModel(pNode->GetChild(i)) == true)
						return true;
				}

				return false;
			};

			ISkeleton* pSkeleton = nullptr;
			if (IsSkinningModel(m_pScene->GetRootNode()) == true)
			{
				pSkeleton = ISkeleton::Create();

				Model* pRealModel = static_cast<Model*>(pModel);
				pRealModel->SetSkeleton(pSkeleton);
				createSkeleton(m_pScene->GetRootNode(), pModel, pSkeleton);
			}

			uint32_t nNodeCount = pModel->GetNodeCount();
			for (uint32_t i = 0; i < nNodeCount; ++i)
			{
				IModelNode* pNode = pModel->GetNode(i);

				uint32_t nMaterialCount = pNode->GetMaterialCount();
				for (uint32_t j = 0; j < nMaterialCount; ++j)
				{
					IMaterial* pMaterial = pNode->GetMaterial(j);
					pMaterial->LoadTexture();
				}

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

				Skeleton* pRealSkeleton = static_cast<Skeleton*>(pSkeleton);
				pRealSkeleton->SetSkinnedList(pNodeSkinned->GetName(), &vecBoneNames.front(), vecBoneNames.size());
			}

			return true;
		}

		bool SFbxImporter::LoadMotion(IMotion* pMotion, const char* strFilePath, float fScaleFactor)
		{
			if (initSdkObjects() == false)
				return false;

			if (initImporter(strFilePath, fScaleFactor) == false)
				return false;

			createMotion(m_pScene, pMotion);

			return true;
		}

		bool SFbxImporter::initSdkObjects()
		{
			if (m_pSdkManager == nullptr)
			{
				// FBX Manager 생성
				m_pSdkManager = fbxsdk::FbxManager::Create();
				if (m_pSdkManager == nullptr)
					return false;

				// IOSettings 생성
				fbxsdk::FbxIOSettings* pIOSetting = fbxsdk::FbxIOSettings::Create(m_pSdkManager, IOSROOT);
				if (pIOSetting == nullptr)
					return false;

				m_pSdkManager->SetIOSettings(pIOSetting);

				// 플러그인 로드
				// TCMalloc 때문에 디버그 모드에서 에러가 난다.
				// TCMalloc 사용 설정 안해도 사용해버린다. 왜?
				// dll 파일만 있어도 발생함
#if defined(DEBUG) == false && defined(_DEBUG) == false
				fbxsdk::FbxString lPath = FbxGetApplicationDirectory();
				m_pSdkManager->LoadPluginsDirectory(lPath.Buffer());
#endif
			}

			if (m_pImporter == nullptr)
			{
				m_pImporter = fbxsdk::FbxImporter::Create(m_pSdkManager, "");
				if (m_pImporter == nullptr)
					return false;
			}

			if (m_pScene == nullptr)
			{
				// FBX scene 생성
				m_pScene = fbxsdk::FbxScene::Create(m_pSdkManager, "My Scene");
				if (m_pScene == nullptr)
					return false;
			}

			return true;
		}

		bool SFbxImporter::initImporter(const char* strFilePath, float fScaleFactor)
		{
			// 임포터 생성
			int lFileFormat = -1;
			if (m_pSdkManager->GetIOPluginRegistry()->DetectReaderFileFormat(strFilePath, lFileFormat) == false)
			{
				lFileFormat = m_pSdkManager->GetIOPluginRegistry()->FindReaderIDByDescription("FBX binery (*.fbx)");
			}

			// 임포터 초기화
			if (m_pImporter->Initialize(strFilePath, -1, m_pSdkManager->GetIOSettings()) == false)
			{
				PRINT_LOG("FBX Importer Error : %s", m_pImporter->GetStatus().GetErrorString());
				return false;
			}

			if (m_pImporter->IsFBX())
			{
				FbxIOSettings* ioSettings = m_pSdkManager->GetIOSettings();
				ioSettings->SetBoolProp(IMP_FBX_CONSTRAINT, true);
				ioSettings->SetBoolProp(IMP_FBX_CONSTRAINT_COUNT, true);
				ioSettings->SetBoolProp(IMP_FBX_MATERIAL, true);
				ioSettings->SetBoolProp(IMP_FBX_TEXTURE, true);
				ioSettings->SetBoolProp(IMP_FBX_LINK, true);
				ioSettings->SetBoolProp(IMP_FBX_SHAPE, true);
				ioSettings->SetBoolProp(IMP_FBX_GOBO, true);
				ioSettings->SetBoolProp(IMP_FBX_ANIMATION, true);
				ioSettings->SetBoolProp(IMP_FBX_GLOBAL_SETTINGS, true);
			}

			// 임포트
			m_pScene->Clear();

			if (m_pImporter->Import(m_pScene) == false)
				return false;

			FbxAxisSystem SceneAxisSystem = m_pScene->GetGlobalSettings().GetAxisSystem();

			//FbxAxisSystem OurAxisSystem(FbxAxisSystem::eDirectX);
			FbxAxisSystem OurAxisSystem(
				FbxAxisSystem::eYAxis,
				FbxAxisSystem::eParityOdd,
				FbxAxisSystem::eRightHanded);
			if (SceneAxisSystem != OurAxisSystem)
			{
				OurAxisSystem.ConvertScene(m_pScene);
			}

			// 스케일 조절
			FbxSystemUnit SceneSystemUnit = m_pScene->GetGlobalSettings().GetSystemUnit();
			if (SceneSystemUnit.GetScaleFactor() != fScaleFactor)
			{
				FbxSystemUnit OurSystemUnit(fScaleFactor);
				OurSystemUnit.ConvertScene(m_pScene);
			}

			m_fScaleFactor = fScaleFactor;

			return true;
		}

		void SFbxImporter::createModel(fbxsdk::FbxNode* pNode, IModel* pModel, IModelNode* pParentNode)
		{
			if (pNode == nullptr)
				return;

			IModelNode* pModelNode = buildNode(pNode);
			if (pModelNode != nullptr)
			{
				if (pParentNode != nullptr)
				{
					ModelNode* pRealParentNode = static_cast<ModelNode*>(pParentNode);
					pRealParentNode->AddChildNode(pModelNode);

					ModelNode* pRealChildNode = static_cast<ModelNode*>(pModelNode);
					pRealChildNode->SetParentNode(pParentNode);

					Model* pRealModel = static_cast<Model*>(pModel);
					pRealModel->AddNode(pModelNode, pModelNode->GetName(), false);
				}
				else
				{
					Model* pRealModel = static_cast<Model*>(pModel);
					pRealModel->AddNode(pModelNode, pModelNode->GetName(), true);
				}
			}

			for (int i = 0; i < pNode->GetChildCount(); ++i)
			{
				createModel(pNode->GetChild(i), pModel, pModelNode);
			}
		}

		IModelNode* SFbxImporter::buildNode(fbxsdk::FbxNode* pNode)
		{
			if (pNode == nullptr)
				return nullptr;

			fbxsdk::FbxNodeAttribute* pNodeAttribute = pNode->GetNodeAttribute();
			if (pNodeAttribute == nullptr)
				return nullptr;

			IModelNode* pModelNode = nullptr;

			fbxsdk::FbxNodeAttribute::EType AttributeType = pNodeAttribute->GetAttributeType();
			switch (AttributeType)
			{
			case fbxsdk::FbxNodeAttribute::eMesh:
				pModelNode = createModelNode(pNode);
				break;
			default:
				return nullptr;
			}

			return pModelNode;
		}

		IModelNode* SFbxImporter::createModelNode(fbxsdk::FbxNode* pNode)
		{
			bool bVisible = pNode->GetVisibility();
			if (bVisible == true)
			{
				FbxMesh* pFbxMesh = pNode->GetMesh();
				if (pFbxMesh == nullptr)
					return nullptr;

				if (pFbxMesh->GetControlPointsCount() <= 0)
					return nullptr;

				// 폴리곤이 3개 초과한 버텍스로 이루어져 있다면 나눈다.
				bool bNeedToTriangulate = false;
				int nPolygonCount = pFbxMesh->GetPolygonCount();
				for (int i = 0; i < nPolygonCount; ++i)
				{
					int nPolygonSize = pFbxMesh->GetPolygonSize(i);

					if (nPolygonSize > 3)
					{
						bNeedToTriangulate = true;
						break;
					}
				}

				if (bNeedToTriangulate)
				{
					FbxGeometryConverter geometryConverter(m_pSdkManager);
					geometryConverter.Triangulate(pNode->GetNodeAttribute(), true);

					pFbxMesh = pNode->GetMesh();
				}

				if (pFbxMesh == nullptr)
					return nullptr;

				FbxSkin* pSkin = static_cast<FbxSkin*>(pFbxMesh->GetDeformer(0, FbxDeformer::eSkin));
				if (pSkin != nullptr)
				{
					int nClusterCount = pSkin->GetClusterCount();
					if (nClusterCount > 0)
					{
						return createSkinnedNode(pNode);
					}
				}
			}

			return createStaticNode(pNode);
		}

		IModelNode* SFbxImporter::createStaticNode(fbxsdk::FbxNode* pNode)
		{
			std::vector<VertexPosTexNor> vecVertices;
			std::vector<uint32_t> vecIndicecs;
			std::vector<int> vecAttributeList;
			std::vector<IMaterial*> vecMaterial;
			Math::Matrix matTransformation;

			// 행렬
			FbxAMatrix LocalTransform = FbxUtil::GetLocalTransform(pNode);
			FbxUtil::ConvertMatrix(matTransformation, LocalTransform);

			matTransformation *= Math::Matrix::CreateScale(1.f / m_fScaleFactor);

			bool bVisible = pNode->GetVisibility();
			if (bVisible == true)
			{
				// 버텍스 정보
				loadGeometry(pNode, vecVertices, vecIndicecs, vecAttributeList);

				// 메테리얼 정보
				loadMaterial(pNode, vecMaterial);
			}

			ModelNodeStatic* pModelNode = new ModelNodeStatic;
			pModelNode->SetNodeName(pNode->GetName());
			pModelNode->SetVisible(bVisible);
			pModelNode->SetTransformationMatrix(matTransformation);

			if (vecVertices.empty() == false && vecIndicecs.empty() == false)
			{
				IVertexBuffer* pVertexBuffer = IVertexBuffer::Create(VertexPosTexNor::Format(), vecVertices.size(), &vecVertices.front(), D3D11_USAGE_DYNAMIC, IVertexBuffer::eSaveVertexPos/* | IVertexBuffer::eSaveVertexClipSpace*/);
				IIndexBuffer* pIndexBuffer = IIndexBuffer::Create(vecIndicecs.size(), &vecIndicecs.front(), D3D11_USAGE_DYNAMIC, IIndexBuffer::eSaveRawValue);

				if (pVertexBuffer == nullptr || pIndexBuffer == nullptr)
				{
					PRINT_LOG("Can't created buffer(S/F), Vertex %s, Index %s", pVertexBuffer != nullptr ? "S" : "F", pIndexBuffer != nullptr ? "S" : "F");
					SafeDelete(pVertexBuffer);
					SafeDelete(pIndexBuffer);
				}
				else
				{
					pModelNode->SetVertexBuffer(pVertexBuffer);
					pModelNode->SetIndexBuffer(pIndexBuffer);
				}
			}

			pModelNode->AddMaterialArray(&vecMaterial.front(), vecMaterial.size());

			// 폴리곤별 머테리얼 범위 지정
			uint32_t nCurMatID = UINT32_MAX;
			ModelSubset curAttRange;
			std::vector<ModelSubset> vecModelSubsets;

			uint32_t nSize = vecAttributeList.size();
			for (uint32_t i = 0; i < nSize; ++i)
			{
				uint32_t nMatID = static_cast<uint32_t>(vecAttributeList[i]);
				if (nCurMatID != nMatID)
				{
					if (nCurMatID != UINT32_MAX)
					{
						vecModelSubsets.push_back(curAttRange);
					}
					curAttRange.nMaterialID = nMatID;
					curAttRange.nStartIndex = i * 3;
					curAttRange.nIndexCount = 3;

					nCurMatID = nMatID;
				}
				else
				{
					curAttRange.nIndexCount += 3;
				}
			}

			if (nCurMatID != UINT32_MAX)
			{
				vecModelSubsets.push_back(curAttRange);
			}

			pModelNode->AddModelSubsets(vecModelSubsets);

			return pModelNode;
		}

		IModelNode* SFbxImporter::createSkinnedNode(fbxsdk::FbxNode* pNode)
		{
			std::vector<VertexPosTexNor> vecVertices;
			std::vector<uint32_t> vecIndicecs;
			std::vector<int> vecAttributeList;
			boost::unordered_map<int, std::vector<int>> umapControlPointVertex;
			std::vector<IMaterial*> vecMaterial;

			// 행렬
			FbxAMatrix LocalTransform = FbxUtil::GetLocalTransform(pNode);
			Math::Matrix matTransformation;
			FbxUtil::ConvertMatrix(matTransformation, LocalTransform);

			std::vector<String::StringID> vecBoneName;
			std::vector<boost::unordered_multimap<float, int>> vecWeightBone;

			bool bVisible = pNode->GetVisibility();
			if (bVisible == true)
			{
				// 버텍스 정보
				loadGeometry(pNode, vecVertices, vecIndicecs, vecAttributeList, &umapControlPointVertex);

				// 메테리얼 정보
				loadMaterial(pNode, vecMaterial);

				FbxMesh* pFbxMesh = pNode->GetMesh();
				FbxSkin* pSkin = static_cast<FbxSkin*>(pFbxMesh->GetDeformer(0, FbxDeformer::eSkin));
				int nClusterCount = pSkin->GetClusterCount();

				int nVertexCnt = pFbxMesh->GetPolygonVertexCount();
				vecWeightBone.resize(nVertexCnt);

				for (int nBoneNum = 0; nBoneNum < nClusterCount; nBoneNum++)
				{
					fbxsdk::FbxCluster* pCluster = pSkin->GetCluster(nBoneNum);

					fbxsdk::FbxNode* pLinkNode = pCluster->GetLink();
					String::StringID strBoneName = pLinkNode->GetName();
					vecBoneName.emplace_back(strBoneName);

					// 가중치, 본인덱스 정보
					int nIndexCount = pCluster->GetControlPointIndicesCount();
					int* pIndices = pCluster->GetControlPointIndices();
					double* pWeights = pCluster->GetControlPointWeights();

					for (int nIndex = 0; nIndex < nIndexCount; nIndex++)
					{
						int nControlPointIndex = pIndices[nIndex];
						float dWeight = static_cast<float>(pWeights[nIndex]);

						auto iter = umapControlPointVertex.find(nControlPointIndex);
						if (iter == umapControlPointVertex.end())
							continue;

						std::vector<int>& vecVertexIdx = iter->second;
						for (auto nVertexIdx : vecVertexIdx)
						{
							vecWeightBone[nVertexIdx].emplace(dWeight, nBoneNum);
						}
					}

					// Offset Matrix, 본행렬을 로컬좌표로 이동시켜주는 행렬
					FbxAMatrix boneMatrix;
					pCluster->GetTransformLinkMatrix(boneMatrix);

					FbxAMatrix meshMatrix;
					pCluster->GetTransformMatrix(meshMatrix);

					Math::Matrix matOffset;
					FbxUtil::ConvertMatrix(matOffset, boneMatrix.Inverse() * meshMatrix);
					//matOffset = Math::Matrix::ZConversion * matOffset;

					m_umapOffsetMatrix.emplace(strBoneName, matOffset);
					m_umapTransform.emplace(strBoneName, matTransformation);
				}
			}

			std::vector<VertexPosTexNorBleIdx> vecSkinnedVertices;
			uint32_t nSize = vecVertices.size();
			vecSkinnedVertices.resize(nSize);

			for (uint32_t i = 0; i < nSize; ++i)
			{
				vecSkinnedVertices[i] = vecVertices[i];

				// 스키닝을 위한 가중치 정보
				if (vecWeightBone.size() <= i)
					continue;

				int nCnt = 0;
				float* pBlend = reinterpret_cast<float*>(&vecSkinnedVertices[i].blend);
				boost::unordered_multimap<float, int>& umapWeight = vecWeightBone[i];
				for (const auto& iter : umapWeight)
				{
					if (Math::IsZero(iter.first))
						continue;

					if (nCnt == 3)
					{
						vecSkinnedVertices[i].idx |= iter.second << 24;
						break;
					}

					pBlend[nCnt] = iter.first;
					vecSkinnedVertices[i].idx |= iter.second << (8 * nCnt);

					++nCnt;
				}
			}

			ModelNodeSkinned* pModelNode = new ModelNodeSkinned;
			pModelNode->SetNodeName(pNode->GetName());
			pModelNode->SetVisible(bVisible);
			pModelNode->SetTransformationMatrix(matTransformation);
			pModelNode->SetBoneNameList(vecBoneName);

			if (vecSkinnedVertices.empty() == false && vecIndicecs.empty() == false)
			{
				IVertexBuffer* pVertexBuffer = IVertexBuffer::Create(VertexPosTexNorBleIdx::Format(), vecSkinnedVertices.size(), &vecSkinnedVertices.front(), D3D11_USAGE_DYNAMIC, IVertexBuffer::eSaveVertexPos/* | IVertexBuffer::eSaveVertexClipSpace*/);
				IIndexBuffer* pIndexBuffer = IIndexBuffer::Create(vecIndicecs.size(), &vecIndicecs.front(), D3D11_USAGE_DYNAMIC, IIndexBuffer::eSaveRawValue);

				if (pVertexBuffer == nullptr || pIndexBuffer == nullptr)
				{
					PRINT_LOG("Can't created buffer(S/F), Vertex %s, Index %s", pVertexBuffer != nullptr ? "S" : "F", pIndexBuffer != nullptr ? "S" : "F");
					SafeDelete(pVertexBuffer);
					SafeDelete(pIndexBuffer);
				}
				else
				{
					pModelNode->SetVertexBuffer(pVertexBuffer);
					pModelNode->SetIndexBuffer(pIndexBuffer);
				}
			}

			pModelNode->AddMaterialArray(&vecMaterial.front(), vecMaterial.size());

			// 폴리곤별 머테리얼 범위 지정
			uint32_t nCurMatID = UINT32_MAX;
			ModelSubset curAttRange;
			std::vector<ModelSubset> vecModelSubsets;

			nSize = vecAttributeList.size();
			for (uint32_t i = 0; i < nSize; ++i)
			{
				uint32_t nMatID = static_cast<uint32_t>(vecAttributeList[i]);
				if (nCurMatID != nMatID)
				{
					if (nCurMatID != UINT32_MAX)
					{
						vecModelSubsets.push_back(curAttRange);
					}
					curAttRange.nMaterialID = nMatID;
					curAttRange.nStartIndex = i * 3;
					curAttRange.nIndexCount = 3;

					nCurMatID = nMatID;
				}
				else
				{
					curAttRange.nIndexCount += 3;
				}
			}

			if (nCurMatID != UINT32_MAX)
			{
				vecModelSubsets.push_back(curAttRange);
			}

			pModelNode->AddModelSubsets(vecModelSubsets);

			return pModelNode;
		}

		void SFbxImporter::createSkeleton(fbxsdk::FbxNode* pNode, IModel* pModel, ISkeleton* pSkeleton, const String::StringID& strParentName)
		{
			if (pNode == nullptr || pSkeleton == nullptr)
				return;

			String::StringID strBoneName;
			fbxsdk::FbxNodeAttribute* pNodeAttribute = pNode->GetNodeAttribute();
			if (pNodeAttribute != nullptr)
			{
				fbxsdk::FbxNodeAttribute::EType AttributeType = pNodeAttribute->GetAttributeType();
				if (AttributeType == fbxsdk::FbxNodeAttribute::eSkeleton)
				{
					strBoneName = pNode->GetName();

					Math::Matrix matOffset;
					auto iter = m_umapOffsetMatrix.find(strBoneName);
					if (iter != m_umapOffsetMatrix.end())
					{
						matOffset = iter->second;
					}

					Math::Matrix matTransformation;
					iter = m_umapTransform.find(strBoneName);
					if (iter != m_umapTransform.end())
					{
						matTransformation = iter->second;
					}

					Skeleton* pRealSkeleton = static_cast<Skeleton*>(pSkeleton);

					ISkeleton::IBone* pBone = nullptr;
					if (strParentName.empty() == true)
					{
						pBone = pRealSkeleton->CreateBone(strBoneName, matOffset, matTransformation);
					}
					else
					{
						pBone = pRealSkeleton->CreateBone(strParentName, strBoneName, matOffset, matTransformation);
					}
				}
			}

			for (int i = 0; i < pNode->GetChildCount(); ++i)
			{
				createSkeleton(pNode->GetChild(i), pModel, pSkeleton, strBoneName);
			}
		}

		void SFbxImporter::loadGeometry(fbxsdk::FbxNode* pNode, std::vector<VertexPosTexNor>& vecVertices, std::vector<uint32_t>& vecIndicecs,
			std::vector<int>& vecAttributeList, boost::unordered_map<int, std::vector<int>>* pUmapControlPointVertex)
		{
			FbxMesh* pFbxMesh = pNode->GetMesh();

			int nPolygonCount = pFbxMesh->GetPolygonCount();
			vecVertices.resize(nPolygonCount * 3);

			FbxVector4* pControlPoints = pFbxMesh->GetControlPoints();

			FbxAMatrix RefrenceGeometry = FbxUtil::GetGeometry(pFbxMesh->GetNode());
			Math::Matrix geometryMatrix;
			FbxUtil::ConvertMatrix(geometryMatrix, RefrenceGeometry);
			//geometryMatrix = Math::Matrix::ZConversion * geometryMatrix;

			for (int nPolygonIdx = 0; nPolygonIdx < nPolygonCount; nPolygonIdx++)
			{
				int nPolygonSize = pFbxMesh->GetPolygonSize(nPolygonIdx);

				for (int nVertexIdx = 0; nVertexIdx < nPolygonSize; ++nVertexIdx)
				{
					int nControlPointIdx = pFbxMesh->GetPolygonVertex(nPolygonIdx, nVertexIdx);

					FbxVector4 p = pControlPoints[nControlPointIdx];

					int nVIndex = nPolygonIdx * 3 + nVertexIdx;

					VertexPosTexNor& vertex = vecVertices[nVIndex];
					vertex.pos.x = (float)p[0];
					vertex.pos.y = (float)p[1];
					vertex.pos.z = (float)p[2];

					vertex.pos = Math::Vector3::Transform(vertex.pos, geometryMatrix);

					if (pUmapControlPointVertex != nullptr)
					{
						(*pUmapControlPointVertex)[nControlPointIdx].push_back(nVIndex);
					}

					// normal
					{
						for (int i = 0; i < pFbxMesh->GetLayerCount(); ++i)
						{
							FbxLayerElementNormal* pLayerNormals = pFbxMesh->GetLayer(i)->GetNormals();
							if (pLayerNormals != nullptr)
							{
								FbxLayerElement::EMappingMode normalMappingMode = pLayerNormals->GetMappingMode();
								FbxLayerElement::EReferenceMode normalReferenceMode = pLayerNormals->GetReferenceMode();

								if (normalMappingMode == FbxLayerElement::eByControlPoint)
								{
									int _nControlPointIdx = pFbxMesh->GetPolygonVertex(nPolygonIdx, nVertexIdx);
									FbxVector4 normal = pLayerNormals->GetDirectArray().GetAt(_nControlPointIdx);

									FbxUtil::ConvertVector(vertex.normal, normal);
									vertex.normal.Normalize();
								}
								else if (normalMappingMode == FbxLayerElement::eByPolygonVertex)
								{
									switch (normalReferenceMode)
									{
									case FbxLayerElement::eDirect:
									case FbxLayerElement::eIndexToDirect:
									{
										FbxVector4 normal(0.f, 0.f, 0.f, 0.f);
										pFbxMesh->GetPolygonVertexNormal(nPolygonIdx, nVertexIdx, normal);

										FbxUtil::ConvertVector(vertex.normal, normal);
										vertex.normal.Normalize();
									}
									break;
									}
								}
							}
						}
					}

					// uv
					{
						for (int i = 0; i < pFbxMesh->GetLayerCount(); ++i)
						{
							FbxLayer* pLayer = pFbxMesh->GetLayer(i);
							if (pLayer == nullptr)
								continue;

							FbxArray<FbxLayerElement::EType> elementArray = pLayer->GetUVSetChannels();

							int arrayCount = elementArray.GetCount();

							bool bTexCoordsFounded = false;
							for (int j = 0; j < arrayCount; ++j)
							{
								FbxLayerElement::EType elementType = elementArray.GetAt(j);

								FbxLayerElementTexture* pTextureElement = (FbxLayerElementTexture*)pLayer->GetUVs(elementType);
								FbxLayerElementUV* pTextureElementUV = (FbxLayerElementUV*)pLayer->GetUVs(elementType);

								if (pTextureElement == nullptr || pTextureElementUV == nullptr)
									continue;

								FbxLayerElement::EReferenceMode referenceMode = pTextureElement->GetReferenceMode();
								FbxLayerElement::EMappingMode mappingMode = pTextureElement->GetMappingMode();

								FbxLayerElementArrayTemplate<FbxVector2> &UVArray = pTextureElementUV->GetDirectArray();

								switch (mappingMode)
								{
								case FbxLayerElement::eByControlPoint:
								{
									switch (referenceMode)
									{
									case FbxLayerElement::eDirect:
									case FbxLayerElement::eIndexToDirect:
									{
										int nControlPointIndex = pFbxMesh->GetPolygonVertex(nPolygonIdx, nVertexIdx);

										FbxVector2 uv = UVArray.GetAt(nControlPointIndex);
										vertex.uv.x = (float)uv[0];
										vertex.uv.y = 1.f - (float)uv[1];

										bTexCoordsFounded = true;
									}
									break;
									}
								}
								break;
								case FbxLayerElement::eByPolygonVertex:
								{
									switch (referenceMode)
									{
									case FbxLayerElement::eDirect:
									case FbxLayerElement::eIndexToDirect:
									{
										int nTextureUVIndex = pFbxMesh->GetTextureUVIndex(nPolygonIdx, nVertexIdx);

										FbxVector2 uv = UVArray.GetAt(nTextureUVIndex);
										vertex.uv.x = (float)uv[0];
										vertex.uv.y = 1.f - (float)uv[1];

										bTexCoordsFounded = true;
									}
									break;
									}
								}
								break;
								}
							}

							if (bTexCoordsFounded == true)
								break;
						}
					}
				}
			}

			// 인덱스 버퍼
			uint32_t nIndexCount = nPolygonCount * 3;
			vecIndicecs.resize(nIndexCount);
			for (uint32_t i = 0; i < nIndexCount; i += 3)
			{
				vecIndicecs[i + 0] = i;
				vecIndicecs[i + 1] = i + 1;
				vecIndicecs[i + 2] = i + 2;
			}

			// 머테리얼 속성(ID)
			vecAttributeList.resize(nPolygonCount);
			for (int i = 0; i < pFbxMesh->GetLayerCount(); ++i)
			{
				FbxLayer* pLayer = pFbxMesh->GetLayer(i);
				if (pLayer == nullptr)
					continue;

				FbxLayerElementMaterial* pMaterialLayer = pLayer->GetMaterials();
				if (pMaterialLayer == nullptr)
					return;

				FbxLayerElement::EMappingMode mappingMode = pMaterialLayer->GetMappingMode();
				FbxLayerElement::EReferenceMode referenceMode = pMaterialLayer->GetReferenceMode();

				if (referenceMode == FbxLayerElement::eIndex)
					return;

				// 모든 폴리곤이 같은 머테리얼 사용
				if (mappingMode == FbxLayerElement::eAllSame)
				{
					int matID = 0;

					if (pNode->GetMaterialCount())
					{
						matID = pMaterialLayer->GetIndexArray().GetAt(0);
						if (matID == -1)
						{
							matID = 0;
						}
					}

					for (uint32_t j = 0; j < vecAttributeList.size(); ++j)
					{
						vecAttributeList[j] = matID;
					}
				}
				else if (mappingMode == FbxLayerElement::eByPolygon)
				{
					int nMaterialNum = pNode->GetSrcObjectCount<FbxSurfaceMaterial>();

					if (referenceMode == FbxLayerElement::eIndex ||
						referenceMode == FbxLayerElement::eIndexToDirect)
					{
						int nMaterialIndexCount = pMaterialLayer->GetIndexArray().GetCount();

						for (int j = 0; j < nMaterialIndexCount; ++j)
						{
							int matID = pMaterialLayer->GetIndexArray().GetAt(j);
							if (matID == -1)
								matID = 0;
							if (matID >= nMaterialNum)
								matID = 0;

							vecAttributeList[j] = matID;
						}
					}
				}
			}
		}

		void SFbxImporter::loadMaterial(fbxsdk::FbxNode* pNode, std::vector<IMaterial*>& vecMaterial)
		{
			int nMaterialNum = pNode->GetSrcObjectCount<FbxSurfaceMaterial>();

			// 머테리얼이 0개일 때, 기본 머테리얼
			if (nMaterialNum == 0)
			{
				fbxsdk::FbxNodeAttribute* pNodeAttribute = pNode->GetNodeAttribute();
				std::string strName = pNodeAttribute->GetName();
				if (strName.empty())
				{
					strName = "Default";
				}

				IMaterial* pMaterial = IMaterial::Create(strName.c_str());
				vecMaterial.push_back(pMaterial);

				if (pNodeAttribute != nullptr)
				{
					FbxDouble3 color = pNodeAttribute->Color.Get();
					Math::Color f4Albedo((float)color[0], (float)color[1], (float)color[2], 1.f);
					pMaterial->SetAlbedoColor(f4Albedo);
				}
				else
				{
					Math::Color f4Albedo(1.f, 1.f, 1.f, 1.f);
					pMaterial->SetAlbedoColor(f4Albedo);
				}
			}
			else
			{
				for (int nMaterialIndex = 0; nMaterialIndex < nMaterialNum; nMaterialIndex++)
				{
					FbxSurfaceMaterial* pFbxMaterial = pNode->GetMaterial(nMaterialIndex);
					String::StringID strName = pFbxMaterial->GetName();
					if (strName.empty())
					{
						strName = "Unknown Name";
					}

					IMaterial* pMaterial = IMaterial::Create(strName);
					if (pMaterial == nullptr)
						return;

					vecMaterial.push_back(pMaterial);

					if (pFbxMaterial == nullptr)
						continue;

					FbxUtil::ConvertMaterial(pMaterial, pFbxMaterial);

					std::string strPath = m_pImporter->GetFileName().Buffer();
					pMaterial->SetPath(File::GetFilePath(strPath));

					String::StringID strTextureName;
					FbxUtil::LoadTextureName(pFbxMaterial->FindProperty(FbxSurfaceMaterial::sDiffuse), strTextureName);
					pMaterial->SetTextureName(EmMaterial::eAlbedo, strTextureName);
					FbxUtil::LoadTextureName(pFbxMaterial->FindProperty(FbxSurfaceMaterial::sNormalMap), strTextureName);
					pMaterial->SetTextureName(EmMaterial::eNormal, strTextureName);
					FbxUtil::LoadTextureName(pFbxMaterial->FindProperty(FbxSurfaceMaterial::sSpecular), strTextureName);
					pMaterial->SetTextureName(EmMaterial::eSpecularColor, strTextureName);

					// for test
					/*FbxUtil::LoadTextureName(pFbxMaterial->FindProperty(FbxSurfaceMaterial::sDiffuse), strTextureName);
					PRINT_LOG("0 : %s", strTextureName.c_str());
					FbxUtil::LoadTextureName(pFbxMaterial->FindProperty(FbxSurfaceMaterial::sDiffuseFactor), strTextureName);
					PRINT_LOG("1 : %s", strTextureName.c_str());
					FbxUtil::LoadTextureName(pFbxMaterial->FindProperty(FbxSurfaceMaterial::sAmbient), strTextureName);
					PRINT_LOG("2 : %s", strTextureName.c_str());
					FbxUtil::LoadTextureName(pFbxMaterial->FindProperty(FbxSurfaceMaterial::sAmbientFactor), strTextureName);
					PRINT_LOG("3 : %s", strTextureName.c_str());
					FbxUtil::LoadTextureName(pFbxMaterial->FindProperty(FbxSurfaceMaterial::sBump), strTextureName);
					PRINT_LOG("4 : %s", strTextureName.c_str());
					FbxUtil::LoadTextureName(pFbxMaterial->FindProperty(FbxSurfaceMaterial::sNormalMap), strTextureName);
					PRINT_LOG("5 : %s", strTextureName.c_str());
					FbxUtil::LoadTextureName(pFbxMaterial->FindProperty(FbxSurfaceMaterial::sBumpFactor), strTextureName);
					PRINT_LOG("6 : %s", strTextureName.c_str());
					FbxUtil::LoadTextureName(pFbxMaterial->FindProperty(FbxSurfaceMaterial::sEmissive), strTextureName);
					PRINT_LOG("7 : %s", strTextureName.c_str());
					FbxUtil::LoadTextureName(pFbxMaterial->FindProperty(FbxSurfaceMaterial::sEmissiveFactor), strTextureName);
					PRINT_LOG("8 : %s", strTextureName.c_str());
					FbxUtil::LoadTextureName(pFbxMaterial->FindProperty(FbxSurfaceMaterial::sSpecular), strTextureName);
					PRINT_LOG("9 : %s", strTextureName.c_str());
					FbxUtil::LoadTextureName(pFbxMaterial->FindProperty(FbxSurfaceMaterial::sSpecularFactor), strTextureName);
					PRINT_LOG("10 : %s", strTextureName.c_str());
					FbxUtil::LoadTextureName(pFbxMaterial->FindProperty(FbxSurfaceMaterial::sShininess), strTextureName);
					PRINT_LOG("11 : %s", strTextureName.c_str());
					FbxUtil::LoadTextureName(pFbxMaterial->FindProperty(FbxSurfaceMaterial::sTransparentColor), strTextureName);
					PRINT_LOG("12 : %s", strTextureName.c_str());
					FbxUtil::LoadTextureName(pFbxMaterial->FindProperty(FbxSurfaceMaterial::sTransparencyFactor), strTextureName);
					PRINT_LOG("13 : %s", strTextureName.c_str());
					FbxUtil::LoadTextureName(pFbxMaterial->FindProperty(FbxSurfaceMaterial::sReflection), strTextureName);
					PRINT_LOG("14 : %s", strTextureName.c_str());
					FbxUtil::LoadTextureName(pFbxMaterial->FindProperty(FbxSurfaceMaterial::sReflectionFactor), strTextureName);
					PRINT_LOG("15 : %s", strTextureName.c_str());
					FbxUtil::LoadTextureName(pFbxMaterial->FindProperty(FbxSurfaceMaterial::sDisplacementColor), strTextureName);
					PRINT_LOG("16 : %s", strTextureName.c_str());
					FbxUtil::LoadTextureName(pFbxMaterial->FindProperty(FbxSurfaceMaterial::sDisplacementFactor), strTextureName);
					PRINT_LOG("17 : %s", strTextureName.c_str());
					FbxUtil::LoadTextureName(pFbxMaterial->FindProperty(FbxSurfaceMaterial::sDisplacementFactor), strTextureName);
					PRINT_LOG("18 : %s", strTextureName.c_str());*/
				}
			}
		}

		void SFbxImporter::createMotion(FbxScene* pScene, IMotion* pMotion)
		{
			//for (int i = 0; i < pScene->GetSrcObjectCount<FbxAnimStack>(); ++i)
			{
				FbxAnimStack* pAnimStack = pScene->GetSrcObject<FbxAnimStack>(0);

				createMotion(pAnimStack, pScene->GetRootNode(), pMotion);
			}
		}

		void SFbxImporter::createMotion(FbxAnimStack* pAnimStack, fbxsdk::FbxNode* pNode, IMotion* pMotion)
		{
			int nAnimLayers = pAnimStack->GetMemberCount<FbxAnimLayer>();

			const char* strTakeName = pAnimStack->GetName();

			for (int i = 0; i < nAnimLayers; ++i)
			{
				std::function<void(FbxAnimLayer*, fbxsdk::FbxNode*, const char*)> ImportAnimation = [&](FbxAnimLayer* pAnimLayer, fbxsdk::FbxNode* pParentNode, const char* strTakeName)
				{
					createMotion(pAnimLayer, pParentNode, pMotion, strTakeName);

					int nChildCount = pParentNode->GetChildCount();
					for (int j = 0; j < nChildCount; ++j)
					{
						ImportAnimation(pAnimLayer, pParentNode->GetChild(j), strTakeName);
					}
				};

				FbxAnimLayer* pAnimLayer = pAnimStack->GetMember<FbxAnimLayer>(i);
				ImportAnimation(pAnimLayer, pNode, strTakeName);
			}

			Motion* pRealMotion = static_cast<Motion*>(pMotion);
			pRealMotion->CalcClipTime();
		}

		void SFbxImporter::createMotion(fbxsdk::FbxAnimLayer* pAnimLayer, fbxsdk::FbxNode* pNode, IMotion* pMotion, const char* strTakeName)
		{
			fbxsdk::FbxNodeAttribute* pNodeAttribute = pNode->GetNodeAttribute();
			if (pNodeAttribute == nullptr)
				return;

			fbxsdk::FbxNodeAttribute::EType AttributeType = pNodeAttribute->GetAttributeType();
			if (AttributeType != fbxsdk::FbxNodeAttribute::eSkeleton)
				return;

			FbxTakeInfo* pCurrentTakeInfo = m_pScene->GetTakeInfo(strTakeName);

			FbxTime tStart, tStop;

			if (pCurrentTakeInfo)
			{
				tStart = pCurrentTakeInfo->mLocalTimeSpan.GetStart();
				tStop = pCurrentTakeInfo->mLocalTimeSpan.GetStop();
			}
			else
			{
				FbxTimeSpan lTimeLineTimeSpan;
				tStart.SetSecondDouble(FLT_MAX);
				tStop.SetSecondDouble(-FLT_MAX);
			}

			FbxTime::EMode pTimeMode = m_pScene->GetGlobalSettings().GetTimeMode();
			double frameRate = FbxTime::GetFrameRate(pTimeMode);

			// TimeSet을 저장
			FbxAnimCurve* pCurve = nullptr;
			std::set<FbxTime> timeSet;

			// TRANS
			pCurve = pNode->LclTranslation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_X);
			FbxUtil::InsertTimeset(timeSet, pCurve, tStart, tStop);
			pCurve = pNode->LclTranslation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_Y);
			FbxUtil::InsertTimeset(timeSet, pCurve, tStart, tStop);
			pCurve = pNode->LclTranslation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_Z);
			FbxUtil::InsertTimeset(timeSet, pCurve, tStart, tStop);

			// SCALE
			pCurve = pNode->LclScaling.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_X);
			FbxUtil::InsertTimeset(timeSet, pCurve, tStart, tStop);
			pCurve = pNode->LclScaling.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_Y);
			FbxUtil::InsertTimeset(timeSet, pCurve, tStart, tStop);
			pCurve = pNode->LclScaling.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_Z);
			FbxUtil::InsertTimeset(timeSet, pCurve, tStart, tStop);

			// ROTATE
			pCurve = pNode->LclRotation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_X);
			FbxUtil::InsertTimeset(timeSet, pCurve, tStart, tStop);
			pCurve = pNode->LclRotation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_Y);
			FbxUtil::InsertTimeset(timeSet, pCurve, tStart, tStop);
			pCurve = pNode->LclRotation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_Z);
			FbxUtil::InsertTimeset(timeSet, pCurve, tStart, tStop);

			// 애니메이션 키값
			FbxVector4 localT = pNode->LclTranslation.Get();
			FbxVector4 localR = pNode->LclRotation.Get();
			FbxVector4 localS = pNode->LclScaling.Get();

			bool isFirst = true;
			Math::Quaternion lastQuat = Math::Quaternion::Identity;

			std::vector<Motion::Keyframe> vecKeyframes;
			vecKeyframes.resize(timeSet.size());

			uint32_t nIdx = 0;
			for (const auto& iter : timeSet)
			{
				const FbxTime &pTime = iter;

				FbxAMatrix localMatrix = FbxUtil::GetLocalTransform(pNode, pTime);

				Motion::Keyframe& keyframe = vecKeyframes[nIdx++];
				keyframe.fTimePos = static_cast<float>((pTime.GetSecondDouble() * frameRate) / frameRate);

				// ScaleKey
				FbxVector4 scale = localMatrix.GetS();
				Math::Vector3 f3Scale;
				f3Scale.x = static_cast<float>(scale[0]);
				f3Scale.y = static_cast<float>(scale[1]);
				f3Scale.z = static_cast<float>(scale[2]);

				// TransKey
				FbxVector4 trans = localMatrix.GetT();
				if (_isnan(trans[0]) || !_finite(trans[0]))
				{
					trans[0] = localT[0];
				}
				if (_isnan(trans[1]) || !_finite(trans[1]))
				{
					trans[1] = localT[1];
				}
				if (_isnan(trans[2]) || !_finite(trans[2]))
				{
					trans[2] = localT[2];
				}

				Math::Vector3 f3Pos;
				f3Pos.x = static_cast<float>(trans[0]);
				f3Pos.y = static_cast<float>(trans[1]);
				f3Pos.z = static_cast<float>(trans[2]);

				//keyframe.f3Pos = Math::Vector3::Transform(keyframe.f3Pos, Math::Matrix::ZConversion);

				// RotationKey
				FbxQuaternion localQ = localMatrix.GetQ();

				Math::Quaternion quat;
				quat.x = static_cast<float>(localQ[0]);
				quat.y = static_cast<float>(localQ[1]);
				quat.z = static_cast<float>(localQ[2]);
				quat.w = static_cast<float>(localQ[3]);

				if (isFirst == false)
				{
					if (quat.Dot(lastQuat) < 0.f)
					{
						quat = -quat;
					}
				}

				isFirst = false;
				lastQuat = quat;

				keyframe.f3Pos = f3Pos;
				keyframe.f3Scale = f3Scale;
				keyframe.quatRotation = quat;
				//Math::Matrix::Compose(f3Scale, quat, f3Pos, keyframe.matTransform);
			}

			if (vecKeyframes.empty())
			{
				FbxAMatrix defaultMatrix = FbxUtil::GetLocalTransform(pNode);

				Motion::Keyframe keyframe;
				keyframe.fTimePos = 0.f;

				FbxVector4 scale = defaultMatrix.GetS();
				Math::Vector3 f3Scale;
				f3Scale.x = static_cast<float>(scale[0]);
				f3Scale.y = static_cast<float>(scale[1]);
				f3Scale.z = static_cast<float>(scale[2]);

				FbxVector4 trans = defaultMatrix.GetT();
				Math::Vector3 f3Pos;
				f3Pos.x = static_cast<float>(trans[0]);
				f3Pos.y = static_cast<float>(trans[1]);
				f3Pos.z = static_cast<float>(trans[2]);

				FbxQuaternion localQ = defaultMatrix.GetQ();
				Math::Quaternion quat;
				quat.x = static_cast<float>(localQ[0]);
				quat.y = static_cast<float>(localQ[1]);
				quat.z = static_cast<float>(localQ[2]);
				quat.w = static_cast<float>(localQ[3]);

				keyframe.f3Pos = f3Pos;
				keyframe.f3Scale = f3Scale;
				keyframe.quatRotation = quat;
				//Math::Matrix::Compose(f3Scale, quat, f3Pos, keyframe.matTransform);
				vecKeyframes.emplace_back(keyframe);
			}

			Motion* pRealMotion = static_cast<Motion*>(pMotion);
			pRealMotion->AddBoneKeyframes(pNode->GetName(), vecKeyframes);
		}
	}
}