#include "stdafx.h"
#include "HDRFilter.h"

#include "CommonLib/FileUtil.h"

#include "Downscale.h"
#include "GaussianBlur.h"

namespace StrID
{
	RegisterStringID(EffectHDR);

	RegisterStringID(Luminance);
	RegisterStringID(CalcAdaptedLuminance);
	RegisterStringID(ToneMap);
	RegisterStringID(Threshold);

	RegisterStringID(LensFlareFirstPass);
	RegisterStringID(LensFlareSecondPass);
	RegisterStringID(Combine);

	RegisterStringID(g_fMiddleGrey);
	RegisterStringID(g_fMaxLuminance);
	RegisterStringID(g_fElapsedTime);
	RegisterStringID(g_fBloomMultiplier);
	RegisterStringID(g_fThreshold);
	RegisterStringID(g_f2SourceDimensions);
	RegisterStringID(g_texColor);
	RegisterStringID(g_texBloom);
	RegisterStringID(g_texLuminanceCur);
	RegisterStringID(g_texLuminanceLast);
	RegisterStringID(g_texLensFlare1);
	RegisterStringID(g_texLensFlare2);
	RegisterStringID(g_samplerPoint);
	RegisterStringID(g_samplerLinear);
}

namespace EastEngine
{
	namespace Graphics
	{
		HDRFilter::HDRFilter()
			: m_isInit(false)
			, m_isEnableLensFlare(true)
			, m_pEffect(nullptr)
			, m_fBloomThreshold(0.85f)
			, m_fToneMapKey(0.8f)
			, m_fMaxLuminance(512.f)
			, m_fBloomMultiplier(1.f)
			, m_fBlurSigma(2.5f)
			, m_pLuminanceCurrent(nullptr)
			, m_pLuminanceLast(nullptr)
			, m_pAdaptedLuminance(nullptr)
			, m_pSamplerPoint(nullptr)
			, m_pSamplerLinear(nullptr)
		{
		}

		HDRFilter::~HDRFilter()
		{
			Release();
		}

		bool HDRFilter::Init()
		{
			if (m_isInit == true)
				return true;

			m_isInit = true;

			std::string strPath(File::GetPath(File::EmPath::eFx));

#if defined(DEBUG) || defined(_DEBUG)
			strPath.append("PostProcessing\\HDR\\HDR_D.cso");
#else
			strPath.append("PostProcessing\\HDR\\HDR.cso");
#endif

			m_pEffect = IEffect::Create(StrID::EffectHDR, strPath.c_str());
			if (m_pEffect == nullptr)
				return false;

			m_pEffect->CreateTechnique(StrID::Luminance, EmVertexFormat::eUnknown);
			m_pEffect->CreateTechnique(StrID::CalcAdaptedLuminance, EmVertexFormat::eUnknown);
			m_pEffect->CreateTechnique(StrID::ToneMap, EmVertexFormat::eUnknown);
			m_pEffect->CreateTechnique(StrID::Threshold, EmVertexFormat::eUnknown);
			m_pEffect->CreateTechnique(StrID::LensFlareFirstPass, EmVertexFormat::eUnknown);
			m_pEffect->CreateTechnique(StrID::LensFlareSecondPass, EmVertexFormat::eUnknown);
			m_pEffect->CreateTechnique(StrID::Combine, EmVertexFormat::eUnknown);

			SamplerStateDesc desc;
			desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
			desc.MaxAnisotropy = 1;
			desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
			desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
			m_pSamplerPoint = ISamplerState::Create(desc);

			desc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
			m_pSamplerLinear = ISamplerState::Create(desc);

			const Math::UInt2& n2Size = GetDevice()->GetScreenSize();

			int nChainLength = 1;
			int nStartSize = Math::Min(n2Size.x / 16, n2Size.y / 16);
			int nSize = 16;
			for (nSize = 16; nSize < nStartSize; nSize *= 4)
			{
				++nChainLength;
			}

			nSize /= 4;
			RenderTargetDesc2D rtDesc(DXGI_FORMAT_R32_FLOAT, nSize, nSize);
			rtDesc.Build();

			m_vecLuminanceChain.resize(nChainLength);
			for (int i = 0; i < nChainLength; ++i)
			{
				m_vecLuminanceChain[i] = IRenderTarget::Create(rtDesc);

				rtDesc.Width /= 4;
				rtDesc.Height /= 4;
			}

			rtDesc.Width = 1;
			rtDesc.Height = 1;

			m_pLuminanceCurrent = IRenderTarget::Create(rtDesc);
			m_pLuminanceLast = IRenderTarget::Create(rtDesc);
			m_pAdaptedLuminance = IRenderTarget::Create(rtDesc);

			return true;
		}

		void HDRFilter::Release()
		{
			if (m_isInit == false)
				return;

			std::for_each(m_vecLuminanceChain.begin(), m_vecLuminanceChain.end(), [](IRenderTarget* pRenderTarget)
			{
				SafeDelete(pRenderTarget);
			});
			m_vecLuminanceChain.clear();
			
			SafeDelete(m_pLuminanceCurrent);
			SafeDelete(m_pLuminanceLast);
			SafeDelete(m_pAdaptedLuminance);

			IEffect::Destroy(&m_pEffect);

			m_pSamplerPoint = nullptr;
			m_pSamplerLinear = nullptr;

			m_isInit = false;
		}

		void Apply(IDeviceContext* pDeviceContext, IEffect* pEffect, IEffectTech* pTech, IRenderTarget* pResult)
		{
			Math::Viewport viewport;
			viewport.width = static_cast<float>(pResult->GetSize().x);
			viewport.height = static_cast<float>(pResult->GetSize().y);
			pDeviceContext->SetViewport(viewport);
			pDeviceContext->SetRenderTargets(&pResult, 1);

			uint32_t nPassCount = pTech->GetPassCount();
			for (uint32_t p = 0; p < nPassCount; ++p)
			{
				pTech->PassApply(p, pDeviceContext);

				pDeviceContext->Draw(4, 0);
			}
		}

		bool HDRFilter::ToneMap(IRenderTarget* pResult, IRenderTarget* pSource, float fElapsedTime)
		{
			if (pResult == nullptr || pResult->GetTexture() == nullptr)
				return false;

			if (pSource == nullptr || pSource->GetTexture() == nullptr)
				return false;

			D3D_PROFILING(HDRFilter);

			IDevice* pDevice = GetDevice();
			IDeviceContext* pDeviceContext = GetDeviceContext();
			
			{
				D3D_PROFILING(ResetD3D);

				pDeviceContext->ClearState();
				pDeviceContext->SetDefaultViewport();
			}

			pDeviceContext->SetRasterizerState(EmRasterizerState::eSolidCCW);
			pDeviceContext->SetDepthStencilState(EmDepthStencilState::eOff);
			pDeviceContext->SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

			RenderTargetDesc2D desc = pSource->GetDesc2D();
			desc.Width /= 16;
			desc.Height /= 16;
			desc.Build();
			IRenderTarget* pDownscale = pDevice->GetRenderTarget(desc, false);
			if (Downscale::GetInstance()->Apply16SW(pDownscale, pSource) == false)
			{
				pDevice->ReleaseRenderTargets(&pDownscale);
				return false;
			}

			// CalculateAverageLuminance
			{
				D3D_PROFILING(CalculateAverageLuminance);

				// Calculate the initial luminance
				IEffectTech* pTech = m_pEffect->GetTechnique(StrID::Luminance);
				if (pTech == nullptr)
				{
					assert(false);
					return false;
				}

				m_pEffect->SetTexture(StrID::g_texColor, pDownscale->GetTexture());
				m_pEffect->SetSamplerState(StrID::g_samplerPoint, m_pSamplerPoint, 0);
				m_pEffect->SetSamplerState(StrID::g_samplerLinear, m_pSamplerLinear, 0);

				Apply(pDeviceContext, m_pEffect, pTech, m_vecLuminanceChain[0]);
				ClearEffect(pDeviceContext, pTech);

				// Repeatedly downscale
				for (uint32_t i = 1; i < m_vecLuminanceChain.size(); ++i)
				{
					Downscale::GetInstance()->Apply4SW(m_vecLuminanceChain[i], m_vecLuminanceChain[i - 1]);
				}

				// Final downscale
				Downscale::GetInstance()->Apply4SW(m_pLuminanceCurrent, m_vecLuminanceChain.back(), true);

				// Adapt the luminance, to simulate slowly adjust exposure
				m_pEffect->SetFloat(StrID::g_fElapsedTime, fElapsedTime);

				pTech = m_pEffect->GetTechnique(StrID::CalcAdaptedLuminance);
				if (pTech == nullptr)
				{
					assert(false);
					return false;
				}

				m_pEffect->SetTexture(StrID::g_texLuminanceCur, m_pLuminanceCurrent->GetTexture());
				m_pEffect->SetTexture(StrID::g_texLuminanceLast, m_pLuminanceLast->GetTexture());

				m_pEffect->SetSamplerState(StrID::g_samplerPoint, m_pSamplerPoint, 0);
				m_pEffect->SetSamplerState(StrID::g_samplerLinear, m_pSamplerLinear, 0);

				Apply(pDeviceContext, m_pEffect, pTech, m_pAdaptedLuminance);
				ClearEffect(pDeviceContext, pTech);
			}

            // Do the bloom first
			IRenderTarget* pThreshold = pDevice->GetRenderTarget(pDownscale->GetDesc2D(), false);
			{
				D3D_PROFILING(Threshold);

				m_pEffect->SetFloat(StrID::g_fThreshold, m_fBloomThreshold);
				m_pEffect->SetFloat(StrID::g_fMiddleGrey, m_fToneMapKey);
				m_pEffect->SetFloat(StrID::g_fMaxLuminance, m_fMaxLuminance);

				IEffectTech* pTech = m_pEffect->GetTechnique(StrID::Threshold);
				if (pTech == nullptr)
				{
					assert(false);
					return false;
				}

				m_pEffect->SetTexture(StrID::g_texColor, pDownscale->GetTexture());
				m_pEffect->SetTexture(StrID::g_texLuminanceCur, m_pAdaptedLuminance->GetTexture());

				m_pEffect->SetSamplerState(StrID::g_samplerPoint, m_pSamplerPoint, 0);
				m_pEffect->SetSamplerState(StrID::g_samplerLinear, m_pSamplerLinear, 0);

				Apply(pDeviceContext, m_pEffect, pTech, pThreshold);
				ClearEffect(pDeviceContext, pTech);
			}

			IRenderTarget* pPostBlur = pDevice->GetRenderTarget(pDownscale->GetDesc2D(), false);
			GaussianBlur::GetInstance()->Apply(pPostBlur, pThreshold, m_fBlurSigma);
			pDevice->ReleaseRenderTargets(&pThreshold);

			IRenderTarget* pCombine = pPostBlur;
			if (m_isEnableLensFlare == true)
			{
				// LensFlare
				Math::Vector2 f2SourceDimensions;
				f2SourceDimensions.x = static_cast<float>(pPostBlur->GetSize().x);
				f2SourceDimensions.y = static_cast<float>(pPostBlur->GetSize().y);

				m_pEffect->SetVector(StrID::g_f2SourceDimensions, f2SourceDimensions);

				// Calculate the lens flare, first pass
				IRenderTarget* pLensFlare1 = pDevice->GetRenderTarget(pDownscale->GetDesc2D(), false);
				IEffectTech* pTech = m_pEffect->GetTechnique(StrID::LensFlareFirstPass);
				if (pTech == nullptr)
				{
					assert(false);
					return false;
				}

				m_pEffect->SetTexture(StrID::g_texColor, pPostBlur->GetTexture());

				m_pEffect->SetSamplerState(StrID::g_samplerPoint, m_pSamplerPoint, 0);
				m_pEffect->SetSamplerState(StrID::g_samplerLinear, m_pSamplerLinear, 0);

				Apply(pDeviceContext, m_pEffect, pTech, pLensFlare1);
				ClearEffect(pDeviceContext, pTech);

				// Calculate the lens flare, second pass
				IRenderTarget* pLensFlare2 = pDevice->GetRenderTarget(pDownscale->GetDesc2D(), false);
				pTech = m_pEffect->GetTechnique(StrID::LensFlareSecondPass);
				if (pTech == nullptr)
				{
					assert(false);
					return false;
				}

				m_pEffect->SetTexture(StrID::g_texColor, pPostBlur->GetTexture());

				m_pEffect->SetSamplerState(StrID::g_samplerPoint, m_pSamplerPoint, 0);
				m_pEffect->SetSamplerState(StrID::g_samplerLinear, m_pSamplerLinear, 0);

				Apply(pDeviceContext, m_pEffect, pTech, pLensFlare2);
				ClearEffect(pDeviceContext, pTech);

				// Combine the lens flare with the bloom
				pCombine = pDevice->GetRenderTarget(pDownscale->GetDesc2D(), false);
				pTech = m_pEffect->GetTechnique(StrID::Combine);
				if (pTech == nullptr)
				{
					assert(false);
					return false;
				}

				m_pEffect->SetTexture(StrID::g_texColor, pPostBlur->GetTexture());

				m_pEffect->SetTexture(StrID::g_texLensFlare1, pLensFlare1->GetTexture());
				m_pEffect->SetTexture(StrID::g_texLensFlare2, pLensFlare2->GetTexture());

				m_pEffect->SetSamplerState(StrID::g_samplerPoint, m_pSamplerPoint, 0);
				m_pEffect->SetSamplerState(StrID::g_samplerLinear, m_pSamplerLinear, 0);

				Apply(pDeviceContext, m_pEffect, pTech, pCombine);
				ClearEffect(pDeviceContext, pTech);

				pDevice->ReleaseRenderTargets(&pPostBlur);
				pDevice->ReleaseRenderTargets(&pLensFlare1);
				pDevice->ReleaseRenderTargets(&pLensFlare2);
			}

			// Scale it back to half of full size (will do the final scaling step when sampling
			// the bloom texture during tone mapping).
			desc = pCombine->GetDesc2D();
			desc.Width *= 2;
			desc.Height *= 2;
			desc.Build();
			IRenderTarget* pUpScale1 = pDevice->GetRenderTarget(desc, false);
			Downscale::GetInstance()->ApplyHW(pUpScale1, pCombine);
			pDevice->ReleaseRenderTargets(&pCombine);

			desc = pUpScale1->GetDesc2D();
			desc.Width *= 2;
			desc.Height *= 2;
			desc.Build();
			IRenderTarget* pUpScale2 = pDevice->GetRenderTarget(desc, false);
			Downscale::GetInstance()->ApplyHW(pUpScale2, pUpScale1);
			pDevice->ReleaseRenderTargets(&pUpScale1);

			desc = pUpScale2->GetDesc2D();
			desc.Width *= 2;
			desc.Height *= 2;
			desc.Build();
			IRenderTarget* pBloom = pDevice->GetRenderTarget(desc, false);
			Downscale::GetInstance()->ApplyHW(pBloom, pUpScale2);
			pDevice->ReleaseRenderTargets(&pUpScale2);

			{
				D3D_PROFILING(Apply);

				m_pEffect->SetFloat(StrID::g_fMiddleGrey, m_fToneMapKey);
				m_pEffect->SetFloat(StrID::g_fMaxLuminance, m_fMaxLuminance);
				m_pEffect->SetFloat(StrID::g_fBloomMultiplier, m_fBloomMultiplier);

				m_pEffect->SetTexture(StrID::g_texColor, pSource->GetTexture());
				m_pEffect->SetTexture(StrID::g_texLuminanceCur, m_pAdaptedLuminance->GetTexture());
				m_pEffect->SetTexture(StrID::g_texBloom, pBloom->GetTexture());

				IEffectTech* pTech = m_pEffect->GetTechnique(StrID::ToneMap);
				if (pTech == nullptr)
				{
					assert(false);
					return false;
				}

				m_pEffect->SetSamplerState(StrID::g_samplerPoint, m_pSamplerPoint, 0);
				m_pEffect->SetSamplerState(StrID::g_samplerLinear, m_pSamplerLinear, 0);

				Apply(pDeviceContext, m_pEffect, pTech, pResult);
				ClearEffect(pDeviceContext, pTech);
			}

			pDevice->ReleaseRenderTargets(&pBloom);
			pDevice->ReleaseRenderTargets(&pDownscale);

			std::swap(m_pLuminanceCurrent, m_pLuminanceLast);

			return true;
		}

		void HDRFilter::ClearEffect(IDeviceContext* pd3dDeviceContext, IEffectTech* pEffectTech)
		{
			m_pEffect->SetTexture(StrID::g_texColor, nullptr);
			m_pEffect->SetTexture(StrID::g_texBloom, nullptr);
			m_pEffect->SetTexture(StrID::g_texLuminanceCur, nullptr);
			m_pEffect->SetTexture(StrID::g_texLuminanceLast, nullptr);
			m_pEffect->SetTexture(StrID::g_texLensFlare1, nullptr);
			m_pEffect->SetTexture(StrID::g_texLensFlare2, nullptr);
			m_pEffect->UndoSamplerState(StrID::g_samplerPoint, 0);
			m_pEffect->UndoSamplerState(StrID::g_samplerLinear, 0);
		}
	}
}