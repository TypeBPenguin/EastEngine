#include "stdafx.h"
#include "SkyRenderer.h"

#include "CommonLib/FileUtil.h"
#include "CommonLib/Config.h"

#include "DirectX/Camera.h"

namespace StrID
{
	RegisterStringID(EffectSky);

	RegisterStringID(Sky);
	RegisterStringID(Skybox);
	RegisterStringID(SkyEffect);
	RegisterStringID(SkyCloud);

	RegisterStringID(g_matWVP);
	RegisterStringID(g_colorApex);
	RegisterStringID(g_colorCenter);
	RegisterStringID(g_fBlend);
	RegisterStringID(g_texSkyCubemap);
	RegisterStringID(g_texEffect);
	RegisterStringID(g_texCloud);
	RegisterStringID(g_texCloudBlend);
	RegisterStringID(g_samLinearWrap);
}

namespace EastEngine
{
	namespace Graphics
	{
		SkyRenderer::SkyRenderer()
			: m_pEffect(nullptr)
		{
		}

		SkyRenderer::~SkyRenderer()
		{
			IEffect::Destroy(&m_pEffect);
		}

		bool SkyRenderer::Init(const Math::Viewport& viewport)
		{
			std::string strPath(File::GetPath(File::EmPath::eFx));

#if defined(DEBUG) || defined(_DEBUG)
			strPath.append("Sky\\Sky_D.cso");
#else
			strPath.append("Sky\\Sky.cso");
#endif

			m_pEffect = IEffect::Create(StrID::EffectSky, strPath.c_str());
			if (m_pEffect == nullptr)
				return false;

			m_pEffect->CreateTechnique(StrID::Sky, EmVertexFormat::ePosTex);

			m_pEffect->CreateTechnique(StrID::Skybox, EmVertexFormat::ePosTexNor);
			m_pEffect->CreateTechnique(StrID::SkyEffect, EmVertexFormat::ePosTex);
			m_pEffect->CreateTechnique(StrID::SkyCloud, EmVertexFormat::ePosTex);

			return true;
		}

		void SkyRenderer::Render(IDevice* pDevice, IDeviceContext* pDeviceContext, Camera* pCamera, uint32_t nRenderGroupFlag)
		{
			D3D_PROFILING(pDeviceContext, SkyRenderer);

			IEffectTech* pEffectTech = m_pEffect->GetTechnique(StrID::Skybox);
			if (pEffectTech == nullptr)
			{
				LOG_ERROR("Not Exist EffectTech !!");
				return;
			}

			int nThreadID = GetThreadID(ThreadType::eRender);

			pDeviceContext->ClearState();

			if (pDeviceContext->SetInputLayout(pEffectTech->GetLayoutFormat()) == false)
				return;

			pDeviceContext->SetDefaultViewport();

			pDeviceContext->SetRasterizerState(EmRasterizerState::eSolidCW);
			pDeviceContext->SetBlendState(EmBlendState::eOff);
			pDeviceContext->SetDepthStencilState(EmDepthStencilState::eRead_Write_Off);

			IRenderTarget* pRenderTarget = nullptr;
			if (Config::IsEnable("HDRFilter"_s) == true)
			{
				auto desc = pDevice->GetMainRenderTarget()->GetDesc2D();
				if (Config::IsEnable("HDRFilter"_s) == true)
				{
					desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
					desc.Build();
				}

				pRenderTarget = pDevice->GetRenderTarget(desc);
			}
			else
			{
				auto& desc = pDevice->GetMainRenderTarget()->GetDesc2D();
				pRenderTarget = pDevice->GetRenderTarget(desc);
			}

			pDeviceContext->ClearRenderTargetView(pRenderTarget, Math::Color::Black);
			pDeviceContext->SetRenderTargets(&pRenderTarget, 1, nullptr);

			pDeviceContext->SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			size_t nSize = m_vecRenderSubsetSkybox[nThreadID].size();
			for (size_t i = 0; i < nSize; ++i)
			{
				RenderSubsetSkybox& renderSubset = m_vecRenderSubsetSkybox[nThreadID][i];

				pDeviceContext->SetVertexBuffers(renderSubset.pVertexBuffer, renderSubset.pVertexBuffer->GetFormatSize(), 0);
				pDeviceContext->SetIndexBuffer(renderSubset.pIndexBuffer, 0);

				Math::Matrix matWorld = Math::Matrix::CreateTranslation(pCamera->GetViewMatrix(nThreadID).Invert().Translation());
				m_pEffect->SetMatrix(StrID::g_matWVP, matWorld * pCamera->GetViewMatrix(nThreadID) * pCamera->GetProjMatrix(nThreadID));

				m_pEffect->SetTexture(StrID::g_texSkyCubemap, renderSubset.pTexSkyCubemap);

				uint32_t nPassCount = pEffectTech->GetPassCount();
				for (uint32_t p = 0; p < nPassCount; ++p)
				{
					pEffectTech->PassApply(p, pDeviceContext);

					pDeviceContext->DrawIndexed(renderSubset.pIndexBuffer->GetIndexNum(), 0, 0);
				}
			}

			ClearEffect(pDeviceContext, pEffectTech);

			/*uint32_t nSize = m_vecRenderSubsetSky.size();
			for (uint32_t i = 0; i < nSize; ++i)
			{
				RenderSubsetSky& renderSubset = m_vecRenderSubsetSky[i];

				pDeviceContext->SetVertexBuffers(renderSubset.pVertexBuffer, renderSubset.pVertexBuffer->GetFormatSize(), 0);
				pDeviceContext->SetIndexBuffer(renderSubset.pIndexBuffer, 0);

				m_pEffect->SetMatrix(StrID::g_matWVP, *renderSubset.pMatrix * pCamera->GetViewMatrix() * pCamera->GetProjMatrix());

				m_pEffect->SetVector(StrID::g_colorApex, reinterpret_cast<Math::Vector4&>(*renderSubset.pColorApex));
				m_pEffect->SetVector(StrID::g_colorCenter, reinterpret_cast<Math::Vector4&>(*renderSubset.pColorCenter));

				uint32_t nPassCount = pEffectTech->GetPassCount();
				for (uint32_t p = 0; p < nPassCount; ++p)
				{
					pEffectTech->PassApply(p, pDeviceContext);

					pDeviceContext->DrawIndexed(renderSubset.pIndexBuffer->GetIndexNum(), 0, 0);
				}
			}

			ClearEffect(pDeviceContext, pEffectTech);

			pEffectTech = m_pEffect->GetTechnique(StrID::SkyEffect);
			if (pEffectTech == nullptr)
			{
				LOG_ERROR("Not Exist EffectTech !!");
				return;
			}

			pDeviceContext->SetBlendState(EmBlendState::eAdditive);

			nSize = m_vecRenderSubsetSkyEffect.size();
			for (uint32_t i = 0; i < nSize; ++i)
			{
				RenderSubsetSkyEffect& renderSubset = m_vecRenderSubsetSkyEffect[i];

				pDeviceContext->SetVertexBuffers(renderSubset.pVertexBuffer, renderSubset.pVertexBuffer->GetFormatSize(), 0);
				pDeviceContext->SetIndexBuffer(renderSubset.pIndexBuffer, 0);

				m_pEffect->SetMatrix(StrID::g_matWVP, *renderSubset.pMatrix * pCamera->GetViewMatrix() * pCamera->GetProjMatrix());

				m_pEffect->SetTexture(StrID::g_texEffect, renderSubset.pTexEffect);

				m_pEffect->SetSamplerState(StrID::g_samLinearWrap, pDevice->GetSamplerState(EmSamplerState::eMinMagMipLinearWrap), 0);

				uint32_t nCount = pEffectTech->GetPassCount();
				for (uint32_t p = 0; p < nCount; ++p)
				{
					pEffectTech->PassApply(p, pDeviceContext);

					pDeviceContext->DrawIndexed(renderSubset.pIndexBuffer->GetIndexNum(), 0, 0);
				}
			}

			ClearEffect(pDeviceContext, pEffectTech);

			pEffectTech = m_pEffect->GetTechnique(StrID::SkyCloud);
			if (pEffectTech == nullptr)
			{
				LOG_ERROR("SkyRenderer::Render() : Not Exist EffectTech !!");
				return;
			}

			nSize = m_vecRenderSubsetSkyCloud.size();
			for (uint32_t i = 0; i < nSize; ++i)
			{
				RenderSubsetSkyCloud& renderSubset = m_vecRenderSubsetSkyCloud[i];

				pDeviceContext->SetVertexBuffers(renderSubset.pVertexBuffer, renderSubset.pVertexBuffer->GetFormatSize(), 0);
				pDeviceContext->SetIndexBuffer(renderSubset.pIndexBuffer, 0);

				m_pEffect->SetMatrix(StrID::g_matWVP, *renderSubset.pMatrix * pCamera->GetViewMatrix() * pCamera->GetProjMatrix());

				m_pEffect->SetFloat(StrID::g_fBlend, renderSubset.fBlend);

				m_pEffect->SetTexture(StrID::g_texCloud, renderSubset.pTexCloud);
				m_pEffect->SetTexture(StrID::g_texCloudBlend, renderSubset.pTexCloudBlend);

				m_pEffect->SetSamplerState(StrID::g_samLinearWrap, pDevice->GetSamplerState(EmSamplerState::eMinMagMipLinearWrap), 0);

				uint32_t nPassCount = pEffectTech->GetPassCount();
				for (uint32_t p = 0; p < nPassCount; ++p)
				{
					pEffectTech->PassApply(p, pDeviceContext);

					pDeviceContext->DrawIndexed(renderSubset.pIndexBuffer->GetIndexNum(), 0, 0);
				}
			}

			ClearEffect(pDeviceContext, pEffectTech);*/

			pDevice->ReleaseRenderTargets(&pRenderTarget, 1);
		}

		void SkyRenderer::Flush()
		{
			int nThreadID = GetThreadID(ThreadType::eRender);

			m_vecRenderSubsetSky[nThreadID].clear();
			m_vecRenderSubsetSkybox[nThreadID].clear();
			m_vecRenderSubsetSkyEffect[nThreadID].clear();
			m_vecRenderSubsetSkyCloud[nThreadID].clear();
		}

		void SkyRenderer::ClearEffect(IDeviceContext* pd3dDeviceContext, IEffectTech* pEffectTech)
		{
			m_pEffect->SetTexture(StrID::g_texEffect, nullptr);
			m_pEffect->SetTexture(StrID::g_texCloud, nullptr);
			m_pEffect->SetTexture(StrID::g_texCloudBlend, nullptr);
			m_pEffect->UndoSamplerState(StrID::g_samLinearWrap, 0);

			m_pEffect->ClearState(pd3dDeviceContext, pEffectTech);
		}
	}
}