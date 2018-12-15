#include "stdafx.h"
#include "CascadedShadows.h"

#include "D3DInterface.h"
#include "Camera.h"

#include "DirectionalLight.h"

namespace eastengine
{
	namespace graphics
	{
		CascadedShadows::CascadedShadows()
			: m_pConfig(nullptr)
			, m_pLight(nullptr)
			, m_pCascadedShadowMap(nullptr)
			, m_pRasterizerShadow(nullptr)
			, m_pSamplerShadowPCF(nullptr)
			, m_pSamplerShadowPoint(nullptr)
			, m_nPCFBlurSize(5)
			, m_fDepthBias(0.002f)
		{
		}

		CascadedShadows::~CascadedShadows()
		{
			Release();
		}

		bool CascadedShadows::Init(IDirectionalLight* pLight, const CascadedShadowsConfig* pConfig)
		{
			// Initialize m_iBufferSize to 0 to trigger a reallocate on the first frame.   
			m_copyConfig = *pConfig;
			m_copyConfig.nBufferSize = 0;

			m_pConfig = pConfig;

			m_pLight = pLight;

			RasterizerStateDesc rsDesc;
			rsDesc.FillMode = D3D11_FILL_SOLID;
			//rsDesc.CullMode = D3D11_CULL_NONE;
			rsDesc.CullMode = D3D11_CULL_BACK;
			rsDesc.FrontCounterClockwise = false;
			rsDesc.DepthBias = 0;
			rsDesc.DepthBiasClamp = 0.f;
			rsDesc.DepthClipEnable = true;
			rsDesc.ScissorEnable = false;
			rsDesc.MultisampleEnable = true;
			rsDesc.AntialiasedLineEnable = false;

			// Setting the slope scale depth biase greatly decreases surface acne and incorrect self shadowing.
			rsDesc.SlopeScaledDepthBias = 1.f;
			m_pRasterizerShadow = IRasterizerState::Create(rsDesc);
			if (m_pRasterizerShadow == nullptr)
			{
				assert(false);
				return false;
			}

			SamplerStateDesc samDesc;
			samDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
			samDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
			samDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
			samDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
			samDesc.MipLODBias = 0.f;
			samDesc.MaxAnisotropy = 0;
			samDesc.ComparisonFunc = D3D11_COMPARISON_LESS;
			samDesc.BorderColor[0] = samDesc.BorderColor[1] = samDesc.BorderColor[2] = samDesc.BorderColor[3] = 0.f;
			samDesc.MinLOD = 0.f;
			samDesc.MaxLOD = 0.f;
			m_pSamplerShadowPCF = ISamplerState::Create(samDesc);
			if (m_pSamplerShadowPCF == nullptr)
			{
				assert(false);
				return false;
			}

			samDesc.MaxAnisotropy = 15;
			samDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
			samDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
			samDesc.Filter = D3D11_FILTER_ANISOTROPIC;
			samDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
			m_pSamplerShadowPoint = ISamplerState::Create(samDesc);
			if (m_pSamplerShadowPoint == nullptr)
			{
				assert(false);
				return false;
			}

			return true;
		}

		void CascadedShadows::Release()
		{
			SafeDelete(m_pCascadedShadowMap);

			m_pRasterizerShadow = nullptr;
			m_pSamplerShadowPoint = nullptr;
			m_pSamplerShadowPCF = nullptr;

			m_pConfig = nullptr;
		}

		void CascadedShadows::Update()
		{
			Camera* pCamera = Camera::GetInstance();
			if (pCamera == nullptr)
				return;

			RefreshShadowResource();

			int nThreadID = GetThreadID(ThreadType::eUpdate);
			const float fNearClip = pCamera->GetNearClip(nThreadID);
			const float fFarClip = pCamera->GetFarClip(nThreadID);

			const math::Matrix matInvCameraView = pCamera->GetViewMatrix(nThreadID).Invert();

			math::Matrix matCameraWorld = math::Matrix::CreateTranslation(pCamera->GetPosition());

			collision::Frustum frustum;
			collision::Frustum::CreateFromMatrix(frustum, pCamera->GetProjMatrix(nThreadID));

			math::float3 f3Corners[collision::CornerCount];
			frustum.GetCorners(f3Corners);

			const float fZOffset = (pCamera->GetLookat() - pCamera->GetPosition()).Length();

			const math::float3 f3Target = m_pLight->GetDirection();

			const float fIncreaseRate = 2.f;
			float fTotalRate = 0.f;
			for (uint32_t i = 0; i < m_copyConfig.nLevel; ++i)
			{
				fTotalRate += static_cast<float>(std::pow(fIncreaseRate, i));
			}

			assert(math::IsZero(fTotalRate) == false);

			float fStandardDistance = m_copyConfig.fCascadeDistance / fTotalRate;

			float fDistance = 0.f;
			for (uint32_t i = 0; i < m_copyConfig.nLevel; ++i)
			{
				float fPrevDistance = fDistance;
				fDistance += fStandardDistance * static_cast<float>(std::pow(fIncreaseRate, i));

				m_f2SplitDepths[i].x = fPrevDistance;
				m_f2SplitDepths[i].y = fDistance;

				math::float3 f3SplitFrustumCorners[collision::CornerCount];

				for (int j = 0; j < 4; ++j)
				{
					f3SplitFrustumCorners[j] = f3Corners[j] * (fPrevDistance / fFarClip);
					f3SplitFrustumCorners[j].z -= std::max(f3SplitFrustumCorners[j].z * 0.1f, fZOffset);
				}

				for (int j = 4; j < collision::CornerCount; ++j)
				{
					f3SplitFrustumCorners[j] = f3Corners[j] * (fDistance / fFarClip);
					f3SplitFrustumCorners[j].z += std::max(f3SplitFrustumCorners[j].z * 0.1f, fZOffset);
				}

				math::float3 f3FrustumCornersWS[collision::CornerCount];
				math::float3::Transform(f3SplitFrustumCorners, collision::CornerCount, matInvCameraView, f3FrustumCornersWS);

				math::float3 f3FrustumCentroid;
				for (int j = 0; j < collision::CornerCount; ++j)
				{
					f3FrustumCentroid += f3FrustumCornersWS[j];
				}
				f3FrustumCentroid /= collision::CornerCount;

				float fDistFromCenteroid = 50.f + std::max((fDistance - fPrevDistance), math::float3::Distance(f3SplitFrustumCorners[collision::eNearLeftBottom], f3SplitFrustumCorners[collision::eFarRightTop]));
				m_matViews[i] = math::Matrix::CreateLookAt(f3FrustumCentroid - (m_pLight->GetDirection() * fDistFromCenteroid), f3FrustumCentroid, math::float3::Up);

				math::float3 f3FrustumCornersLS[collision::CornerCount];
				math::float3::Transform(f3FrustumCornersWS, collision::CornerCount, m_matViews[i], f3FrustumCornersLS);

				math::float3 f3Min(f3FrustumCornersLS[0]);
				math::float3 f3Max(f3FrustumCornersLS[0]);
				for (int j = 1; j < collision::CornerCount; ++j)
				{
					f3Min = math::float3::Min(f3Min, f3FrustumCornersLS[j]);
					f3Max = math::float3::Max(f3Max, f3FrustumCornersLS[j]);
				}

				m_matProjections[i] = math::Matrix::CreateOrthographicOffCenter(f3Min.x, f3Max.x, f3Min.y, f3Max.y, f3Min.z, f3Max.z);

				collision::Frustum::CreateFromMatrix(m_frustums[i], m_matProjections[i]);
				m_frustums[i].Transform(m_matViews[i].Invert());
			}
		}

		const std::shared_ptr<ITexture>& CascadedShadows::GetShadowMap() const
		{
			return m_pCascadedShadowMap->GetTexture();
		}

		void CascadedShadows::RefreshShadowResource()
		{
			// If any of these 3 paramaters was changed, we must reallocate the D3D resources.
			if (m_copyConfig.nLevel != m_pConfig->nLevel ||
				m_copyConfig.emBufferFormat != m_pConfig->emBufferFormat ||
				m_copyConfig.nBufferSize != m_pConfig->nBufferSize)
			{
				m_copyConfig = *m_pConfig;

				for (uint32_t index = 0; index < m_copyConfig.nLevel; ++index)
				{
					m_viewportCascade[index].height = static_cast<float>(m_copyConfig.nBufferSize);
					m_viewportCascade[index].width = static_cast<float>(m_copyConfig.nBufferSize);
					m_viewportCascade[index].maxDepth = 1.f;
					m_viewportCascade[index].minDepth = 0.f;
					m_viewportCascade[index].x = static_cast<float>(m_copyConfig.nBufferSize * index);
					m_viewportCascade[index].y = 0;
				}

				DXGI_FORMAT format = DXGI_FORMAT_R32_TYPELESS;

				switch (m_copyConfig.emBufferFormat)
				{
				case EmBufferFormat::eR32:
					format = DXGI_FORMAT_R32_TYPELESS;
					break;
				case EmBufferFormat::eR24G8:
					format = DXGI_FORMAT_R24G8_TYPELESS;
					break;
				case EmBufferFormat::eR16:
					format = DXGI_FORMAT_R16_TYPELESS;
					break;
				case EmBufferFormat::eR8:
					format = DXGI_FORMAT_R8_TYPELESS;
					break;
				}

				SafeDelete(m_pCascadedShadowMap);
				DepthStencilDesc depthStencilDesc(format,
					m_copyConfig.nBufferSize * m_copyConfig.nLevel,
					m_copyConfig.nBufferSize);
				depthStencilDesc.Build();

				m_pCascadedShadowMap = IDepthStencil::Create(depthStencilDesc);
				if (m_pCascadedShadowMap == nullptr)
				{
					assert(false);
				}
			}
		}
	}
}