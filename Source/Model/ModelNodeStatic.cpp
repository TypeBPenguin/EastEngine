#include "stdafx.h"
#include "ModelNodeStatic.h"

#include "GeometryModel.h"

#include "CommonLib/Config.h"
#include "Renderer/RendererManager.h"

namespace EastEngine
{
	namespace Graphics
	{
		ModelNodeStatic::ModelNodeStatic(uint32_t nLodMax)
			: ModelNode(EmModelNode::eStatic)
		{
			m_nLodMax = nLodMax;
		}

		ModelNodeStatic::~ModelNodeStatic()
		{
		}

		void ModelNodeStatic::Update(float fElapsedTime, const Math::Matrix& matParent, ISkeletonInstance* pSkeletonInstance, IMaterialInstance* pMaterialInstance, bool isModelVisible)
		{
			Math::Matrix matTransformation = matParent;
			if (m_strAttachedBoneName.empty() == false && pSkeletonInstance != nullptr)
			{
				ISkeletonInstance::IBone* pBone = pSkeletonInstance->GetBone(m_strAttachedBoneName);
				if (pBone != nullptr)
				{
					const Math::Matrix& matMotionTransform = pBone->GetMotionTransform();
					matTransformation = matMotionTransform * matParent;
				}
			}

			m_matWorld = matTransformation;

			UpdateBoundingBox(m_matWorld);

			if (m_isVisible == true && isModelVisible == true)
			{
				uint32_t nLevel = Math::Min(m_nLod, m_nLodMax);
				if (m_pVertexBuffer[nLevel] == nullptr || m_pIndexBuffer[nLevel] == nullptr)
				{
					LOG_WARNING("Model Data is nullptr, LOD : %d, %d", nLevel, m_nLodMax);
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
								if (m_vecMaterial[modelSubset.nMaterialID]->IsLoadComplete() == false)
									continue;

								pMaterial = m_vecMaterial[modelSubset.nMaterialID];
							}
						}

						RenderSubsetStatic subset(&modelSubset, m_pVertexBuffer[nLevel], m_pIndexBuffer[nLevel], pMaterial, m_matWorld, modelSubset.nStartIndex, modelSubset.nIndexCount, m_fDistanceFromCamera, m_boundingSphere);
						RendererManager::GetInstance()->AddRender(subset);
					}
				}
			}

			const size_t nSize = m_vecChildModelNode.size();
			for (size_t i = 0; i < nSize; ++i)
			{
				m_vecChildModelNode[i]->Update(fElapsedTime, m_matWorld, pSkeletonInstance, pMaterialInstance, isModelVisible);
			}
		}
	}
}