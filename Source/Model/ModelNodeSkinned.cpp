#include "stdafx.h"
#include "ModelNodeSkinned.h"

#include "DirectX/VTFMgr.h"

#include "Renderer/RendererManager.h"

namespace EastEngine
{
	namespace Graphics
	{
		ModelNodeSkinned::ModelNodeSkinned(uint32_t nLodMax)
			: ModelNode(EmModelNode::eSkinned)
		{
			m_nLodMax = nLodMax;
		}

		ModelNodeSkinned::~ModelNodeSkinned()
		{
		}

		void ModelNodeSkinned::Update(float fElapsedTime, const Math::Matrix& matParent, ISkeletonInstance* pSkeletonInstance, IMaterialInstance* pMaterialInstance, bool isModelVisible)
		{
			Math::Matrix matTransformation = matParent;
			if (m_strAttachedBoneName.empty() == false && pSkeletonInstance != nullptr)
			{
				ISkeletonInstance::IBone* pBone = pSkeletonInstance->GetBone(m_strAttachedBoneName);
				if (pBone != nullptr)
				{
					const Math::Matrix& matBoneTransform = pBone->GetTransform();
					matTransformation = matBoneTransform * matParent;
				}
			}

			m_matWorld = matTransformation;

			UpdateBoundingBox();

			//if (m_isVisible == true && isModelVisible == true)
			{
				uint32_t nVTFID = eInvalidVTFID;
				Math::Matrix* pMatrixBuffer = nullptr;
				if (pSkeletonInstance != nullptr && VTFManager::GetInstance()->Allocate(m_vecBoneName.size(), &pMatrixBuffer, nVTFID) == true)
				{
					if (nVTFID != eInvalidVTFID)
					{
						const Math::Matrix** ppSkinnedMatrix = nullptr;
						uint32_t nElementCount = 0;
						pSkeletonInstance->GetSkinnedData(GetName(), &ppSkinnedMatrix, nElementCount);

						if (ppSkinnedMatrix != nullptr && nElementCount > 0)
						{
							for (uint32_t i = 0; i < nElementCount; ++i)
							{
								Math::Matrix& matBone = pMatrixBuffer[i];

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
									matBone = Math::Matrix::Identity;
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
							nVTFID = eInvalidVTFID;
						}
					}
				}

				if (nVTFID != eInvalidVTFID)
				{
					uint32_t nLevel = Math::Min(m_nLod, m_nLodMax);
					if (m_pVertexBuffer[nLevel] == nullptr || m_pIndexBuffer[nLevel] == nullptr)
					{
						PRINT_LOG("Model Data is nullptr, LOD : %d", nLevel);
					}
					else
					{
						for (auto& modelSubset : m_vecModelSubsets[nLevel])
						{
							IMaterial* pMaterial = nullptr;

							if (pMaterialInstance != nullptr)
							{
								pMaterial = pMaterialInstance->GetMaterial(m_strNodeName, modelSubset.nMaterialID);
							}

							if (pMaterial == nullptr)
							{
								if (modelSubset.nMaterialID < m_vecMaterial.size())
								{
									pMaterial = m_vecMaterial[modelSubset.nMaterialID];
								}
							}

							RenderSubsetSkinned subset(&modelSubset, m_pVertexBuffer[nLevel], m_pIndexBuffer[nLevel], pMaterial, m_matWorld, modelSubset.nStartIndex, modelSubset.nIndexCount, nVTFID, m_fDistanceFromCamera);
							RendererManager::GetInstance()->AddRender(subset);
						}
					}
				}
			}

			uint32_t nSize = m_vecChildModelNode.size();
			for (uint32_t i = 0; i < nSize; ++i)
			{
				m_vecChildModelNode[i]->Update(fElapsedTime, m_matWorld, pSkeletonInstance, pMaterialInstance, isModelVisible);
			}
		}
	}
}