#include "stdafx.h"
#include "TerrainRenderer.h"

#include "CommonLib/FileUtil.h"
#include "CommonLib/Config.h"

#include "DirectX/CameraManager.h"

namespace StrID
{
	RegisterStringID(EffectTerrain);
	RegisterStringID(RenderHeightfield);
	
	RegisterStringID(g_RenderCaustics);
	RegisterStringID(g_UseDynamicLOD);
	RegisterStringID(g_FrustumCullInHS);
	RegisterStringID(g_DynamicTessFactor);
	RegisterStringID(g_StaticTessFactor);
	RegisterStringID(g_TerrainBeingRendered);
	RegisterStringID(g_HalfSpaceCullSign);
	RegisterStringID(g_HalfSpaceCullPosition);
	RegisterStringID(g_SkipCausticsCalculation);
	RegisterStringID(g_matView);
	RegisterStringID(g_ModelViewProjectionMatrix);
	RegisterStringID(g_LightModelViewProjectionMatrix);
	RegisterStringID(g_CameraPosition);
	RegisterStringID(g_CameraDirection);
	RegisterStringID(g_LightPosition);
	RegisterStringID(g_HeightFieldSize);
	RegisterStringID(g_HeightfieldTexture);
	RegisterStringID(g_LayerdefTexture);
	RegisterStringID(g_RockBumpTexture);
	RegisterStringID(g_RockMicroBumpTexture);
	RegisterStringID(g_RockDiffuseTexture);
	RegisterStringID(g_SandBumpTexture);
	RegisterStringID(g_SandMicroBumpTexture);
	RegisterStringID(g_SandDiffuseTexture);
	RegisterStringID(g_GrassDiffuseTexture);
	RegisterStringID(g_SlopeDiffuseTexture);
	RegisterStringID(g_WaterBumpTexture);
	RegisterStringID(g_DepthTexture);
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
			pDeviceContext->SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

			Math::Matrix matViewProj = pCamera->GetViewMatrix() * pCamera->GetProjMatrix();
			m_pEffect->SetVector(StrID::g_CameraPosition, pCamera->GetPosition());
			m_pEffect->SetVector(StrID::g_CameraDirection, pCamera->GetDir());

			m_pEffect->SetFloat(StrID::g_RenderCaustics, 1.f);
			m_pEffect->SetFloat(StrID::g_UseDynamicLOD, 1.f);
			m_pEffect->SetFloat(StrID::g_FrustumCullInHS, 0.f);
			m_pEffect->SetFloat(StrID::g_DynamicTessFactor, 50.f);
			m_pEffect->SetFloat(StrID::g_StaticTessFactor, 12.f);
			m_pEffect->SetFloat(StrID::g_TerrainBeingRendered, 1.f);
			m_pEffect->SetFloat(StrID::g_SkipCausticsCalculation, 0.f);

			if (Config::IsEnableWireframe() == true)
			{
				pDeviceContext->SetRasterizerState(EmRasterizerState::eWireFrame);
			}
			else
			{
				pDeviceContext->SetRasterizerState(EmRasterizerState::eCCW);
			}

			for (auto& renderSubset : m_vecTerrain)
			{
				m_pEffect->SetTexture(StrID::g_HeightfieldTexture, renderSubset.pTexHeightField);
				m_pEffect->SetTexture(StrID::g_LayerdefTexture, renderSubset.pTexLayerdef);
				m_pEffect->SetTexture(StrID::g_RockBumpTexture, renderSubset.pTexRockBump);
				m_pEffect->SetTexture(StrID::g_RockMicroBumpTexture, renderSubset.pTexRockMicroBump);
				m_pEffect->SetTexture(StrID::g_RockDiffuseTexture, renderSubset.pTexRockDiffuse);
				m_pEffect->SetTexture(StrID::g_SandBumpTexture, renderSubset.pTexSandBump);
				m_pEffect->SetTexture(StrID::g_SandMicroBumpTexture, renderSubset.pTexSandMicroBump);
				m_pEffect->SetTexture(StrID::g_SandDiffuseTexture, renderSubset.pTexSandDiffuse);
				m_pEffect->SetTexture(StrID::g_GrassDiffuseTexture, renderSubset.pTexGrassDiffuse);
				m_pEffect->SetTexture(StrID::g_SlopeDiffuseTexture, renderSubset.pTexSlopeDiffuse);

				m_pEffect->SetFloat(StrID::g_HeightFieldSize, renderSubset.fHeightFieldSize);
				m_pEffect->SetFloat(StrID::g_HalfSpaceCullSign, renderSubset.fHalfSpaceCullSign);
				m_pEffect->SetFloat(StrID::g_HalfSpaceCullPosition, renderSubset.fHalfSpaceCullPosition);

				m_pEffect->SetMatrix(StrID::g_matView, renderSubset.matWorld * pCamera->GetViewMatrix());
				m_pEffect->SetMatrix(StrID::g_ModelViewProjectionMatrix, renderSubset.matWorld * matViewProj);

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
			m_pEffect->SetTexture(StrID::g_HeightfieldTexture, nullptr);
			m_pEffect->SetTexture(StrID::g_LayerdefTexture, nullptr);
			m_pEffect->SetTexture(StrID::g_RockBumpTexture, nullptr);
			m_pEffect->SetTexture(StrID::g_RockMicroBumpTexture, nullptr);
			m_pEffect->SetTexture(StrID::g_RockDiffuseTexture, nullptr);
			m_pEffect->SetTexture(StrID::g_SandBumpTexture, nullptr);
			m_pEffect->SetTexture(StrID::g_SandMicroBumpTexture, nullptr);
			m_pEffect->SetTexture(StrID::g_SandDiffuseTexture, nullptr);
			m_pEffect->SetTexture(StrID::g_GrassDiffuseTexture, nullptr);
			m_pEffect->SetTexture(StrID::g_SlopeDiffuseTexture, nullptr);
			m_pEffect->SetTexture(StrID::g_DepthTexture, nullptr);

			m_pEffect->ClearState(pd3dDeviceContext, pTech);
		}
	}
}