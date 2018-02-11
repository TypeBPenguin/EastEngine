#include "stdafx.h"
#include "GaussianBlur.h"

#include "CommonLib/FileUtil.h"

namespace StrID
{
	RegisterStringID(EffectGaussianBlur);
	RegisterStringID(GaussianBlurH);
	RegisterStringID(GaussianBlurV);
	RegisterStringID(GaussianDepthBlurH);
	RegisterStringID(GaussianDepthBlurV);

	RegisterStringID(g_fSigma);
	RegisterStringID(g_f2SourceDimensions);
	RegisterStringID(g_texColor);
	RegisterStringID(g_texDepth);
	RegisterStringID(g_samplerPoint);
}

namespace EastEngine
{
	namespace Graphics
	{
		GaussianBlur::GaussianBlur()
			: m_isInit(false)
			, m_pEffect(nullptr)
			, m_pSamplerPoint(nullptr)
		{
		}

		GaussianBlur::~GaussianBlur()
		{
			Release();
		}

		bool GaussianBlur::Init()
		{
			if (m_isInit == true)
				return true;

			m_isInit = true;

			std::string strPath(File::GetPath(File::EmPath::eFx));

#if defined(DEBUG) || defined(_DEBUG)
			strPath.append("PostProcessing\\GaussianBlur\\GaussianBlur_D.cso");
#else
			strPath.append("PostProcessing\\GaussianBlur\\GaussianBlur.cso");
#endif

			m_pEffect = IEffect::Create(StrID::EffectGaussianBlur, strPath.c_str());
			if (m_pEffect == nullptr)
				return false;

			m_pEffect->CreateTechnique(StrID::GaussianBlurH, EmVertexFormat::eUnknown);
			m_pEffect->CreateTechnique(StrID::GaussianBlurV, EmVertexFormat::eUnknown);
			m_pEffect->CreateTechnique(StrID::GaussianDepthBlurH, EmVertexFormat::eUnknown);
			m_pEffect->CreateTechnique(StrID::GaussianDepthBlurV, EmVertexFormat::eUnknown);

			SamplerStateDesc desc;
			desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
			desc.MaxAnisotropy = 1;
			desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
			desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;

			m_pSamplerPoint = ISamplerState::Create(desc);

			return true;
		}

		void GaussianBlur::Release()
		{
			if (m_isInit == false)
				return;

			IEffect::Destroy(&m_pEffect);

			m_pSamplerPoint = nullptr;

			m_isInit = false;
		}

		void ApplyBlur(IDeviceContext* pDeviceContext, IEffect* pEffect, IEffectTech* pTech, IRenderTarget* pResult, IRenderTarget* pSource)
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

		bool GaussianBlur::Apply(IDevice* pDevice, IDeviceContext* pDeviceContext, IRenderTarget* pResult, IRenderTarget* pSource, float fSigma)
		{
			if (pResult == nullptr || pResult->GetTexture() == nullptr)
				return false;

			if (pSource == nullptr || pSource->GetTexture() == nullptr)
				return false;

			PERF_TRACER_EVENT("GaussianBlur::Apply", "");
			D3D_PROFILING(pDeviceContext, GaussianBlur);

			pDeviceContext->ClearState();
			pDeviceContext->SetDefaultViewport();
			pDeviceContext->SetRasterizerState(EmRasterizerState::eSolidCCW);
			pDeviceContext->SetDepthStencilState(EmDepthStencilState::eRead_Write_Off);
			pDeviceContext->SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

			m_pEffect->SetFloat(StrID::g_fSigma, fSigma);

			// GaussianBlurH
			IEffectTech* pTech = m_pEffect->GetTechnique(StrID::GaussianBlurH);
			if (pTech == nullptr)
			{
				assert(false);
				return false;
			}

			const RenderTargetDesc2D& desc = pSource->GetDesc2D();
			IRenderTarget* pGaussianBlur = pDevice->GetRenderTarget(desc, false);
			{
				D3D_PROFILING(pDeviceContext, GaussianBlurH);

				m_pEffect->SetSamplerState(StrID::g_samplerPoint, m_pSamplerPoint, 0);

				ApplyBlur(pDeviceContext, m_pEffect, pTech, pGaussianBlur, pSource);
				ClearEffect(pDeviceContext, pTech);
			}

			// GaussianBlurV
			pTech = m_pEffect->GetTechnique(StrID::GaussianBlurV);
			if (pTech == nullptr)
			{
				pDevice->ReleaseRenderTargets(&pGaussianBlur);

				assert(false);
				return false;
			}

			{
				D3D_PROFILING(pDeviceContext, GaussianBlurV);

				m_pEffect->SetSamplerState(StrID::g_samplerPoint, m_pSamplerPoint, 0);

				ApplyBlur(pDeviceContext, m_pEffect, pTech, pResult, pGaussianBlur);
				ClearEffect(pDeviceContext, pTech);

				pDevice->ReleaseRenderTargets(&pGaussianBlur);
			}

			return true;
		}

		bool GaussianBlur::Apply(IDevice* pDevice, IDeviceContext* pDeviceContext, IRenderTarget* pResult, IRenderTarget* pSource, IRenderTarget* pDepth, float fSigma)
		{
			if (pResult == nullptr || pResult->GetTexture() == nullptr)
				return false;

			if (pSource == nullptr || pSource->GetTexture() == nullptr)
				return false;

			if (pDepth == nullptr || pDepth->GetTexture() == nullptr)
				return false;

			PERF_TRACER_EVENT("GaussianBlur::Apply", "");
			D3D_PROFILING(pDeviceContext, GaussianBlur);

			pDeviceContext->ClearState();
			pDeviceContext->SetDefaultViewport();
			pDeviceContext->SetRasterizerState(EmRasterizerState::eSolidCCW);
			pDeviceContext->SetDepthStencilState(EmDepthStencilState::eRead_Write_Off);
			pDeviceContext->SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

			m_pEffect->SetFloat(StrID::g_fSigma, fSigma);

			// GaussianDepthBlurH
			IEffectTech* pTech = m_pEffect->GetTechnique(StrID::GaussianDepthBlurH);
			if (pTech == nullptr)
			{
				assert(false);
				return false;
			}

			const RenderTargetDesc2D& desc = pSource->GetDesc2D();
			IRenderTarget* pGaussianBlur = pDevice->GetRenderTarget(desc, false);
			{
				D3D_PROFILING(pDeviceContext, GaussianDepthBlurH);

				m_pEffect->SetTexture(StrID::g_texDepth, pDepth->GetTexture());
				m_pEffect->SetSamplerState(StrID::g_samplerPoint, m_pSamplerPoint, 0);

				ApplyBlur(pDeviceContext, m_pEffect, pTech, pGaussianBlur, pSource);
				ClearEffect(pDeviceContext, pTech);
			}

			// GaussianDepthBlurV
			pTech = m_pEffect->GetTechnique(StrID::GaussianDepthBlurV);
			if (pTech == nullptr)
			{
				pDevice->ReleaseRenderTargets(&pGaussianBlur);

				assert(false);
				return false;
			}

			{
				D3D_PROFILING(pDeviceContext, GaussianDepthBlurV);

				m_pEffect->SetTexture(StrID::g_texDepth, pDepth->GetTexture());
				m_pEffect->SetSamplerState(StrID::g_samplerPoint, m_pSamplerPoint, 0);

				ApplyBlur(pDeviceContext, m_pEffect, pTech, pResult, pGaussianBlur);
				ClearEffect(pDeviceContext, pTech);

				pDevice->ReleaseRenderTargets(&pGaussianBlur);
			}

			return true;
		}

		void GaussianBlur::ClearEffect(IDeviceContext* pd3dDeviceContext, IEffectTech* pEffectTech)
		{
			m_pEffect->SetTexture(StrID::g_texColor, nullptr);
			m_pEffect->SetTexture(StrID::g_texDepth, nullptr);
			m_pEffect->UndoSamplerState(StrID::g_samplerPoint, 0);

			m_pEffect->ClearState(pd3dDeviceContext, pEffectTech);
		}
	}
}