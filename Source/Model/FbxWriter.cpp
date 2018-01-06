#include "stdafx.h"
#include "FbxWriter.h"

#include "Model.h"
#include "ModelNodeSkinned.h"
#include "ModelNodeStatic.h"
#include "Motion.h"

#include "Skeleton.h"

#include "CommonLib/FileUtil.h"

#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXCollision.h>

#include "ExternLib/FBXExporter/ExportObjects/ExportXmlParser.h"
#include "ExternLib/FBXExporter/ExportObjects/ExportPath.h"
#include "ExternLib/FBXExporter/ExportObjects/ExportMaterial.h"
#include "ExternLib/FBXExporter/ExportObjects/ExportObjects.h"

using namespace ATG;

extern ATG::ExportScene* g_pScene;
extern ExportPath g_CurrentInputFileName;

namespace EastEngine
{
	namespace Graphics
	{
		// 언젠가는 고정된 포맷이 아닌, D3DDECLUSAGE 에 대응해서 다양한 포맷에 대응할 수 있는 기능을 만들어야한다.
		template <typename T>
		IVertexBuffer* WriteVertexBuffer(ExportVB* pVB, const D3DVERTEXELEMENT9* pVertexElements, size_t nVertexElementCount)
		{
			const size_t nVertexCount = pVB->GetVertexCount();
			if (nVertexCount == 0)
				return nullptr;

			std::vector<T> vecVertices;
			vecVertices.resize(nVertexCount);

			const size_t stride = sizeof(T);

			for (size_t i = 0; i < nVertexCount; ++i)
			{
				uint8_t* pVertex = pVB->GetVertex(i);

				uint8_t* pData = reinterpret_cast<uint8_t*>(&vecVertices[i]);

				for (size_t j = 0; j < nVertexElementCount; j++)
				{
					const uint8_t* pVertexData = pVertex + pVertexElements[j].Offset;

					// 현재 포맷에서 지원하지 않는 포맷은 assert, 나중에 개선하도록 하자
					switch (pVertexElements[j].Usage)
					{
					case D3DDECLUSAGE::D3DDECLUSAGE_POSITION:
					{
						assert(pVertexElements[j].Type == D3DDECLTYPE::D3DDECLTYPE_FLOAT3);
						Math::Vector3* pPos = reinterpret_cast<Math::Vector3*>(pData);
						*pPos = *reinterpret_cast<const Math::Vector3*>(pVertexData);
					}
					break;
					case D3DDECLUSAGE::D3DDECLUSAGE_TEXCOORD:
					{
						assert(pVertexElements[j].Type == D3DDECLTYPE::D3DDECLTYPE_FLOAT2);
						const size_t nOffset = sizeof(Math::Vector3);
						Math::Vector2* pUV = reinterpret_cast<Math::Vector2*>(pData + nOffset);
						*pUV = *reinterpret_cast<const Math::Vector2*>(pVertexData);
					}
					break;
					case D3DDECLUSAGE::D3DDECLUSAGE_NORMAL:
					{
						assert(pVertexElements[j].Type == D3DDECLTYPE::D3DDECLTYPE_FLOAT3);
						const size_t nOffset = sizeof(Math::Vector3) + sizeof(Math::Vector2);
						Math::Vector3* pNormal = reinterpret_cast<Math::Vector3*>(pData + nOffset);
						*pNormal = *reinterpret_cast<const Math::Vector3*>(pVertexData);
					}
					break;
					case D3DDECLUSAGE::D3DDECLUSAGE_BLENDWEIGHT:
					{
						assert(pVertexElements[j].Type == D3DDECLTYPE::D3DDECLTYPE_UBYTE4N);
						const size_t nOffset = sizeof(Math::Vector3) + sizeof(Math::Vector2) + sizeof(Math::Vector3);
						const Math::UByte4* pBlendData = reinterpret_cast<const Math::UByte4*>(pVertexData);
						Math::Vector3* pBlend = reinterpret_cast<Math::Vector3*>(pData + nOffset);
						*pBlend = Math::Vector3(static_cast<float>(pBlendData->x) / 255.f, static_cast<float>(pBlendData->y) / 255.f, static_cast<float>(pBlendData->z) / 255.f);
					}
					break;
					case D3DDECLUSAGE::D3DDECLUSAGE_BLENDINDICES:
					{
						assert(pVertexElements[j].Type == D3DDECLTYPE::D3DDECLTYPE_UBYTE4);
						const size_t nOffset = sizeof(Math::Vector3) + sizeof(Math::Vector2) + sizeof(Math::Vector3) + sizeof(Math::Vector3);
						Math::UByte4* pIndex = reinterpret_cast<Math::UByte4*>(pData + nOffset);
						*pIndex = *reinterpret_cast<const Math::UByte4*>(pVertexData);
					}
					break;
					default:
						break;
					}
				}
			}

			return IVertexBuffer::Create(T::Format(), vecVertices.size(), vecVertices.data(), D3D11_USAGE::D3D11_USAGE_DYNAMIC);
		}

		IIndexBuffer* WriteIndexBuffer(ExportIB* pIB)
		{
			const size_t nIndexCount = pIB->GetIndexCount();
			if (nIndexCount == 0)
				return nullptr;

			if (pIB->GetIndexSize() == 2)
			{
				// 지원안함 ㅠ.ㅠ
				assert(false);
			}
			else
			{
				const uint32_t* pIndices = reinterpret_cast<const uint32_t*>(pIB->GetIndexData());
				return IIndexBuffer::Create(nIndexCount, pIndices, D3D11_USAGE_DYNAMIC);
			}

			return nullptr;
		}

		void WriteIBSubset(ExportIBSubset* pSubset, ModelSubset& subset)
		{
			switch (pSubset->GetPrimitiveType())
			{
			case ExportIBSubset::TriangleList:
				subset.emPrimitiveType = EmPrimitive::eTriangleList;
				break;
			case ExportIBSubset::TriangleStrip:
				subset.emPrimitiveType = EmPrimitive::eTriangleStrip;
				break;
			case ExportIBSubset::QuadList:
				subset.emPrimitiveType = EmPrimitive::eQuadPatchList;
				break;
			}

			subset.strName = pSubset->GetName().SafeString();
			subset.nStartIndex = pSubset->GetStartIndex();
			subset.nIndexCount = pSubset->GetIndexCount();
		}

		void WriteMaterial(ExportMaterial* pExportMaterial, MaterialInfo& materialInfo)
		{
			materialInfo.strName = pExportMaterial->GetName().SafeString();

			ExportMaterialParameter* pColor = pExportMaterial->FindParameter("DiffuseColor");
			if (pColor != nullptr)
			{
				materialInfo.colorAlbedo = Math::Color(pColor->ValueFloat[0], pColor->ValueFloat[1], pColor->ValueFloat[2], pColor->ValueFloat[3]);
			}
			else
			{
				materialInfo.colorAlbedo = Math::Color::White;
			}

			// 없음
			// pColor = pExportMaterial->FindParameter("AmbientColor");

			pColor = pExportMaterial->FindParameter("EmissiveColor");
			if (pColor != nullptr)
			{
				materialInfo.colorEmissive = Math::Color(pColor->ValueFloat[0], pColor->ValueFloat[1], pColor->ValueFloat[2], 0.f);
			}
			else
			{
				materialInfo.colorEmissive = Math::Color::Transparent;
			}

			// SpecularColor는 albedo 컬러에서 계산하거나 texture 만 사용함
			pColor = pExportMaterial->FindParameter("SpecularColor");

			ExportMaterialParameter* pDiffuse = pExportMaterial->FindParameter("DiffuseTexture");
			if (pDiffuse != nullptr)
			{
				materialInfo.strTextureNameArray[EmMaterial::eAlbedo] = pDiffuse->ValueString.SafeString();
			}

			ExportMaterialParameter* pNormal = pExportMaterial->FindParameter("NormalMapTexture");
			if (pNormal != nullptr)
			{
				materialInfo.strTextureNameArray[EmMaterial::eNormal] = pNormal->ValueString.SafeString();
			}

			ExportMaterialParameter* pSpecular = pExportMaterial->FindParameter("SpecularMapTexture");
			if (pSpecular != nullptr)
			{
				materialInfo.strTextureNameArray[EmMaterial::eSpecular] = pSpecular->ValueString.SafeString();
			}
		}

		ModelNode* CreateModelNode(ExportModel* pExportModel, Model* pModel)
		{
			ATG::ExportMesh* pMesh = static_cast<ATG::ExportMesh*>(pExportModel->GetMesh());
			if (pMesh == nullptr)
				return nullptr;

			Collision::AABB aabb;

			switch (pMesh->GetSmallestBound())
			{
			case ExportMeshBase::SphereBound:
				aabb.Center = *reinterpret_cast<Math::Vector3*>(&pMesh->GetBoundingSphere().Center);
				aabb.Extents = Math::Vector3(pMesh->GetBoundingSphere().Radius);
				break;
			case ExportMeshBase::AxisAlignedBoxBound:
				aabb.Center = *reinterpret_cast<Math::Vector3*>(&pMesh->GetBoundingAABB().Center);
				aabb.Extents = *reinterpret_cast<Math::Vector3*>(&pMesh->GetBoundingAABB().Extents);
				break;
			}

			int nVertexBufferCount = 0;

			switch (pMesh->GetMeshType())
			{
			case ExportMeshBase::PolyMesh:
			{
				ModelNode* pModelNode = nullptr;

				ExportSubDProcessMesh* pSubDMesh = pMesh->GetSubDMesh();
				if (pSubDMesh != nullptr)
				{
					nVertexBufferCount = 2;
				}
				else
				{
					nVertexBufferCount = 1;

					IVertexBuffer* pVertexBuffer = nullptr;
					IIndexBuffer* pIndexBuffer = nullptr;

					bool isSkinned = pMesh->GetInfluenceCount() > 0;
					if (isSkinned == true)
					{
						ModelNodeSkinned* pSkinnedNode = new ModelNodeSkinned;
						pModelNode = pSkinnedNode;

						pVertexBuffer = WriteVertexBuffer<VertexPosTexNorWeiIdx>(pMesh->GetVB(), &pMesh->GetVertexDeclElement(0), pMesh->GetVertexDeclElementCount());
						pIndexBuffer = WriteIndexBuffer(pMesh->GetIB());

						const size_t nInfluenceCount = pMesh->GetInfluenceCount();
						std::vector<String::StringID> vecBoneNames;
						vecBoneNames.resize(nInfluenceCount);

						for (size_t i = 0; i < nInfluenceCount; ++i)
						{
							ExportString strName = pMesh->GetInfluence(i);
							vecBoneNames[i] = strName.SafeString();
						}

						pSkinnedNode->SetBoneNameList(vecBoneNames);
					}
					else
					{
						pModelNode = new ModelNodeStatic;

						pVertexBuffer = WriteVertexBuffer<VertexPosTexNor>(pMesh->GetVB(), &pMesh->GetVertexDeclElement(0), pMesh->GetVertexDeclElementCount());
						pIndexBuffer = WriteIndexBuffer(pMesh->GetIB());
					}

					pModelNode->SetNodeName(pMesh->GetName().SafeString());
					pModelNode->SetVisible(true);

					if (pVertexBuffer == nullptr || pIndexBuffer == nullptr)
					{
						LOG_ERROR("Can't created buffer(S/F), Vertex %s, Index %s", pVertexBuffer != nullptr ? "S" : "F", pIndexBuffer != nullptr ? "S" : "F");
					}

					pModelNode->SetVertexBuffer(pVertexBuffer);
					pModelNode->SetIndexBuffer(pIndexBuffer);

					const size_t nBindingCount = pExportModel->GetBindingCount();

					std::vector<ModelSubset> vecSubsets;
					vecSubsets.resize(nBindingCount);

					std::vector<IMaterial*> vecMaterials;

					for (size_t i = 0; i < nBindingCount; ++i)
					{
						ExportMaterialSubsetBinding* pBinding = pExportModel->GetBinding(i);

						ExportIBSubset* pSubset = pMesh->FindSubset(pBinding->SubsetName);
						WriteIBSubset(pSubset, vecSubsets[i]);

						const size_t nMaterialCount = vecMaterials.size();
						for (size_t j = 0; j < nMaterialCount; ++j)
						{
							if (String::IsEquals(vecMaterials[j]->GetName().c_str(), pBinding->pMaterial->GetName()) == true)
							{
								vecSubsets[i].nMaterialID = j;
								break;
							}
						}

						if (vecSubsets[i].nMaterialID == std::numeric_limits<uint32_t>::max())
						{
							vecSubsets[i].nMaterialID = vecMaterials.size();

							MaterialInfo materialInfo;
							materialInfo.strPath = File::GetFilePath(pModel->GetFilePath());
							WriteMaterial(pBinding->pMaterial, materialInfo);

							vecMaterials.emplace_back(IMaterial::Create(&materialInfo));
						}
					}

					pModelNode->AddModelSubsets(vecSubsets);

					if (vecMaterials.empty() == false)
					{
						pModelNode->AddMaterialArray(vecMaterials.data(), vecMaterials.size());
					}

					pModelNode->SetOriginAABB(aabb);
				}

				return pModelNode;
			}
			break;
			}

			return nullptr;
		}

		void CreateModel(ExportFrame* pFrame, Model* pModel, ModelNode* pParentNode, const std::unordered_map<String::StringID, Math::Matrix>& umapMotionOffset, Skeleton* pSkeleton, const String::StringID& strParentBoneName)
		{
			ModelNode* pModelNode = nullptr;
			String::StringID strBoneName;

			const size_t nModelCount = pFrame->GetModelCount();
			if (nModelCount > 0)
			{
				if (nModelCount > 2)
				{
					ExportLog::LogWarning("Frame \"%s\" has %Iu meshes.  Only one mesh per frame is supported in the SDKMesh format.", pFrame->GetName().SafeString(), nModelCount);
				}

				ExportModel* pExportModel = pFrame->GetModelByIndex(0);
				pModelNode = CreateModelNode(pExportModel, pModel);
				if (pModelNode != nullptr)
				{
					if (pParentNode != nullptr)
					{
						pParentNode->AddChildNode(pModelNode);
						pModelNode->SetParentNode(pParentNode);
						pModel->AddNode(pModelNode, pModelNode->GetName(), false);
					}
					else
					{
						pModel->AddNode(pModelNode, pModelNode->GetName(), true);

						if (strParentBoneName.empty() == false)
						{
							pModelNode->SetAttachedBoneName(strParentBoneName);
						}
					}
				}
			}
			else if (pFrame->GetName() != nullptr)
			{
				String::StringID strName = pFrame->GetName();
				auto iter = umapMotionOffset.find(strName);
				if (iter != umapMotionOffset.end())
				{
					if (pSkeleton == nullptr)
					{
						pSkeleton = static_cast<Skeleton*>(ISkeleton::Create());
						pModel->SetSkeleton(pSkeleton);
					}

					const Math::Matrix& matMotionOffset = iter->second;
					const Math::Matrix& matDefaultMotionData = *reinterpret_cast<const Math::Matrix*>(&pFrame->Transform().Matrix());

					strBoneName = pFrame->GetName().SafeString();

					if (strParentBoneName.empty() == true)
					{
						pSkeleton->CreateBone(strBoneName, matMotionOffset, matDefaultMotionData);
					}
					else
					{
						pSkeleton->CreateBone(strParentBoneName, strBoneName, matMotionOffset, matDefaultMotionData);
					}
				}
			}

			const size_t nChildCount = pFrame->GetChildCount();
			for (size_t i = 0; i < nChildCount; ++i)
			{
				ExportFrame* pChildFrame = pFrame->GetChildByIndex(i);
				CreateModel(pChildFrame, pModel, pModelNode, umapMotionOffset, pSkeleton, strBoneName);
			}
		}

		bool WriteModel(IModel* pModel, const std::unordered_map<String::StringID, Math::Matrix>& umapMotionOffset)
		{
			if (g_pScene == nullptr || pModel == nullptr)
				return false;

			Model* pRealModel = static_cast<Model*>(pModel);
			CreateModel(g_pScene, pRealModel, nullptr, umapMotionOffset, nullptr, String::StringID());

			Skeleton* pSkeleton = static_cast<Skeleton*>(pModel->GetSkeleton());
			if (pSkeleton != nullptr && pSkeleton->GetBoneCount() > 0)
			{
				const size_t nNodeCount = pModel->GetNodeCount();
				for (size_t i = 0; i < nNodeCount; ++i)
				{
					IModelNode* pModelNode = pModel->GetNode(i);
					if (pModelNode == nullptr)
						continue;

					if (pModelNode->GetType() != EmModelNode::eSkinned)
						continue;

					ModelNodeSkinned* pSkinnedNode = static_cast<ModelNodeSkinned*>(pModelNode);

					const size_t nBoneCount = pSkinnedNode->GetBoneCount();

					std::vector<String::StringID> vecBoneNames;
					vecBoneNames.resize(nBoneCount);

					for (size_t j = 0; j < nBoneCount; ++j)
					{
						vecBoneNames[j] = pSkinnedNode->GetBoneName(j);
					}

					pSkeleton->SetSkinnedList(pSkinnedNode->GetName(), vecBoneNames.data(), vecBoneNames.size());
				}
			}

			return true;
		}

		bool SamplePositionData(ExportAnimationPositionKey* pKeys, size_t nKeyCount, Motion::Keyframe* pDestKeys, size_t nDestKeyCount, float fKeyInterval)
		{
			if (nKeyCount == 0)
				return false;

			size_t nCurrentSrcKey = 0;
			bool isEndKey = false;
			ExportAnimationPositionKey StartKey = pKeys[nCurrentSrcKey];
			ExportAnimationPositionKey EndKey = {};
			if (nKeyCount > 1)
			{
				isEndKey = true;
				EndKey = pKeys[nCurrentSrcKey + 1];
			}

			float fTime = 0;
			for (size_t i = 0; i < nDestKeyCount; ++i)
			{
				while (isEndKey && fTime >= EndKey.fTime)
				{
					StartKey = EndKey;
					++nCurrentSrcKey;
					if (nCurrentSrcKey >= nKeyCount)
					{
						isEndKey = false;
					}
					else
					{
						EndKey = pKeys[nCurrentSrcKey];
					}
				}

				if (isEndKey == false)
				{
					pDestKeys[i].f3Pos = *reinterpret_cast<Math::Vector3*>(&StartKey.Position);
				}
				else
				{
					assert(fTime <= EndKey.fTime);
					float fLerpFactor = (fTime - StartKey.fTime) / (EndKey.fTime - StartKey.fTime);
					fLerpFactor = std::min(std::max(0.f, fLerpFactor), 1.f);

					DirectX::XMVECTOR v = DirectX::XMLoadFloat3(&StartKey.Position);
					DirectX::XMVECTOR vEnd = DirectX::XMLoadFloat3(&EndKey.Position);

					v = DirectX::XMVectorLerp(v, vEnd, fLerpFactor);

					DirectX::XMStoreFloat3(reinterpret_cast<DirectX::XMFLOAT3*>(&pDestKeys[i].f3Pos), v);
				}

				fTime += fKeyInterval;
			}

			return true;
		}

		bool SampleOrientationData(ExportAnimationOrientationKey* pKeys, size_t nKeyCount, Motion::Keyframe* pDestKeys, size_t nDestKeyCount, float fKeyInterval)
		{
			if (nKeyCount == 0)
				return false;

			size_t nCurrentSrcKey = 0;
			bool isEndKey = false;
			ExportAnimationOrientationKey StartKey = pKeys[nCurrentSrcKey];
			ExportAnimationOrientationKey EndKey = {};
			if (nKeyCount > 1)
			{
				isEndKey = true;
				EndKey = pKeys[nCurrentSrcKey + 1];
			}

			float fTime = 0;
			for (size_t i = 0; i < nDestKeyCount; ++i)
			{
				while (isEndKey && fTime >= EndKey.fTime)
				{
					StartKey = EndKey;
					++nCurrentSrcKey;
					if (nCurrentSrcKey >= nKeyCount)
					{
						isEndKey = false;
					}
					else
					{
						EndKey = pKeys[nCurrentSrcKey];
					}
				}

				if (isEndKey == false)
				{
					pDestKeys[i].quatRotation = *reinterpret_cast<Math::Quaternion*>(&StartKey.Orientation);
				}
				else
				{
					assert(fTime <= EndKey.fTime);
					float fLerpFactor = (fTime - StartKey.fTime) / (EndKey.fTime - StartKey.fTime);
					fLerpFactor = std::min(std::max(0.f, fLerpFactor), 1.f);

					DirectX::XMVECTOR v = DirectX::XMLoadFloat4(&StartKey.Orientation);
					DirectX::XMVECTOR vEnd = DirectX::XMLoadFloat4(&EndKey.Orientation);

					v = DirectX::XMVectorLerp(v, vEnd, fLerpFactor);

					DirectX::XMStoreFloat4(reinterpret_cast<DirectX::XMFLOAT4*>(&pDestKeys[i].quatRotation), v);
				}

				fTime += fKeyInterval;
			}

			return true;
		}

		bool SampleScaleData(ExportAnimationScaleKey* pKeys, size_t nKeyCount, Motion::Keyframe* pDestKeys, size_t nDestKeyCount, float fKeyInterval)
		{
			if (nKeyCount == 0)
				return false;

			size_t nCurrentSrcKey = 0;
			bool isEndKey = false;
			ExportAnimationScaleKey StartKey = pKeys[nCurrentSrcKey];
			ExportAnimationScaleKey EndKey = {};
			if (nKeyCount > 1)
			{
				isEndKey = true;
				EndKey = pKeys[nCurrentSrcKey + 1];
			}

			float fTime = 0;
			for (size_t i = 0; i < nDestKeyCount; ++i)
			{
				while (isEndKey && fTime >= EndKey.fTime)
				{
					StartKey = EndKey;
					++nCurrentSrcKey;
					if (nCurrentSrcKey >= nKeyCount)
					{
						isEndKey = false;
					}
					else
					{
						EndKey = pKeys[nCurrentSrcKey];
					}
				}

				if (isEndKey == false)
				{
					pDestKeys[i].f3Scale = *reinterpret_cast<Math::Vector3*>(&StartKey.Scale);
				}
				else
				{
					assert(fTime <= EndKey.fTime);
					float fLerpFactor = (fTime - StartKey.fTime) / (EndKey.fTime - StartKey.fTime);
					fLerpFactor = std::min(std::max(0.f, fLerpFactor), 1.f);

					DirectX::XMVECTOR v = DirectX::XMLoadFloat3(&StartKey.Scale);
					DirectX::XMVECTOR vEnd = DirectX::XMLoadFloat3(&EndKey.Scale);

					v = DirectX::XMVectorLerp(v, vEnd, fLerpFactor);

					DirectX::XMStoreFloat3(reinterpret_cast<DirectX::XMFLOAT3*>(&pDestKeys[i].f3Scale), v);
				}

				fTime += fKeyInterval;
			}

			return true;
		}

		bool SampleTimeData(Motion::Keyframe* pDestKeys, size_t nDestKeyCount, float fKeyInterval, float fStartTime)
		{
			float fTime = 0.f;
			for (size_t i = 0; i < nDestKeyCount; ++i)
			{
				pDestKeys[i].fTime = fStartTime + fTime;

				fTime += fKeyInterval;
			}

			return true;
		}

		void WriteKeyframes(ExportAnimationTrack* pTrack, Motion* pMotion, float fDuration, float fFrameInterval, float fStartTime)
		{
			std::vector<Motion::Keyframe> vecKeyframes;
			ExportAnimationTransformTrack& transformTrack = pTrack->TransformTrack;

			String::StringID strName;
			if (transformTrack.pSourceFrame != nullptr)
			{
				strName = transformTrack.pSourceFrame->GetName().SafeString();
			}
			else
			{
				strName = pTrack->GetName().SafeString();
			}

			if (transformTrack.IsTrackEmpty() == false)
			{
				size_t nKeyframeCount = static_cast<size_t>(fDuration / fFrameInterval) + 1;
				
				vecKeyframes.resize(nKeyframeCount);
				
				SampleTimeData(vecKeyframes.data(), vecKeyframes.size(), fFrameInterval, fStartTime);
				SamplePositionData(transformTrack.GetPositionKeys(), transformTrack.GetPositionKeyCount(), vecKeyframes.data(), vecKeyframes.size(), fFrameInterval);
				SampleOrientationData(transformTrack.GetOrientationKeys(), transformTrack.GetOrientationKeyCount(), vecKeyframes.data(), vecKeyframes.size(), fFrameInterval);
				SampleScaleData(transformTrack.GetScaleKeys(), transformTrack.GetScaleKeyCount(), vecKeyframes.data(), vecKeyframes.size(), fFrameInterval);
			}
			else
			{
				vecKeyframes.emplace_back();
			}

			pMotion->AddBoneKeyframes(strName, vecKeyframes);
		}

		bool WriteMotion(IMotion* pMotion)
		{
			if (g_pScene == nullptr || pMotion == nullptr)
				return false;

			// 이 로직에서는 애니메이션 하나만 추출한다.
			ExportAnimation* pAnimation = g_pScene->GetAnimation(0);
			if (pAnimation == nullptr)
				return false;

			Motion* pRealMotion = static_cast<Motion*>(pMotion);
			pRealMotion->SetInfo(pAnimation->fStartTime, pAnimation->fEndTime, pAnimation->fSourceSamplingInterval);

			size_t nTrackCount = pAnimation->GetTrackCount();
			for (size_t i = 0; i < nTrackCount; ++i)
			{
				ExportAnimationTrack* pTrack = pAnimation->GetTrack(i);
				if (pTrack == nullptr)
					continue;

				WriteKeyframes(pTrack, pRealMotion, pAnimation->GetDuration(), pAnimation->fSourceSamplingInterval, pAnimation->fStartTime);
			}

			return true;
		}
	}
}