#include "stdafx.h"
#include "Downscale.h"

#include "CommonLib/FileUtil.h"

namespace StrID
{
	RegisterStringID(EffectDownscale);
	RegisterStringID(Downscale4);
	RegisterStringID(Downscale4Luminance);
	RegisterStringID(DownscaleHW);

	RegisterStringID(g_f2SourceDimensions);
	RegisterStringID(g_texColor);
	RegisterStringID(g_samplerPoint);
	RegisterStringID(g_samplerLinear);
}

namespace EastEngine
{
	namespace Graphics
	{
		Downscale::Downscale()
			: m_isInit(false)
			, m_pEffect(nullptr)
			, m_pSamplerPoint(nullptr)
		{
		}

		Downscale::~Downscale()
		{
			Release();
		}

		bool Downscale::Init()
		{
			if (m_isInit == true)
				return true;

			m_isInit = true;

			std::string strPath(File::GetPath(File::EmPath::eFx));

#if defined(DEBUG) || defined(_DEBUG)
			strPath.append("PostProcessing\\Downscale\\Downscale_D.cso");
#else
			strPath.append("PostProcessing\\Downscale\\Downscale.cso");
#endif

			m_pEffect = IEffect::Create(StrID::EffectDownscale, strPath.c_str());
			if (m_pEffect == nullptr)
				return false;

			m_pEffect->CreateTechnique(StrID::Downscale4, EmVertexFormat::eUnknown);
			m_pEffect->CreateTechnique(StrID::Downscale4Luminance, EmVertexFormat::eUnknown);
			m_pEffect->CreateTechnique(StrID::DownscaleHW, EmVertexFormat::eUnknown);

			SamplerStateDesc desc;
			desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
			desc.MaxAnisotropy = 1;
			desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
			desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
			m_pSamplerPoint = ISamplerState::Create(desc);

			desc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
			m_pSamplerLinear = ISamplerState::Create(desc);

			return true;
		}

		void Downscale::Release()
		{
			if (m_isInit == false)
				return;

			IEffect::Destroy(&m_pEffect);

			m_pSamplerPoint = nullptr;
			m_pSamplerLinear = nullptr;

			m_isInit = false;
		}
		
		void ApplyDownscale(IDeviceContext* pDeviceContext, IEffect* pEffect, IEffectTech* pTech, IRenderTarget* pResult, IRenderTarget* pSource)
		{
			Math::Viewport viewport;
			viewport.width = static_cast<float>(pResult->GetSize().x);
			viewport.height = static_cast<float>(pResult->GetSize().y);
			pDeviceContext->SetViewport(viewport);
			pDeviceContext->SetRenderTargets(&pResult, 1);

			Math::Vector2 f2Size;
			f2Size.x = static_cast<float>(pSource->GetSize().x);
			f2Size.y = static_cast<float>(pSource->GetSize().y);
			pEffect->SetVector(StrID::g_f2SourceDimensions, f2Size);
			pEffect->SetTexture(StrID::g_texColor, pSource->GetTexture());

			uint32_t nPassCount = pTech->GetPassCount();
			for (uint32_t p = 0; p < nPassCount; ++p)
			{
				pTech->PassApply(p, pDeviceContext);

				pDeviceContext->Draw(4, 0);
			}
		}

		bool Downscale::Apply4SW(IRenderTarget* pResult, IRenderTarget* pSource, bool isLuminance)
		{
			if (pResult == nullptr || pResult->GetTexture() == nullptr)
				return false;

			if (pSource == nullptr || pSource->GetTexture() == nullptr)
				return false;

			D3D_PROFILING(Downscale);

			IDeviceContext* pDeviceContext = GetDeviceContext();
			pDeviceContext->ClearState();
			pDeviceContext->SetDefaultViewport();
			pDeviceContext->SetRasterizerState(EmRasterizerState::eSolidCCW);
			pDeviceContext->SetDepthStencilState(EmDepthStencilState::eRead_Write_Off);
			pDeviceContext->SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

			IEffectTech* pTech = m_pEffect->GetTechnique(isLuminance == true ? StrID::Downscale4Luminance : StrID::Downscale4);;
			if (pTech == nullptr)
			{
				assert(false);
				return false;
			}

			m_pEffect->SetSamplerState(StrID::g_samplerPoint, m_pSamplerPoint, 0);
			m_pEffect->SetSamplerState(StrID::g_samplerLinear, m_pSamplerLinear, 0);

			ApplyDownscale(pDeviceContext, m_pEffect, pTech, pResult, pSource);
			ClearEffect(pDeviceContext, pTech);

			return true;
		}

		bool Downscale::Apply16SW(IRenderTarget* pResult, IRenderTarget* pSource)
		{
			if (pResult == nullptr || pResult->GetTexture() == nullptr)
				return false;

			if (pSource == nullptr || pSource->GetTexture() == nullptr)
				return false;

			D3D_PROFILING(Downscale);

			IDevice* pd3d = GetDevice();
			IDeviceContext* pDeviceContext = GetDeviceContext();
			pDeviceContext->ClearState();
			pDeviceContext->SetDefaultViewport();
			pDeviceContext->SetRasterizerState(EmRasterizerState::eSolidCCW);
			pDeviceContext->SetDepthStencilState(EmDepthStencilState::eRead_Write_Off);
			pDeviceContext->SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

			IEffectTech* pTech = m_pEffect->GetTechnique(StrID::Downscale4);
			if (pTech == nullptr)
			{
				assert(false);
				return false;
			}

			RenderTargetDesc2D desc = pSource->GetDesc2D();
			desc.Width /= 4;
			desc.Height /= 4;
			desc.Build();
			IRenderTarget* pDownscale = pd3d->GetRenderTarget(desc, false);

			m_pEffect->SetSamplerState(StrID::g_samplerPoint, m_pSamplerPoint, 0);
			m_pEffect->SetSamplerState(StrID::g_samplerLinear, m_pSamplerLinear, 0);

			ApplyDownscale(pDeviceContext, m_pEffect, pTech, pDownscale, pSource);
			ClearEffect(pDeviceContext, pTech);

			m_pEffect->SetSamplerState(StrID::g_samplerPoint, m_pSamplerPoint, 0);
			m_pEffect->SetSamplerState(StrID::g_samplerLinear, m_pSamplerLinear, 0);

			ApplyDownscale(pDeviceContext, m_pEffect, pTech, pResult, pDownscale);
			ClearEffect(pDeviceContext, pTech);

			pd3d->ReleaseRenderTargets(&pDownscale);

			return true;
		}

		bool Downscale::ApplyHW(IRenderTarget* pResult, IRenderTarget* pSource)
		{
			if (pResult == nullptr || pResult->GetTexture() == nullptr)
				return false;

			if (pSource == nullptr || pSource->GetTexture() == nullptr)
				return false;

			D3D_PROFILING(Downscale);

			IDeviceContext* pDeviceContext = GetDeviceContext();
			pDeviceContext->ClearState();
			pDeviceContext->SetDefaultViewport();
			pDeviceContext->SetRasterizerState(EmRasterizerState::eSolidCCW);
			pDeviceContext->SetDepthStencilState(EmDepthStencilState::eRead_Write_Off);
			pDeviceContext->SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

			IEffectTech* pTech = m_pEffect->GetTechnique(StrID::DownscaleHW);
			if (pTech == nullptr)
			{
				assert(false);
				return false;
			}

			ApplyDownscale(pDeviceContext, m_pEffect, pTech, pResult, pSource);
			ClearEffect(pDeviceContext, pTech);

			return true;
		}

		bool Downscale::Apply16HW(IRenderTarget* pResult, IRenderTarget* pSource)
		{
			if (pResult == nullptr || pResult->GetTexture() == nullptr)
				return false;

			if (pSource == nullptr || pSource->GetTexture() == nullptr)
				return false;

			D3D_PROFILING(Downscale);

			IDevice* pd3d = GetDevice();
			IDeviceContext* pDeviceContext = GetDeviceContext();
			pDeviceContext->ClearState();
			pDeviceContext->SetDefaultViewport();
			pDeviceContext->SetRasterizerState(EmRasterizerState::eSolidCCW);
			pDeviceContext->SetDepthStencilState(EmDepthStencilState::eRead_Write_Off);
			pDeviceContext->SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

			IEffectTech* pTech = m_pEffect->GetTechnique(StrID::DownscaleHW);
			if (pTech == nullptr)
			{
				assert(false);
				return false;
			}

			// 2
			RenderTargetDesc2D desc = pSource->GetDesc2D();
			desc.Width /= 2;
			desc.Height /= 2;
			desc.Build();
			IRenderTarget* pDownscale1 = pd3d->GetRenderTarget(desc, false);

			m_pEffect->SetSamplerState(StrID::g_samplerPoint, m_pSamplerPoint, 0);
			m_pEffect->SetSamplerState(StrID::g_samplerLinear, m_pSamplerLinear, 0);

			ApplyDownscale(pDeviceContext, m_pEffect, pTech, pDownscale1, pSource);
			ClearEffect(pDeviceContext, pTech);

			// 4
			desc.Width /= 2;
			desc.Height /= 2;
			desc.Build();
			IRenderTarget* pDownscale2 = pd3d->GetRenderTarget(desc, false);

			m_pEffect->SetSamplerState(StrID::g_samplerPoint, m_pSamplerPoint, 0);
			m_pEffect->SetSamplerState(StrID::g_samplerLinear, m_pSamplerLinear, 0);

			ApplyDownscale(pDeviceContext, m_pEffect, pTech, pDownscale2, pDownscale1);
			ClearEffect(pDeviceContext, pTech);
			pd3d->ReleaseRenderTargets(&pDownscale1);

			// 8
			desc.Width /= 2;
			desc.Height /= 2;
			desc.Build();
			IRenderTarget* pDownscale3 = pd3d->GetRenderTarget(desc, false);

			m_pEffect->SetSamplerState(StrID::g_samplerPoint, m_pSamplerPoint, 0);
			m_pEffect->SetSamplerState(StrID::g_samplerLinear, m_pSamplerLinear, 0);

			ApplyDownscale(pDeviceContext, m_pEffect, pTech, pDownscale3, pDownscale2);
			ClearEffect(pDeviceContext, pTech);
			pd3d->ReleaseRenderTargets(&pDownscale2);

			// 16
			m_pEffect->SetSamplerState(StrID::g_samplerPoint, m_pSamplerPoint, 0);
			m_pEffect->SetSamplerState(StrID::g_samplerLinear, m_pSamplerLinear, 0);

			ApplyDownscale(pDeviceContext, m_pEffect, pTech, pResult, pDownscale3);
			ClearEffect(pDeviceContext, pTech);
			pd3d->ReleaseRenderTargets(&pDownscale3);

			return true;
		}

		void Downscale::ClearEffect(IDeviceContext* pd3dDeviceContext, IEffectTech* pTech)
		{
			m_pEffect->SetTexture(StrID::g_texColor, nullptr);
			m_pEffect->UndoSamplerState(StrID::g_samplerPoint, 0);
			m_pEffect->UndoSamplerState(StrID::g_samplerLinear, 0);

			m_pEffect->ClearState(pd3dDeviceContext, pTech);
		}
	}
}