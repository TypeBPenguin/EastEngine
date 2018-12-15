#include "stdafx.h"
#include "ModelNodeSkinned.h"

#include "CommonLib/FileStream.h"

#include "GeometryModel.h"

namespace eastengine
{
	namespace graphics
	{
		ModelNodeSkinned::ModelNodeSkinned(LOD emLOD)
			: ModelNode(emLOD)
		{
		}

		ModelNodeSkinned::ModelNodeSkinned(const char* filePath, BinaryReader& binaryReader)
			: ModelNode(eSkinned, filePath, binaryReader)
		{
			const uint32_t boneCount = binaryReader;
			m_vecBoneName.resize(boneCount);
			for (uint32_t i = 0; i < boneCount; ++i)
			{
				m_vecBoneName[i] = binaryReader.ReadString();
			}
		}

		ModelNodeSkinned::~ModelNodeSkinned()
		{
		}

		void ModelNodeSkinned::Update(float elapsedTime, const math::Matrix& matParent, ISkeletonInstance* pSkeletonInstance, IMaterialInstance* pMaterialInstance, bool isModelVisible) const
		{
			math::Matrix matTransformation = matParent;
			if (m_attachedBoneName != StrID::None && pSkeletonInstance != nullptr)
			{
				ISkeletonInstance::IBone* pBone = pSkeletonInstance->GetBone(m_attachedBoneName);
				if (pBone != nullptr)
				{
					const math::Matrix& matMotionTransform = pBone->GetSkinningMatrix();
					matTransformation = matMotionTransform * matParent;
				}
			}

			if (m_isVisible == true && isModelVisible == true)
			{
				IVTFManager* pVTFManager = GetVTFManager();

				uint32_t nVTFID = IVTFManager::eInvalidVTFID;
				math::Matrix* pMatrixBuffer = nullptr;
				if (pSkeletonInstance != nullptr && pVTFManager->Allocate(static_cast<uint32_t>(m_vecBoneName.size()), &pMatrixBuffer, nVTFID) == true)
				{
					if (nVTFID != IVTFManager::eInvalidVTFID)
					{
						const math::Matrix* const* ppSkinnedMatrix = nullptr;
						uint32_t nElementCount = 0;
						pSkeletonInstance->GetSkinnedData(GetName(), &ppSkinnedMatrix, nElementCount);

						if (ppSkinnedMatrix != nullptr && nElementCount > 0)
						{
							for (uint32_t i = 0; i < nElementCount; ++i)
							{
								math::Matrix& matBone = pMatrixBuffer[i];

								if (ppSkinnedMatrix[i] != nullptr)
								{
									matBone = *ppSkinnedMatrix[i];
									matBone._14 = matBone._41;
									matBone._24 = matBone._42;
									matBone._34 = matBone._43;
									matBone._41 = matBone._42 = matBone._43 = matBone._44 = 0.f;
								}
								else
								{
									matBone = math::Matrix::Identity;
									matBone._14 = matBone._41;
									matBone._24 = matBone._42;
									matBone._34 = matBone._43;
									matBone._41 = matBone._42 = matBone._43 = matBone._44 = 0.f;
								}
							}
						}
						else
						{
							assert(false);
							nVTFID = IVTFManager::eInvalidVTFID;
						}
					}
				}

				if (nVTFID != IVTFManager::eInvalidVTFID)
				{
					const LOD emLod = std::min(m_emLod, m_emMaxLod);
					if (m_pVertexBuffer[emLod] == nullptr || m_pIndexBuffer[emLod] == nullptr)
					{
						LOG_WARNING("Model Data is nullptr, LOD : %d", emLod);
					}
					else
					{
						OcclusionCullingData occlusionCullingData;
						occlusionCullingData.pVertices = m_rawVertices.data();
						occlusionCullingData.pIndices = m_rawIndices.data();
						occlusionCullingData.indexCount = m_rawIndices.size();
						m_originAABB.Transform(occlusionCullingData.aabb, matTransformation);

						for (auto& modelSubset : m_vecModelSubsets[emLod])
						{
							IMaterial* pMaterial = nullptr;

							if (pMaterialInstance != nullptr)
							{
								pMaterial = pMaterialInstance->GetMaterial(m_nodeName, modelSubset.nMaterialID);
							}

							if (pMaterial == nullptr)
							{
								if (modelSubset.nMaterialID < m_vecMaterial.size())
								{
									pMaterial = m_vecMaterial[modelSubset.nMaterialID];
								}
							}

							if (pMaterial != nullptr && pMaterial->IsVisible() == false)
								continue;

							RenderJobSkinned renderJob(&modelSubset, m_pVertexBuffer[emLod], m_pIndexBuffer[emLod], pMaterial, matTransformation, modelSubset.nStartIndex, modelSubset.nIndexCount, nVTFID, m_fDistanceFromCamera, occlusionCullingData);
							PushRenderJob(renderJob);

							if (GetOptions().OnCollisionVisible == true)
							{
								IVertexBuffer* pVertexBuffer = nullptr;
								IIndexBuffer* pIndexBuffer = nullptr;
								geometry::GetDebugModel(geometry::DebugModelType::eBox, &pVertexBuffer, &pIndexBuffer);

								const math::Matrix matWorld = math::Matrix::Compose(occlusionCullingData.aabb.Extents * 2.f, math::Quaternion::Identity, occlusionCullingData.aabb.Center);
								RenderJobVertex debugJob(pVertexBuffer, pIndexBuffer, matWorld, math::Color::Blue);
								PushRenderJob(debugJob);
							}
						}
					}
				}
			}

			const size_t nSize = m_vecChildModelNode.size();
			for (size_t i = 0; i < nSize; ++i)
			{
				m_vecChildModelNode[i]->Update(elapsedTime, matTransformation, pSkeletonInstance, pMaterialInstance, isModelVisible);
			}
		}
	}
}