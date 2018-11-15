#include "stdafx.h"
#include "ModelNodeStatic.h"

#include "GeometryModel.h"

namespace eastengine
{
	namespace graphics
	{
		ModelNodeStatic::ModelNodeStatic(LOD emLOD)
			: ModelNode(emLOD)
		{
		}

		ModelNodeStatic::~ModelNodeStatic()
		{
		}

		void ModelNodeStatic::Update(float fElapsedTime, const math::Matrix& matParent, ISkeletonInstance* pSkeletonInstance, IMaterialInstance* pMaterialInstance, bool isModelVisible) const
		{
			math::Matrix matTransformation = matParent;
			if (m_strAttachedBoneName.empty() == false && pSkeletonInstance != nullptr)
			{
				ISkeletonInstance::IBone* pBone = pSkeletonInstance->GetBone(m_strAttachedBoneName);
				if (pBone != nullptr)
				{
					const math::Matrix& matMotionTransform = pBone->GetSkinningMatrix();
					matTransformation = matMotionTransform * matParent;
				}
			}

			if (m_isVisible == true && isModelVisible == true)
			{
				const LOD level = std::min(m_emLod, m_emMaxLod);
				if (m_pVertexBuffer[level] == nullptr || m_pIndexBuffer[level] == nullptr)
				{
					LOG_WARNING("Model Data is nullptr, LOD : %d, %d", level, m_emMaxLod);
				}
				else
				{
					OcclusionCullingData occlusionCullingData;
					occlusionCullingData.pVertices = m_rawVertices.data();
					occlusionCullingData.pIndices = m_rawIndices.data();
					occlusionCullingData.indexCount = m_rawIndices.size();
					m_originAABB.Transform(occlusionCullingData.aabb, matTransformation);

					for (auto& modelSubset : m_vecModelSubsets[level])
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

						if (pMaterial != nullptr && pMaterial->IsVisible() == false)
							continue;

						RenderJobStatic subset(&modelSubset, m_pVertexBuffer[level], m_pIndexBuffer[level], pMaterial, matTransformation, modelSubset.nStartIndex, modelSubset.nIndexCount, m_fDistanceFromCamera, occlusionCullingData);
						PushRenderJob(subset);
					}
				}
			}

			const size_t nSize = m_vecChildModelNode.size();
			for (size_t i = 0; i < nSize; ++i)
			{
				m_vecChildModelNode[i]->Update(fElapsedTime, matTransformation, pSkeletonInstance, pMaterialInstance, isModelVisible);
			}
		}
	}
}