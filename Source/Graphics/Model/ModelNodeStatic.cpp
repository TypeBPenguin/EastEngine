#include "stdafx.h"
#include "ModelNodeStatic.h"

#include "CommonLib/FileStream.h"
#include "CommonLib/FileUtil.h"

#include "GeometryModel.h"

namespace est
{
	namespace graphics
	{
		ModelNodeStatic::ModelNodeStatic(LOD emLOD)
			: ModelNode(emLOD)
		{
		}

		ModelNodeStatic::ModelNodeStatic(const wchar_t* filePath, BinaryReader& binaryReader)
			: ModelNode(eStatic, filePath, binaryReader)
		{
		}

		ModelNodeStatic::~ModelNodeStatic()
		{
		}

		void ModelNodeStatic::Update(float elapsedTime, const math::Matrix& matParent, ISkeletonInstance* pSkeletonInstance, IMaterialInstance* pMaterialInstance, ITransformInstance* pTransformInstance, bool isModelVisible) const
		{
			math::Matrix matTransformation = matParent;
			if (m_attachedBoneName != sid::None && pSkeletonInstance != nullptr)
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
				const math::Matrix& matPrevTransform = pTransformInstance->GetPrevTransform(GetName());

				const LOD level = std::min(m_emLod, m_emMaxLod);
				if (m_pVertexBuffer[level] == nullptr || m_pIndexBuffer[level] == nullptr)
				{
					LOG_WARNING(L"Model Data is nullptr, LOD : %d, %d", level, m_emMaxLod);
				}
				else
				{
					OcclusionCullingData occlusionCullingData;
					occlusionCullingData.pVertices = m_rawVertices.data();
					occlusionCullingData.pIndices = m_rawIndices.data();
					occlusionCullingData.indexCount = m_rawIndices.size();
					m_originAABB.Transform(occlusionCullingData.aabb, matTransformation);

					for (auto& modelSubset : m_modelSubsets[level])
					{
						MaterialPtr pMaterial;

						if (pMaterialInstance != nullptr)
						{
							pMaterial = pMaterialInstance->GetMaterial(m_nodeName, modelSubset.materialID);
						}

						if (pMaterial == nullptr)
						{
							if (modelSubset.materialID < m_materials.size())
							{
								pMaterial = m_materials[modelSubset.materialID];
							}
						}

						if (pMaterial != nullptr && pMaterial->IsVisible() == false)
							continue;

						RenderJobStatic renderJob(&modelSubset, m_pVertexBuffer[level], m_pIndexBuffer[level], pMaterial, matTransformation, matPrevTransform, modelSubset.startIndex, modelSubset.indexCount, m_fDistanceFromCamera, occlusionCullingData);
						PushRenderJob(renderJob);

						if (GetOptions().OnCollisionVisible == true)
						{
							VertexBufferPtr pVertexBuffer;
							IndexBufferPtr pIndexBuffer;
							geometry::GetDebugModel(geometry::DebugModelType::eBox, &pVertexBuffer, &pIndexBuffer);

							const math::Matrix matWorld = math::Matrix::Compose(occlusionCullingData.aabb.Extents * 2.f, math::Quaternion::Identity, occlusionCullingData.aabb.Center);
							RenderJobVertex debugJob(pVertexBuffer, pIndexBuffer, matWorld, math::Color::Red);
							PushRenderJob(debugJob);
						}
					}
				}

				pTransformInstance->SetTransform(GetName(), matTransformation);
			}

			for (auto& pNode : m_childNodes)
			{
				pNode->Update(elapsedTime, matTransformation, pSkeletonInstance, pMaterialInstance, pTransformInstance, isModelVisible);
			}
		}
	}
}