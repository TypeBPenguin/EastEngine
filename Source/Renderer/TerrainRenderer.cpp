#include "stdafx.h"
#include "TerrainRenderer.h"

#include "CommonLib/FileUtil.h"
#include "CommonLib/Config.h"

#include "DirectX/CameraManager.h"

namespace StrID
{
	RegisterStringID(EffectTerrain);
	RegisterStringID(RenderHeightfield);

	RegisterStringID(g_texHeightField);
	RegisterStringID(g_texColor);

	RegisterStringID(g_texDetail);
	RegisterStringID(g_texDetailNormal);

	RegisterStringID(g_UseDynamicLOD);
	RegisterStringID(g_FrustumCullInHS);
	RegisterStringID(g_DynamicTessFactor);
	RegisterStringID(g_StaticTessFactor);

	RegisterStringID(g_ModelViewProjectionMatrix);
	RegisterStringID(g_matWorld);

	RegisterStringID(g_CameraPosition);
	RegisterStringID(g_CameraDirection);

	RegisterStringID(g_f2PatchSize);
	RegisterStringID(g_f2HeightFieldSize);
}

namespace EastEngine
{
	namespace Graphics
	{
		TerrainRenderer::TerrainRenderer()
			: m_pEffect(nullptr)
		{
		}

		TerrainRenderer::~TerrainRenderer()
		{
			IEffect::Destroy(&m_pEffect);
		}

		bool TerrainRenderer::Init(const Math::Viewport& viewport)
		{
			std::string strPath(File::GetPath(File::EmPath::eFx));

#if defined(DEBUG) || defined(_DEBUG)
			strPath.append("Terrain\\Terrain_D.cso");
#else
			strPath.append("Terrain\\Terrain.cso");
#endif

			m_pEffect = IEffect::Create(StrID::EffectTerrain, strPath.c_str());
			if (m_pEffect == nullptr)
				return false;

			m_pEffect->CreateTechnique(StrID::RenderHeightfield, EmVertexFormat::ePos4);

			return true;
		}

		void TerrainRenderer::Render(uint32_t nRenderGroupFlag)
		{
			if (m_vecTerrain.empty())
				return;

			D3D_PROFILING(TerrainRenderer);

			Camera* pCamera = CameraManager::GetInstance()->GetMainCamera();
			if (pCamera == nullptr)
			{
				PRINT_LOG("Not Exist Main Camera !!");
				return;
			}

			IDevice* pDevice = GetDevice();
			IDeviceContext* pDeviceContext = GetDeviceContext();
			IGBuffers* pGBuffers = GetGBuffers();

			pDeviceContext->ClearState();
			pDeviceContext->SetDefaultViewport();

			pDeviceContext->SetRenderTargets(pGBuffers->GetGBuffers(), EmGBuffer::Count, pDevice->GetMainDepthStencil());

			pDeviceContext->SetDepthStencilState(EmDepthStencilState::eOn);
			pDeviceContext->SetBlendState(EmBlendState::eOff);

			Math::Matrix matViewProj = pCamera->GetViewMatrix() * pCamera->GetProjMatrix();
			m_pEffect->SetVector(StrID::g_CameraPosition, pCamera->GetPosition());
			m_pEffect->SetVector(StrID::g_CameraDirection, pCamera->GetDir());

			if (Config::IsEnable("Wireframe"_s) == true)
			{
				pDeviceContext->SetRasterizerState(EmRasterizerState::eWireFrame);
			}
			else
			{
				pDeviceContext->SetRasterizerState(EmRasterizerState::eCCW);
			}

			for (auto& renderSubset : m_vecTerrain)
			{
				m_pEffect->SetVector(StrID::g_f2PatchSize, renderSubset.f2PatchSize);
				m_pEffect->SetVector(StrID::g_f2HeightFieldSize, renderSubset.f2HeightFieldSize);

				m_pEffect->SetFloat(StrID::g_UseDynamicLOD, renderSubset.isEnableDynamicLOD == true ? 1.f : 0.f);
				m_pEffect->SetFloat(StrID::g_FrustumCullInHS, renderSubset.isEnableFrustumCullInHS == true ? 1.f : 0.f);

				m_pEffect->SetFloat(StrID::g_DynamicTessFactor, renderSubset.fDynamicTessFactor);
				m_pEffect->SetFloat(StrID::g_StaticTessFactor, renderSubset.fStaticTessFactor);

				m_pEffect->SetTexture(StrID::g_texHeightField, renderSubset.pTexHeightField);
				m_pEffect->SetTexture(StrID::g_texColor, renderSubset.pTexColorMap);

				m_pEffect->SetTexture(StrID::g_texDetail, renderSubset.pTexDetailMap);
				m_pEffect->SetTexture(StrID::g_texDetailNormal, renderSubset.pTexDetailNormalMap);

				m_pEffect->SetMatrix(StrID::g_ModelViewProjectionMatrix, renderSubset.matWorld * matViewProj);
				m_pEffect->SetMatrix(StrID::g_matWorld, renderSubset.matWorld);

				IEffectTech* pEffectTech = m_pEffect->GetTechnique(StrID::RenderHeightfield);
				if (pEffectTech == nullptr)
				{
					PRINT_LOG("Not Exist EffectTech !!");
					return;
				}

				if (pDeviceContext->SetInputLayout(pEffectTech->GetLayoutFormat()) == false)
					return;

				pDeviceContext->SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST);
				pEffectTech->PassApply(0, pDeviceContext);

				pDeviceContext->SetVertexBuffers(renderSubset.pVertexBuffer, renderSubset.pVertexBuffer->GetFormatSize(), 0);
				pDeviceContext->Draw(renderSubset.pVertexBuffer->GetVertexNum(), 0);

				ClearEffect(pDeviceContext, pEffectTech);
			}
		}

		void TerrainRenderer::ClearEffect(IDeviceContext* pd3dDeviceContext, IEffectTech* pTech)
		{
			m_pEffect->SetTexture(StrID::g_texHeightField, nullptr);
			m_pEffect->SetTexture(StrID::g_texColor, nullptr);

			m_pEffect->SetTexture(StrID::g_texDetail, nullptr);
			m_pEffect->SetTexture(StrID::g_texDetailNormal, nullptr);

			m_pEffect->ClearState(pd3dDeviceContext, pTech);
		}
	}
}