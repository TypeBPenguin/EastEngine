#include "stdafx.h"
#include "CascadedShadows.h"

#include "GraphicsInterface.h"

#include "Camera.h"
#include "Light.h"

namespace est
{
	namespace graphics
	{
		CascadedShadows::CascadedShadows()
		{
		}

		CascadedShadows::~CascadedShadows()
		{
		}

		void CascadedShadows::Update(const IDirectionalLight* pDirectionLight)
		{
			if (pDirectionLight == nullptr)
				return;

			Camera& camera = GetCamera();
			
			const float farClip = camera.GetProjection().farClip;

			const math::Matrix matInvCameraView = camera.GetViewMatrix().Invert();

			collision::Frustum frustum;
			collision::Frustum::CreateFromMatrix(frustum, camera.GetProjectionMatrix());

			math::float3 corners[collision::CornerCount];
			frustum.GetCorners(corners);

			const float zOffset = (camera.GetLookat() - camera.GetPosition()).Length();

			constexpr float increaseRate = 2.f;
			float totalRate = 0.f;
			for (uint32_t i = 0; i < m_config.numCascades; ++i)
			{
				totalRate += static_cast<float>(std::pow(increaseRate, i));
			}

			assert(math::IsZero(totalRate) == false);

			const float standardDistance = m_config.cascadeDistance / totalRate;

			float fDistance = 0.f;
			for (uint32_t i = 0; i < m_config.numCascades; ++i)
			{
				m_viewportCascade[i].height = static_cast<float>(m_config.resolution);
				m_viewportCascade[i].width = static_cast<float>(m_config.resolution);
				m_viewportCascade[i].maxDepth = 1.f;
				m_viewportCascade[i].minDepth = 0.f;
				m_viewportCascade[i].x = static_cast<float>(m_config.resolution * i);
				m_viewportCascade[i].y = 0.f;

				const float prevDistance = fDistance;
				fDistance += standardDistance * static_cast<float>(std::pow(increaseRate, i));

				m_splitDepths[i].x = prevDistance;
				m_splitDepths[i].y = fDistance;

				math::float3 splitFrustumCorners[collision::CornerCount];

				for (int j = 0; j < 4; ++j)
				{
					splitFrustumCorners[j] = corners[j] * (prevDistance / farClip);
					splitFrustumCorners[j].z -= std::max(splitFrustumCorners[j].z * 0.1f, zOffset);
				}

				for (int j = 4; j < collision::CornerCount; ++j)
				{
					splitFrustumCorners[j] = corners[j] * (fDistance / farClip);
					splitFrustumCorners[j].z += std::max(splitFrustumCorners[j].z * 0.1f, zOffset);
				}

				math::float3 frustumCornersWS[collision::CornerCount];
				math::float3::Transform(splitFrustumCorners, collision::CornerCount, matInvCameraView, frustumCornersWS);

				math::float3 frustumCentroid;
				for (int j = 0; j < collision::CornerCount; ++j)
				{
					frustumCentroid += frustumCornersWS[j];
				}
				frustumCentroid /= collision::CornerCount;

				const float distFromCenteroid = 50.f + std::max((fDistance - prevDistance), math::float3::Distance(splitFrustumCorners[collision::eNearLeftBottom], splitFrustumCorners[collision::eFarRightTop]));
				const math::float3 viewPos(frustumCentroid - (pDirectionLight->GetDirection() * distFromCenteroid));
				m_matViews[i] = math::Matrix::CreateLookAt(viewPos, frustumCentroid, math::float3::Up);

				math::float3 frustumCornersLS[collision::CornerCount];
				math::float3::Transform(frustumCornersWS, collision::CornerCount, m_matViews[i], frustumCornersLS);

				math::float3 min(frustumCornersLS[0]);
				math::float3 max(frustumCornersLS[0]);
				for (int j = 1; j < collision::CornerCount; ++j)
				{
					min = math::float3::Min(min, frustumCornersLS[j]);
					max = math::float3::Max(max, frustumCornersLS[j]);
				}

				m_matProjections[i] = math::Matrix::CreateOrthographicOffCenter(min.x, max.x, min.y, max.y, min.z, max.z);
				//m_matProjections[i] = math::Matrix::CreatePerspectiveOffCenter(min.x, max.x, min.y, max.y, min.z, max.z);

				collision::Frustum::CreateFromMatrix(m_frustums[i], m_matProjections[i]);
				m_frustums[i].Transform(m_matViews[i].Invert());

				m_data.cascadeViewProjectionMatrix[i] = m_matViews[i] * m_matProjections[i];
				m_data.cascadeViewProjectionMatrix[i] = m_data.cascadeViewProjectionMatrix[i].Transpose();

				math::Matrix projectionMatrix = m_matProjections[i];
				projectionMatrix._33 /= camera.GetProjection().farClip;
				projectionMatrix._43 /= camera.GetProjection().farClip;

				m_data.cascadeViewLinearProjectionMatrix[i] = m_matViews[i] * projectionMatrix;
				m_data.cascadeViewLinearProjectionMatrix[i] = m_data.cascadeViewLinearProjectionMatrix[i].Transpose();

				m_data.viewPos[i] = math::float4(viewPos.x, viewPos.y, viewPos.z, 0.f);
				m_data.splitDepths[i] = m_splitDepths[i];
			}

			m_data.pcfBlurSize = math::int2(m_config.pcfBlurSize / -2, m_config.pcfBlurSize / 2 + 1);
			m_data.texelOffset = GetTexelOffset();
			m_data.numCascades = m_config.numCascades;
			m_data.depthBias = m_config.depthBias;
		}
	}
}