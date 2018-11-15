#include "stdafx.h"
#include "Shadow.h"

#include "D3DInterface.h"

namespace eastengine
{
	namespace graphics
	{
		ShadowMap::ShadowMap()
			: m_nPCFBlurSize(5)
			, m_fDepthBias(0.001f)
			, m_fCalcDepthBias(0.001f)
			, m_pShadowConfig(nullptr)
			, m_pLight(nullptr)
			, m_pShadowMap(nullptr)
		{
		}

		ShadowMap::~ShadowMap()
		{
			Release();
		}

		bool ShadowMap::Init(ISpotLight* pLight, const ShadowConfig* pShadowConfig)
		{
			// Initialize m_iBufferSize to 0 to trigger a reallocate on the first frame.   
			m_copyConfig = *pShadowConfig;
			m_copyConfig.nBufferSize = 0;

			m_pShadowConfig = pShadowConfig;

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

		void ShadowMap::Release()
		{
			SafeDelete(m_pShadowMap);

			m_pRasterizerShadow = nullptr;
			m_pSamplerShadowPoint = nullptr;
			m_pSamplerShadowPCF = nullptr;

			m_pShadowConfig = nullptr;
		}

		void ShadowMap::Update()
		{
			RefreshShadowResource();

			const float fAttenuation = 0.01f;
			const float fInvAttenuation = 1.f / fAttenuation;
			float fDepth = std::sqrtf(fInvAttenuation * m_pLight->GetIntensity() * math::PI);

			float fFov = math::ToRadians(m_pLight->GetAngle());

			math::float3 f3Pos = m_pLight->GetPosition() - (m_pLight->GetDirection() * fDepth * 0.1f);
			const math::float3& f3Target = m_pLight->GetPosition();
			m_matView = math::Matrix::CreateLookAt(f3Pos, f3Target, math::float3::Up);

			m_matProjection = math::Matrix::CreatePerspectiveFieldOfView(fFov, 1.f, 0.01f, fDepth);

			Collision::Frustum::CreateFromMatrix(m_frustum, m_matProjection);
			m_frustum.Transform(m_matView.Invert());

			// 아래 검증 필요
			m_fCalcDepthBias = m_fDepthBias * /*(1.f / fDepth * 0.01f) **/ (1.f / (fDepth * fFov));
			//m_fCalcDepthBias = m_fDepthBias * (1.f / fDepth * 0.01f);
		}

		const std::shared_ptr<ITexture>& ShadowMap::GetShadowMap() const
		{
			return m_pShadowMap->GetTexture();
		}

		void ShadowMap::RefreshShadowResource()
		{
			// If any of these 3 paramaters was changed, we must reallocate the D3D resources.
			if (m_copyConfig.emBufferFormat != m_pShadowConfig->emBufferFormat ||
				m_copyConfig.nBufferSize != m_pShadowConfig->nBufferSize)
			{
				m_copyConfig = *m_pShadowConfig;

				m_viewport.height = static_cast<float>(m_copyConfig.nBufferSize);
				m_viewport.width = static_cast<float>(m_copyConfig.nBufferSize);
				m_viewport.maxDepth = 1.f;
				m_viewport.minDepth = 0.f;
				m_viewport.x = 0;
				m_viewport.y = 0;

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

				SafeDelete(m_pShadowMap);
				DepthStencilDesc depthStencilDesc(format,
					m_copyConfig.nBufferSize,
					m_copyConfig.nBufferSize);
				depthStencilDesc.Build();

				m_pShadowMap = IDepthStencil::Create(depthStencilDesc);
				if (m_pShadowMap == nullptr)
				{
					assert(false);
				}
			}
		}
	}
}