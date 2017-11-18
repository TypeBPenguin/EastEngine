#include "stdafx.h"
#include "DepthOfField.h"

#include "CommonLib/FileUtil.h"

#include "DirectX/CameraManager.h"

#include "GaussianBlur.h"
#include "Downscale.h"

namespace StrID
{
	RegisterStringID(EffectDepthOfField);
	RegisterStringID(DOFDiscBlur);

	RegisterStringID(g_fFocalDistance);
	RegisterStringID(g_fFocalWidth);
	RegisterStringID(g_f2FilterTaps);
	RegisterStringID(g_matInvProj);
	RegisterStringID(g_texColor);
	RegisterStringID(g_texDepth);
	RegisterStringID(g_samplerPoint);
	RegisterStringID(g_samplerLinear);
}

namespace EastEngine
{
	namespace Graphics
	{
		DepthOfField::DepthOfField()
			: m_isInit(false)
			, m_pSamplerPoint(nullptr)
		{
		}

		DepthOfField::~DepthOfField()
		{
			Release();
		}

		bool DepthOfField::Init()
		{
			if (m_isInit == true)
				return true;

			m_isInit = true;

			std::string strPath(File::GetPath(File::EmPath::eFx));

#if defined(DEBUG) || defined(_DEBUG)
			strPath.append("PostProcessing\\DepthOfField\\DepthOfField_D.cso");
#else
			strPath.append("PostProcessing\\DepthOfField\\DepthOfField.cso");
#endif

			m_pEffect = IEffect::Create(StrID::EffectDepthOfField, strPath.c_str());
			if (m_pEffect == nullptr)
				return false;

			m_pEffect->CreateTechnique(StrID::DOFDiscBlur, EmVertexFormat::ePos);

			m_pSamplerPoint = GetDevice()->GetSamplerState(EmSamplerState::eMinMagMipPointClamp);
			m_pSamplerLinear = GetDevice()->GetSamplerState(EmSamplerState::eMinMagMipLinearClamp);

			return true;
		}

		void DepthOfField::Release()
		{
			if (m_isInit == false)
				return;

			IEffect::Destroy(&m_pEffect);

			m_pSamplerPoint = nullptr;
			m_pSamplerLinear = nullptr;

			m_isInit = false;
		}

		bool DepthOfField::Apply(IRenderTarget* pResult, IRenderTarget* pSource, const std::shared_ptr<ITexture>& pDepth)
		{
			if (pResult == nullptr || pResult->GetTexture() == nullptr)
				return false;

			if (pSource == nullptr || pSource->GetTexture() == nullptr)
				return false;

			D3D_PROFILING(DepthOfField);

			Camera* pCamera = CameraManager::GetInstance()->GetMainCamera();
			if (pCamera == nullptr)
				return false;
			
			IDeviceContext* pDeviceContext = GetDeviceContext();
			if (pDeviceContext->SetInputLayout(EmVertexFormat::ePos) == false)
				return false;

			pDeviceContext->ClearState();

			pDeviceContext->SetRasterizerState(EmRasterizerState::eCCW);
			pDeviceContext->SetDepthStencilState(EmDepthStencilState::eOff);
			pDeviceContext->SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

			m_pEffect->SetFloat(StrID::g_fFocalDistance, m_setting.fFocalDistnace);
			m_pEffect->SetFloat(StrID::g_fFocalWidth, m_setting.fFocalWidth * 0.5f);

			m_pEffect->SetMatrix(StrID::g_matInvProj, pCamera->GetProjMatrix().Invert());

			m_pEffect->SetSamplerState(StrID::g_samplerPoint, m_pSamplerPoint, 0);
			m_pEffect->SetSamplerState(StrID::g_samplerLinear, m_pSamplerLinear, 0);

			m_pEffect->SetTexture(StrID::g_texDepth, pDepth);
			m_pEffect->SetTexture(StrID::g_texColor, pSource->GetTexture());

			Math::Viewport viewport;
			viewport.width = static_cast<float>(pResult->GetSize().x);
			viewport.height = static_cast<float>(pResult->GetSize().y);
			pDeviceContext->SetViewport(viewport);
			pDeviceContext->SetRenderTargets(&pResult, 1);

			IEffectTech* pTech = m_pEffect->GetTechnique(StrID::DOFDiscBlur);
			if (pTech == nullptr)
			{
				assert(false);
				return false;
			}

			// Scale tap offsets based on render target size
			float dx = 0.5f / static_cast<float>(pSource->GetSize().x);
			float dy = 0.5f / static_cast<float>(pSource->GetSize().y);

			// Generate the texture coordinate offsets for our disc
			Math::Vector2 discOffsets[12];
			discOffsets[0] = Math::Vector2(-0.326212f * dx, -0.40581f * dy);
			discOffsets[1] = Math::Vector2(-0.840144f * dx, -0.07358f * dy);
			discOffsets[2] = Math::Vector2(-0.840144f * dx, 0.457137f * dy);
			discOffsets[3] = Math::Vector2(-0.203345f * dx, 0.620716f * dy);
			discOffsets[4] = Math::Vector2(0.96234f * dx, -0.194983f * dy);
			discOffsets[5] = Math::Vector2(0.473434f * dx, -0.480026f * dy);
			discOffsets[6] = Math::Vector2(0.519456f * dx, 0.767022f * dy);
			discOffsets[7] = Math::Vector2(0.185461f * dx, -0.893124f * dy);
			discOffsets[8] = Math::Vector2(0.507431f * dx, 0.064425f * dy);
			discOffsets[9] = Math::Vector2(0.89642f * dx, 0.412458f * dy);
			discOffsets[10] = Math::Vector2(-0.32194f * dx, -0.932615f * dy);
			discOffsets[11] = Math::Vector2(-0.791559f * dx, -0.59771f * dy);

			m_pEffect->SetVectorArray(StrID::g_f2FilterTaps, discOffsets, 0, 12);

			uint32_t nPassCount = pTech->GetPassCount();
			for (uint32_t p = 0; p < nPassCount; ++p)
			{
				pTech->PassApply(p, pDeviceContext);

				pDeviceContext->Draw(4, 0);
			}

			ClearEffect(pDeviceContext, pTech);

			return true;
		}

		void DepthOfField::ClearEffect(IDeviceContext* pd3dDeviceContext, IEffectTech* pTech)
		{
			m_pEffect->SetTexture(StrID::g_texColor, nullptr);
			m_pEffect->SetTexture(StrID::g_texDepth, nullptr);

			m_pEffect->UndoSamplerState(StrID::g_samplerPoint, 0);
			m_pEffect->UndoSamplerState(StrID::g_samplerLinear, 0);

			m_pEffect->ClearState(pd3dDeviceContext, pTech);
		}
	}
}