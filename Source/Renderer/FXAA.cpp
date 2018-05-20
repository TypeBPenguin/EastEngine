#include "stdafx.h"
#include "FXAA.h"

#include "CommonLib/FileUtil.h"

namespace StrID
{
	RegisterStringID(EffectFXAA);

	RegisterStringID(FXAA);
	RegisterStringID(g_f4RcpFrame);
	RegisterStringID(g_texInput);
	RegisterStringID(g_samAnisotropic);
}

namespace eastengine
{
	namespace graphics
	{
		FXAA::FXAA()
			: m_isInit(false)
			, m_pEffect(nullptr)
			, m_pSamplerAnisotropic(nullptr)
		{
		}

		FXAA::~FXAA()
		{
			Release();
		}

		bool FXAA::Init()
		{
			if (m_isInit == true)
				return true;

			m_isInit = true;

			std::string strPath(file::GetPath(file::EmPath::eFx));

#if defined(DEBUG) || defined(_DEBUG)
			strPath.append("PostProcessing\\FXAA\\FXAA_D.cso");
#else
			strPath.append("PostProcessing\\FXAA\\FXAA.cso");
#endif

			m_pEffect = IEffect::Create(StrID::EffectFXAA, strPath.c_str());
			if (m_pEffect == nullptr)
				return false;

			m_pEffect->CreateTechnique(StrID::FXAA, EmVertexFormat::eUnknown);

			SamplerStateDesc samplerDesc;
			samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
			samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
			samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
			samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
			samplerDesc.MipLODBias = 0.f;
			samplerDesc.MaxAnisotropy = 4;
			samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
			samplerDesc.BorderColor[0] = samplerDesc.BorderColor[1] = samplerDesc.BorderColor[2] = samplerDesc.BorderColor[3] = 0.f;
			samplerDesc.MinLOD = 0.f;
			samplerDesc.MaxLOD = 0.f;

			m_pSamplerAnisotropic = ISamplerState::Create(samplerDesc);

			return true;
		}

		void FXAA::Release()
		{
			if (m_isInit == false)
				return;

			IEffect::Destroy(&m_pEffect);

			m_isInit = false;
		}

		bool FXAA::Apply(IDevice* pDevice, IDeviceContext* pDeviceContext, IRenderTarget* pResult, IRenderTarget* pSource)
		{
			if (pResult == nullptr || pSource == nullptr)
				return false;

			TRACER_EVENT("FXAA::Apply");
			D3D_PROFILING(pDeviceContext, FXAA);

			IEffectTech* pEffectTech = m_pEffect->GetTechnique(StrID::FXAA);
			if (pEffectTech == nullptr)
			{
				LOG_ERROR("Not Exist EffectTech !!");
				return false;
			}

			if (pDeviceContext->SetInputLayout(EmVertexFormat::ePos) == false)
				return false;

			pDeviceContext->ClearState();
			pDeviceContext->SetDefaultViewport();

			pDeviceContext->SetRenderTargets(&pResult, 1, nullptr);
			pDeviceContext->SetDepthStencilState(EmDepthStencilState::eRead_Write_Off);

			pDeviceContext->SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

			const math::UInt2& nScreenSize = pDevice->GetScreenSize();

			math::Vector4 vRcpFrame(1.f / nScreenSize.x, 1.f / nScreenSize.y, 0.f, 0.f);
			m_pEffect->SetVector(StrID::g_f4RcpFrame, vRcpFrame);

			m_pEffect->SetTexture(StrID::g_texInput, pSource->GetTexture());

			m_pEffect->SetSamplerState(StrID::g_samAnisotropic, m_pSamplerAnisotropic, 0);

			uint32_t nPassCount = pEffectTech->GetPassCount();
			for (uint32_t p = 0; p < nPassCount; ++p)
			{
				pEffectTech->PassApply(p, pDeviceContext);

				pDeviceContext->Draw(4, 0);
			}

			ClearEffect(pDeviceContext, pEffectTech);

			return true;
		}
		
		void FXAA::ClearEffect(IDeviceContext* pd3dDeviceContext, IEffectTech* pTech)
		{
			m_pEffect->SetTexture(StrID::g_texInput, nullptr);
			m_pEffect->UndoSamplerState(StrID::g_samAnisotropic, 0);
		}
	}
}