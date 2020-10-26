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

namespace est
{
	namespace graphics
	{
		// 언젠가는 고정된 포맷이 아닌, D3DDECLUSAGE 에 대응해서 다양한 포맷에 대응할 수 있는 기능을 만들어야한다.
		template <typename T>
		VertexBufferPtr WriteVertexBuffer(ExportVB* pVB, const D3DVERTEXELEMENT9* pVertexElements, size_t nVertexElementCount, std::vector<VertexPos>& rawVertices_out)
		{
			const size_t vertexCount = pVB->GetVertexCount();
			if (vertexCount == 0)
				return nullptr;

			std::vector<T> vecVertices;
			vecVertices.resize(vertexCount);

			rawVertices_out.resize(vertexCount);

			const size_t stride = sizeof(T);

			for (size_t i = 0; i < vertexCount; ++i)
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
						math::float3* pPos = reinterpret_cast<math::float3*>(pData);
						*pPos = *reinterpret_cast<const math::float3*>(pVertexData);

						rawVertices_out.emplace_back(*pPos);
					}
					break;
					case D3DDECLUSAGE::D3DDECLUSAGE_TEXCOORD:
					{
						assert(pVertexElements[j].Type == D3DDECLTYPE::D3DDECLTYPE_FLOAT2);
						const size_t nOffset = sizeof(math::float3);
						math::float2* pUV = reinterpret_cast<math::float2*>(pData + nOffset);
						*pUV = *reinterpret_cast<const math::float2*>(pVertexData);
					}
					break;
					case D3DDECLUSAGE::D3DDECLUSAGE_NORMAL:
					{
						assert(pVertexElements[j].Type == D3DDECLTYPE::D3DDECLTYPE_FLOAT3);
						const size_t nOffset = sizeof(math::float3) + sizeof(math::float2);
						math::float3* pNormal = reinterpret_cast<math::float3*>(pData + nOffset);
						*pNormal = *reinterpret_cast<const math::float3*>(pVertexData);
					}
					break;
					case D3DDECLUSAGE::D3DDECLUSAGE_BLENDWEIGHT:
					{
						assert(pVertexElements[j].Type == D3DDECLTYPE::D3DDECLTYPE_UBYTE4N);
						const size_t nOffset = sizeof(math::float3) + sizeof(math::float2) + sizeof(math::float3);
						const math::UByte4* pBlendData = reinterpret_cast<const math::UByte4*>(pVertexData);
						math::float3* pBlend = reinterpret_cast<math::float3*>(pData + nOffset);
						*pBlend = math::float3(static_cast<float>(pBlendData->x) / 255.f, static_cast<float>(pBlendData->y) / 255.f, static_cast<float>(pBlendData->z) / 255.f);
					}
					break;
					case D3DDECLUSAGE::D3DDECLUSAGE_BLENDINDICES:
					{
						assert(pVertexElements[j].Type == D3DDECLTYPE::D3DDECLTYPE_UBYTE4);
						const size_t nOffset = sizeof(math::float3) + sizeof(math::float2) + sizeof(math::float3) + sizeof(math::float3);
						const math::UByte4* pIndexData = reinterpret_cast<const math::UByte4*>(pVertexData);
						uint16_t* pIndex = reinterpret_cast<uint16_t*>(pData + nOffset);
						pIndex[0] = static_cast<uint16_t>(pIndexData->x);
						pIndex[1] = static_cast<uint16_t>(pIndexData->y);
						pIndex[2] = static_cast<uint16_t>(pIndexData->z);
						pIndex[3] = static_cast<uint16_t>(pIndexData->w);
					}
					break;
					default:
						break;
					}
				}
			}

			return CreateVertexBuffer(reinterpret_cast<uint8_t*>(vecVertices.data()), static_cast<uint32_t>(vecVertices.size()), sizeof(T), true);
		}

		IndexBufferPtr WriteIndexBuffer(ExportIB* pIB, std::vector<uint32_t>& rawIndices_out)
		{
			const size_t indexCount = pIB->GetIndexCount();
			if (indexCount == 0)
				return nullptr;

			if (pIB->GetIndexSize() == 2)
			{
				// 지원안함 ㅠ.ㅠ
				assert(false);
			}
			else
			{
				const uint8_t* pIndices = reinterpret_cast<const uint8_t*>(pIB->GetIndexData());

				const uint32_t* pRawIndicex = reinterpret_cast<const uint32_t*>(pIndices);
				for (uint32_t i = 0; i < indexCount; ++i)
				{
					rawIndices_out.emplace_back(pRawIndicex[i]);
				}

				return CreateIndexBuffer(pIndices, static_cast<uint32_t>(indexCount), sizeof(uint32_t), true);
			}

			return nullptr;
		}

		void WriteIBSubset(ExportIBSubset* pSubset, ModelSubset& subset)
		{
			switch (pSubset->GetPrimitiveType())
			{
			case ExportIBSubset::TriangleList:
				subset.pimitiveType = Primitive::eTriangleList;
				break;
			case ExportIBSubset::TriangleStrip:
				subset.pimitiveType = Primitive::eTriangleStrip;
				break;
			case ExportIBSubset::QuadList:
				subset.pimitiveType = Primitive::eQuadPatchList;
				break;
			}

			subset.name = pSubset->GetName().SafeString();
			subset.startIndex = pSubset->GetStartIndex();
			subset.indexCount = pSubset->GetIndexCount();
		}

		void WriteMaterial(ExportMaterial* pExportMaterial, IMaterial::Data& materialData)
		{
			materialData.name = pExportMaterial->GetName().SafeString();

			ExportMaterialParameter* pColor = pExportMaterial->FindParameter("DiffuseColor");
			if (pColor != nullptr)
			{
				materialData.colorAlbedo = math::Color(pColor->ValueFloat[0], pColor->ValueFloat[1], pColor->ValueFloat[2], pColor->ValueFloat[3]);
			}
			else
			{
				materialData.colorAlbedo = math::Color::White;
			}

			// 없음
			// pColor = pExportMaterial->FindParameter("AmbientColor");

			pColor = pExportMaterial->FindParameter("EmissiveColor");
			if (pColor != nullptr)
			{
				materialData.colorEmissive = math::Color(pColor->ValueFloat[0], pColor->ValueFloat[1], pColor->ValueFloat[2], 0.f);
			}
			else
			{
				materialData.colorEmissive = math::Color::Transparent;
			}

			// SpecularColor는 albedo 컬러에서 계산하거나 texture 만 사용함
			pColor = pExportMaterial->FindParameter("SpecularColor");

			ExportMaterialParameter* pDiffuse = pExportMaterial->FindParameter("DiffuseTexture");
			if (pDiffuse != nullptr)
			{
				materialData.textureNameArray[IMaterial::eAlbedo] = pDiffuse->ValueString.SafeString();
			}

			ExportMaterialParameter* pNormal = pExportMaterial->FindParameter("NormalMapTexture");
			if (pNormal != nullptr)
			{
				materialData.textureNameArray[IMaterial::eNormal] = pNormal->ValueString.SafeString();
			}

			ExportMaterialParameter* pSpecular = pExportMaterial->FindParameter("SpecularMapTexture");
			if (pSpecular != nullptr)
			{
				materialData.textureNameArray[IMaterial::eSpecular] = pSpecular->ValueString.SafeString();
			}
		}

		std::unique_ptr<ModelNode> CreateModelNode(ExportModel* pExportModel, Model* pModel)
		{
			ATG::ExportMesh* pMesh = static_cast<ATG::ExportMesh*>(pExportModel->GetMesh());
			if (pMesh == nullptr)
				return nullptr;

			collision::AABB aabb;

			switch (pMesh->GetSmallestBound())
			{
			case ExportMeshBase::SphereBound:
				aabb.Center = *reinterpret_cast<math::float3*>(&pMesh->GetBoundingSphere().Center);
				aabb.Extents = math::float3(pMesh->GetBoundingSphere().Radius);
				break;
			case ExportMeshBase::AxisAlignedBoxBound:
				aabb.Center = *reinterpret_cast<math::float3*>(&pMesh->GetBoundingAABB().Center);
				aabb.Extents = *reinterpret_cast<math::float3*>(&pMesh->GetBoundingAABB().Extents);
				break;
			}

			int nVertexBufferCount = 0;

			switch (pMesh->GetMeshType())
			{
			case ExportMeshBase::PolyMesh:
			{
				std::unique_ptr<ModelNode> pModelNode;

				ExportSubDProcessMesh* pSubDMesh = pMesh->GetSubDMesh();
				if (pSubDMesh != nullptr)
				{
					nVertexBufferCount = 2;
				}
				else
				{
					nVertexBufferCount = 1;

					VertexBufferPtr pVertexBuffer = nullptr;
					IndexBufferPtr pIndexBuffer = nullptr;

					bool isSkinned = pMesh->GetInfluenceCount() > 0;
					if (isSkinned == true)
					{
						pModelNode = std::make_unique<ModelNodeSkinned>();
						ModelNodeSkinned* pSkinnedNode = static_cast<ModelNodeSkinned*>(pModelNode.get());

						std::vector<VertexPos> rawVertices;
						pVertexBuffer = WriteVertexBuffer<VertexPosTexNorWeiIdx>(pMesh->GetVB(), &pMesh->GetVertexDeclElement(0), pMesh->GetVertexDeclElementCount(), rawVertices);

						std::vector<uint32_t> rawIndices;
						pIndexBuffer = WriteIndexBuffer(pMesh->GetIB(), rawIndices);

						const size_t nInfluenceCount = pMesh->GetInfluenceCount();
						std::vector<string::StringID> boneNames;
						boneNames.resize(nInfluenceCount);

						for (size_t i = 0; i < nInfluenceCount; ++i)
						{
							ExportString strName = pMesh->GetInfluence(i);
							boneNames[i] = strName.SafeString();
						}

						pSkinnedNode->SetBoneNameList(boneNames);
					}
					else
					{
						pModelNode = std::make_unique<ModelNodeStatic>();

						std::vector<VertexPos> rawVertices;
						pVertexBuffer = WriteVertexBuffer<VertexPosTexNor>(pMesh->GetVB(), &pMesh->GetVertexDeclElement(0), pMesh->GetVertexDeclElementCount(), rawVertices);
						pModelNode->SetRawVertices(rawVertices.data(), rawVertices.size());

						std::vector<uint32_t> rawIndices;
						pIndexBuffer = WriteIndexBuffer(pMesh->GetIB(), rawIndices);
					}

					pModelNode->SetNodeName(pMesh->GetName().SafeString());
					pModelNode->SetVisible(true);

					if (pVertexBuffer == nullptr || pIndexBuffer == nullptr)
					{
						LOG_ERROR(L"Can't created buffer(S/F), Vertex %s, Index %s", pVertexBuffer != nullptr ? "S" : "F", pIndexBuffer != nullptr ? "S" : "F");
					}

					pModelNode->SetVertexBuffer(pVertexBuffer);
					pModelNode->SetIndexBuffer(pIndexBuffer);

					const size_t nBindingCount = pExportModel->GetBindingCount();

					std::vector<ModelSubset> vecSubsets;
					vecSubsets.resize(nBindingCount);

					std::vector<MaterialPtr> materials;

					for (size_t i = 0; i < nBindingCount; ++i)
					{
						ExportMaterialSubsetBinding* pBinding = pExportModel->GetBinding(i);
						const std::wstring materialName = string::MultiToWide(pBinding->pMaterial->GetName());

						ExportIBSubset* pSubset = pMesh->FindSubset(pBinding->SubsetName);
						WriteIBSubset(pSubset, vecSubsets[i]);

						const size_t nMaterialCount = materials.size();
						for (size_t j = 0; j < nMaterialCount; ++j)
						{
							if (string::IsEquals(materials[j]->GetName().c_str(), materialName.c_str()) == true)
							{
								vecSubsets[i].materialID = static_cast<uint32_t>(j);
								break;
							}
						}

						if (vecSubsets[i].materialID == std::numeric_limits<uint32_t>::max())
						{
							vecSubsets[i].materialID = static_cast<uint32_t>(materials.size());

							IMaterial::Data materialData;
							materialData.path = file::GetFilePath(pModel->GetFilePath());
							WriteMaterial(pBinding->pMaterial, materialData);

							materials.emplace_back(CreateMaterial(&materialData));
						}
					}

					pModelNode->AddModelSubsets(vecSubsets);

					if (materials.empty() == false)
					{
						pModelNode->AddMaterialArray(materials.data(), materials.size());
					}

					ReleaseResource(pVertexBuffer);
					ReleaseResource(pIndexBuffer);
					for (auto& pMaterial : materials)
					{
						ReleaseResource(pMaterial);
					}
					materials.clear();

					pModelNode->SetOriginAABB(aabb);
				}

				return pModelNode;
			}
			break;
			}

			return nullptr;
		}

		void CreateModel(ExportFrame* pFrame, Model* pModel, ModelNode* pParentNode, const tsl::robin_map<std::string, math::Matrix>& umapMotionOffset, Skeleton* pSkeleton, const string::StringID& parentBoneName)
		{
			std::unique_ptr<ModelNode> pModelNode;

			ModelNode* pNode = nullptr;
			string::StringID boneName;

			const size_t modelCount = pFrame->GetModelCount();
			if (modelCount > 0)
			{
				if (modelCount > 2)
				{
					ExportLog::LogWarning("Frame \"%s\" has %Iu meshes.  Only one mesh per frame is supported in the SDKMesh format.", pFrame->GetName().SafeString(), modelCount);
				}

				ExportModel* pExportModel = pFrame->GetModelByIndex(0);
				pModelNode = CreateModelNode(pExportModel, pModel);
				if (pModelNode != nullptr)
				{
					pNode = pModelNode.get();
					if (pParentNode != nullptr)
					{
						pModelNode->SetParentName(pParentNode->GetName());
						const string::StringID nodeName = pModelNode->GetName();
						pModel->AddNode(std::move(pModelNode), nodeName, pParentNode->GetName());
					}
					else
					{
						if (parentBoneName.empty() == false)
						{
							pModelNode->SetAttachedBoneName(parentBoneName);
						}

						const string::StringID nodeName = pModelNode->GetName();
						pModel->AddNode(std::move(pModelNode), nodeName);
					}
				}
			}
			else if (pFrame->GetName() != nullptr)
			{
				if (pSkeleton == nullptr)
				{
					pSkeleton = static_cast<Skeleton*>(pModel->GetSkeleton());
				}

				boneName = pFrame->GetName().SafeString();

				math::Matrix matMotionOffset;
				auto iter = umapMotionOffset.find(pFrame->GetName().SafeString());
				if (iter != umapMotionOffset.end())
				{
					matMotionOffset = iter->second;
				}

				const math::Matrix& matDefaultMotionData = *reinterpret_cast<const math::Matrix*>(&pFrame->Transform().Matrix());

				if (parentBoneName.empty() == true)
				{
					pSkeleton->CreateBone(boneName, matMotionOffset, matDefaultMotionData);
				}
				else
				{
					pSkeleton->CreateBone(boneName, parentBoneName, matMotionOffset, matDefaultMotionData);
				}
			}

			const size_t nChildCount = pFrame->GetChildCount();
			for (size_t i = 0; i < nChildCount; ++i)
			{
				ExportFrame* pChildFrame = pFrame->GetChildByIndex(i);
				CreateModel(pChildFrame, pModel, pNode, umapMotionOffset, pSkeleton, boneName);
			}
		}

		bool WriteModel(Model* pModel, const tsl::robin_map<std::string, math::Matrix>& umapMotionOffset)
		{
			if (g_pScene == nullptr || pModel == nullptr)
				return false;

			CreateModel(g_pScene, pModel, nullptr, umapMotionOffset, nullptr, string::StringID());

			Skeleton* pSkeleton = static_cast<Skeleton*>(pModel->GetSkeleton());
			if (pSkeleton != nullptr && pSkeleton->GetBoneCount() > 0)
			{
				const uint32_t nNodeCount = pModel->GetNodeCount();
				for (uint32_t i = 0; i < nNodeCount; ++i)
				{
					IModelNode* pModelNode = pModel->GetNode(i);
					if (pModelNode == nullptr)
						continue;

					if (pModelNode->GetType() != IModelNode::eSkinned)
						continue;

					ModelNodeSkinned* pSkinnedNode = static_cast<ModelNodeSkinned*>(pModelNode);

					const uint32_t nBoneCount = pSkinnedNode->GetBoneCount();

					std::vector<string::StringID> boneNames;
					boneNames.resize(nBoneCount);

					for (uint32_t j = 0; j < nBoneCount; ++j)
					{
						boneNames[j] = pSkinnedNode->GetBoneName(j);
					}

					pSkeleton->SetSkinnedList(pSkinnedNode->GetName(), boneNames.data(), boneNames.size());
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

			float time = 0;
			for (size_t i = 0; i < nDestKeyCount; ++i)
			{
				while (isEndKey && time >= EndKey.time)
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
					pDestKeys[i].transform.position = *reinterpret_cast<math::float3*>(&StartKey.Position);
				}
				else
				{
					assert(time <= EndKey.time);
					float fLerpFactor = (time - StartKey.time) / (EndKey.time - StartKey.time);
					fLerpFactor = std::min(std::max(0.f, fLerpFactor), 1.f);

					DirectX::XMVECTOR v = DirectX::XMLoadFloat3(&StartKey.Position);
					DirectX::XMVECTOR vEnd = DirectX::XMLoadFloat3(&EndKey.Position);

					v = DirectX::XMVectorLerp(v, vEnd, fLerpFactor);

					DirectX::XMStoreFloat3(reinterpret_cast<DirectX::XMFLOAT3*>(&pDestKeys[i].transform.position), v);
				}

				time += fKeyInterval;
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

			float time = 0;
			for (size_t i = 0; i < nDestKeyCount; ++i)
			{
				while (isEndKey && time >= EndKey.time)
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
					pDestKeys[i].transform.rotation = *reinterpret_cast<math::Quaternion*>(&StartKey.Orientation);
				}
				else
				{
					assert(time <= EndKey.time);
					float fLerpFactor = (time - StartKey.time) / (EndKey.time - StartKey.time);
					fLerpFactor = std::min(std::max(0.f, fLerpFactor), 1.f);

					DirectX::XMVECTOR v = DirectX::XMLoadFloat4(&StartKey.Orientation);
					DirectX::XMVECTOR vEnd = DirectX::XMLoadFloat4(&EndKey.Orientation);

					v = DirectX::XMVectorLerp(v, vEnd, fLerpFactor);

					DirectX::XMStoreFloat4(reinterpret_cast<DirectX::XMFLOAT4*>(&pDestKeys[i].transform.rotation), v);
				}

				time += fKeyInterval;
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

			float time = 0;
			for (size_t i = 0; i < nDestKeyCount; ++i)
			{
				while (isEndKey && time >= EndKey.time)
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
					pDestKeys[i].transform.scale = *reinterpret_cast<math::float3*>(&StartKey.Scale);
				}
				else
				{
					assert(time <= EndKey.time);
					float fLerpFactor = (time - StartKey.time) / (EndKey.time - StartKey.time);
					fLerpFactor = std::min(std::max(0.f, fLerpFactor), 1.f);

					DirectX::XMVECTOR v = DirectX::XMLoadFloat3(&StartKey.Scale);
					DirectX::XMVECTOR vEnd = DirectX::XMLoadFloat3(&EndKey.Scale);

					v = DirectX::XMVectorLerp(v, vEnd, fLerpFactor);

					DirectX::XMStoreFloat3(reinterpret_cast<DirectX::XMFLOAT3*>(&pDestKeys[i].transform.scale), v);
				}

				time += fKeyInterval;
			}

			return true;
		}

		bool SampleTimeData(Motion::Keyframe* pDestKeys, size_t nDestKeyCount, float fKeyInterval, float fStartTime)
		{
			float time = 0.f;
			for (size_t i = 0; i < nDestKeyCount; ++i)
			{
				pDestKeys[i].time = fStartTime + time;

				time += fKeyInterval;
			}

			return true;
		}

		void WriteKeyframes(ExportAnimationTrack* pTrack, Motion* pMotion, float fDuration, float fFrameInterval, float fStartTime)
		{
			std::vector<Motion::Keyframe> vecKeyframes;
			ExportAnimationTransformTrack& transformTrack = pTrack->TransformTrack;

			string::StringID strName;
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

			pMotion->AddBoneKeyframes(strName, std::move(vecKeyframes));
		}

		bool WriteMotion(Motion* pMotion)
		{
			if (g_pScene == nullptr || pMotion == nullptr)
				return false;

			if (g_pScene->GetAnimationCount() == 0)
				return false;

			// 이 로직에서는 애니메이션 하나만 추출한다.
			ExportAnimation* pAnimation = g_pScene->GetAnimation(0);
			if (pAnimation == nullptr)
				return false;

			pMotion->SetInfo(pAnimation->fStartTime, pAnimation->fEndTime, pAnimation->fSourceSamplingInterval);

			size_t nTrackCount = pAnimation->GetTrackCount();
			for (size_t i = 0; i < nTrackCount; ++i)
			{
				ExportAnimationTrack* pTrack = pAnimation->GetTrack(i);
				if (pTrack == nullptr)
					continue;

				WriteKeyframes(pTrack, pMotion, pAnimation->GetDuration(), pAnimation->fSourceSamplingInterval, pAnimation->fStartTime);
			}

			return true;
		}
	}
}