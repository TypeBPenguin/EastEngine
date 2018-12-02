#include "stdafx.h"
#include "ModelNodeStatic.h"

#include "CommonLib/FileStream.h"
#include "CommonLib/FileUtil.h"

#include "GeometryModel.h"

namespace eastengine
{
	namespace graphics
	{
		ModelNodeStatic::ModelNodeStatic(LOD emLOD)
			: ModelNode(emLOD)
		{
		}

		ModelNodeStatic::ModelNodeStatic(const char* filePath, const BYTE** ppBuffer)
			: ModelNode(eStatic, filePath, ppBuffer)
		{
		}

		ModelNodeStatic::~ModelNodeStatic()
		{
		}

		void ModelNodeStatic::Update(float elapsedTime, const math::Matrix& matParent, ISkeletonInstance* pSkeletonInstance, IMaterialInstance* pMaterialInstance, bool isModelVisible) const
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

						RenderJobStatic renderJob(&modelSubset, m_pVertexBuffer[level], m_pIndexBuffer[level], pMaterial, matTransformation, modelSubset.nStartIndex, modelSubset.nIndexCount, m_fDistanceFromCamera, occlusionCullingData);
						PushRenderJob(renderJob);

						if (GetOptions().OnCollisionVisible == true)
						{
							IVertexBuffer* pVertexBuffer = nullptr;
							IIndexBuffer* pIndexBuffer = nullptr;
							geometry::GetDebugModel(geometry::DebugModelType::eBox, &pVertexBuffer, &pIndexBuffer);

							const math::Matrix matWorld = math::Matrix::Compose(occlusionCullingData.aabb.Extents * 2.f, math::Quaternion::Identity, occlusionCullingData.aabb.Center);
							RenderJobVertex debugJob(pVertexBuffer, pIndexBuffer, matWorld, math::Color::Red);
							PushRenderJob(debugJob);
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