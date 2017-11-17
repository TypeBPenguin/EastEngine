#include "stdafx.h"
#include "ColorGrading.h"

#include "../CommonLib/FileUtil.h"

namespace StrID
{
	RegisterStringID(EffectColorGrading);
	RegisterStringID(ColorGrading);

	RegisterStringID(g_texSrc);
	RegisterStringID(g_samLinearWrap);
}

namespace EastEngine
{
	namespace Graphics
	{
		ColorGrading::ColorGrading()
			: m_isInit(false)
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

		bool ColorGrading::Apply(IRenderTarget* pResult, IRenderTarget* pSource)
		{
			if (pResult == nullptr || pResult->GetTexture() == nullptr)
				return false;

			if (pSource == nullptr || pSource->GetTexture() == nullptr)
				return false;

			D3D_PROFILING(ColorGrading);

			IEffectTech* pEffectTech = m_pEffect->GetTechnique(StrID::ColorGrading);
			if (pEffectTech == nullptr)
			{
				PRINT_LOG("Not Exist EffectTech !!");
				return false;
			}

			IDeviceContext* pDeviceContext = GetDeviceContext();

			pDeviceContext->ClearState();

			Math::Viewport viewport;
			viewport.width = static_cast<float>(pResult->GetSize().x);
			viewport.height = static_cast<float>(pResult->GetSize().y);
			pDeviceContext->SetViewport(viewport);
			pDeviceContext->SetRenderTargets(&pResult, 1);

			pDeviceContext->SetRasterizerState(EmRasterizerState::eCCW);
			pDeviceContext->SetDepthStencilState(EmDepthStencilState::eOff);

			pDeviceContext->SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

			m_pEffect->SetTexture(StrID::g_texSrc, pSource->GetTexture());

			m_pEffect->SetSamplerState(StrID::g_samLinearWrap, m_pSamplerState, 0);

			uint32_t nPassCount = pEffectTech->GetPassCount();
			for (uint32_t p = 0; p < nPassCount; ++p)
			{
				pEffectTech->PassApply(p, pDeviceContext);

				pDeviceContext->Draw(4, 0);
			}

			ClearEffect(pDeviceContext, pEffectTech);

			return true;
		}

		void ColorGrading::Flush()
		{
		}

		void ColorGrading::ClearEffect(IDeviceContext* pd3dDeviceContext, IEffectTech* pTech)
		{
			m_pEffect->SetTexture(StrID::g_texSrc, nullptr);
			m_pEffect->UndoSamplerState(StrID::g_samLinearWrap, 0);

			m_pEffect->ClearState(pd3dDeviceContext, pTech);
		}
	}
}