#include "stdafx.h"
#include "SSS.h"

#include "CommonLib/FileUtil.h"

namespace StrID
{
	RegisterStringID(EffectSSS);
	RegisterStringID(SSS);

	RegisterStringID(g_texColor);
	RegisterStringID(g_texDepth);
	RegisterStringID(g_sssWidth);
}

namespace EastEngine
{
	namespace Graphics
	{
		SSS::SSS()
			: m_isInit(false)
			, m_pEffect(nullptr)
			, m_fSSSWidth(0.1f)
		{
		}

		SSS::~SSS()
		{
			Release();
		}

		bool SSS::Init()
		{
			if (m_isInit == true)
				return true;

			m_isInit = true;

			std::string strPath(File::GetPath(File::EmPath::eFx));

#if defined(DEBUG) || defined(_DEBUG)
			strPath.append("PostProcessing\\SSS\\SSS_D.cso");
#else
			strPath.append("PostProcessing\\SSS\\SSS.cso");
#endif

			m_pEffect = IEffect::Create(StrID::EffectSSS, strPath.c_str());
			if (m_pEffect == nullptr)
				return false;

			m_pEffect->CreateTechnique(StrID::SSS, EmVertexFormat::eUnknown);

			return true;
		}

		void SSS::Release()
		{
			if (m_isInit == false)
				return;

			IEffect::Destroy(&m_pEffect);

			m_isInit = false;
		}

		bool SSS::Apply(IDevice* pDevice, IDeviceContext* pDeviceContext, IRenderTarget* pResult, IRenderTarget* pSource, const std::shared_ptr<ITexture>& pDepth)
		{
			if (pResult == nullptr || pResult->GetTexture() == nullptr)
				return false;

			if (pSource == nullptr || pSource->GetTexture() == nullptr)
				return false;

			D3D_PROFILING(pDeviceContext, SSS);

			pDeviceContext->ClearState();

			pDeviceContext->SetRasterizerState(EmRasterizerState::eSolidCCW);
			pDeviceContext->SetDepthStencilState(EmDepthStencilState::eRead_Write_Off);
			pDeviceContext->SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

			m_pEffect->SetFloat(StrID::g_sssWidth, m_fSSSWidth);

			m_pEffect->SetTexture(StrID::g_texDepth, pDepth);
			m_pEffect->SetTexture(StrID::g_texColor, pSource->GetTexture());

			Math::Viewport viewport;
			viewport.width = static_cast<float>(pResult->GetSize().x);
			viewport.height = static_cast<float>(pResult->GetSize().y);
			pDeviceContext->SetViewport(viewport);
			pDeviceContext->SetRenderTargets(&pResult, 1);

			IEffectTech* pTech = m_pEffect->GetTechnique(StrID::SSS);
			if (pTech == nullptr)
			{
				assert(false);
				return false;
			}

			const uint32_t nPassCount = pTech->GetPassCount();
			for (uint32_t p = 0; p < nPassCount; ++p)
			{
				pTech->PassApply(p, pDeviceContext);

				pDeviceContext->Draw(4, 0);
			}

			ClearEffect(pDeviceContext, pTech);

			return true;
		}

		void SSS::ClearEffect(IDeviceContext* pd3dDeviceContext, IEffectTech* pTech)
		{
			m_pEffect->SetTexture(StrID::g_texColor, nullptr);
			m_pEffect->SetTexture(StrID::g_texDepth, nullptr);

			m_pEffect->ClearState(pd3dDeviceContext, pTech);
		}
	}
}