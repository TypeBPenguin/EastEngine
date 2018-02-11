#include "stdafx.h"
#include "ColorGrading.h"

#include "CommonLib/FileUtil.h"

namespace StrID
{
	RegisterStringID(EffectColorGrading);
	RegisterStringID(ColorGrading);

	RegisterStringID(g_colorGuide);

	RegisterStringID(g_texSrc);
	RegisterStringID(g_samLinearWrap);
}

namespace EastEngine
{
	namespace Graphics
	{
		ColorGrading::ColorGrading()
			: m_isInit(false)
			, m_f3ColorGuide(1.f, 0.588f, 0.529f)
			, m_pEffect(nullptr)
			, m_pSamplerState(nullptr)
		{
		}

		ColorGrading::~ColorGrading()
		{
			Release();
		}

		bool ColorGrading::Init()
		{
			if (m_isInit == true)
				return true;

			m_isInit = true;

			std::string strPath(File::GetPath(File::EmPath::eFx));

#if defined(DEBUG) || defined(_DEBUG)
			strPath.append("PostProcessing\\ColorGrading\\ColorGrading_D.cso");
#else
			strPath.append("PostProcessing\\ColorGrading\\ColorGrading.cso");
#endif

			m_pEffect = IEffect::Create(StrID::EffectColorGrading, strPath.c_str());
			if (m_pEffect == nullptr)
				return false;

			m_pEffect->CreateTechnique(StrID::ColorGrading, EmVertexFormat::eUnknown);

			m_pSamplerState = GetDevice()->GetSamplerState(EmSamplerState::eMinMagMipLinearWrap);

			return true;
		}

		void ColorGrading::Release()
		{
			if (m_isInit == false)
				return;

			IEffect::Destroy(&m_pEffect);

			m_isInit = false;
		}

		bool ColorGrading::Apply(IDevice* pDevice, IDeviceContext* pDeviceContext, IRenderTarget* pResult, IRenderTarget* pSource)
		{
			if (pResult == nullptr || pResult->GetTexture() == nullptr)
				return false;

			if (pSource == nullptr || pSource->GetTexture() == nullptr)
				return false;

			PERF_TRACER_EVENT("ColorGrading::Apply", "");
			D3D_PROFILING(pDeviceContext, ColorGrading);

			IEffectTech* pEffectTech = m_pEffect->GetTechnique(StrID::ColorGrading);
			if (pEffectTech == nullptr)
			{
				LOG_ERROR("Not Exist EffectTech !!");
				return false;
			}

			pDeviceContext->ClearState();

			Math::Viewport viewport;
			viewport.width = static_cast<float>(pResult->GetSize().x);
			viewport.height = static_cast<float>(pResult->GetSize().y);
			pDeviceContext->SetViewport(viewport);
			pDeviceContext->SetRenderTargets(&pResult, 1);

			pDeviceContext->SetRasterizerState(EmRasterizerState::eSolidCCW);
			pDeviceContext->SetDepthStencilState(EmDepthStencilState::eRead_Write_Off);

			pDeviceContext->SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

			m_pEffect->SetVector(StrID::g_colorGuide, m_f3ColorGuide);
			m_pEffect->SetTexture(StrID::g_texSrc, pSource->GetTexture());
			m_pEffect->SetSamplerState(StrID::g_samLinearWrap, m_pSamplerState, 0);

			const uint32_t nPassCount = pEffectTech->GetPassCount();
			for (uint32_t p = 0; p < nPassCount; ++p)
			{
				pEffectTech->PassApply(p, pDeviceContext);

				pDeviceContext->Draw(4, 0);
			}

			ClearEffect(pDeviceContext, pEffectTech);

			return true;
		}

		void ColorGrading::ClearEffect(IDeviceContext* pd3dDeviceContext, IEffectTech* pTech)
		{
			m_pEffect->SetTexture(StrID::g_texSrc, nullptr);
			m_pEffect->UndoSamplerState(StrID::g_samLinearWrap, 0);

			m_pEffect->ClearState(pd3dDeviceContext, pTech);
		}
	}
}