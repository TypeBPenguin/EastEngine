#include "stdafx.h"
#include "TerrainRenderer.h"

#include "CommonLib/FileUtil.h"
#include "CommonLib/Config.h"

#include "DirectX/Camera.h"

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
		class TerrainRenderer::Impl
		{
		public:
			Impl();
			~Impl();

		public:
			void AddRender(const RenderSubsetTerrain& renderPiece);

			void Render(IDevice* pDevice, IDeviceContext* pDeviceContext, Camera* pCamera, uint32_t nRenderGroupFlag);
			void Flush();

		private:
			bool CreateEffect();
			void ClearEffect(IDeviceContext* pd3dDeviceContext, IEffectTech* pTech);

		private:
			IEffect* m_pEffect{ nullptr };

			std::vector<RenderSubsetTerrain> m_vecTerrain;
		};

		TerrainRenderer::Impl::Impl()
		{
		}

		TerrainRenderer::Impl::~Impl()
		{
			IEffect::Destroy(&m_pEffect);
		}

		void TerrainRenderer::Impl::AddRender(const RenderSubsetTerrain& renderPiece)
		{
			m_vecTerrain.emplace_back(renderPiece);
		}

		void TerrainRenderer::Impl::Render(IDevice* pDevice, IDeviceContext* pDeviceContext, Camera* pCamera, uint32_t nRenderGroupFlag)
		{
			if (m_vecTerrain.empty())
				return;

			PERF_TRACER_EVENT("TerrainRenderer::Render", "");
			D3D_PROFILING(pDeviceContext, TerrainRenderer);

			IGBuffers* pGBuffers = GetGBuffers();

			int nThreadID = GetThreadID(ThreadType::eRender);
			const Math::Matrix& matView = pCamera->GetViewMatrix(nThreadID);
			const Math::Matrix& matProj = pCamera->GetProjMatrix(nThreadID);

			pDeviceContext->ClearState();
			pDeviceContext->SetDefaultViewport();

			pDeviceContext->SetRenderTargets(pGBuffers->GetGBuffers(), EmGBuffer::Count, pDevice->GetMainDepthStencil());

			pDeviceContext->SetDepthStencilState(EmDepthStencilState::eRead_Write_On);
			pDeviceContext->SetBlendState(EmBlendState::eOff);

			Math::Matrix matViewProj = matView * matProj;
			m_pEffect->SetVector(StrID::g_CameraPosition, matView.Invert().Translation());
			m_pEffect->SetVector(StrID::g_CameraDirection, pCamera->GetDir());

			if (Config::IsEnable("Wireframe"_s) == true)
			{
				pDeviceContext->SetRasterizerState(EmRasterizerState::eWireframeCullNone);
			}
			else
			{
				pDeviceContext->SetRasterizerState(EmRasterizerState::eSolidCCW);
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
					LOG_ERROR("Not Exist EffectTech !!");
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

		void TerrainRenderer::Impl::Flush()
		{
			m_vecTerrain.clear();
		}

		bool TerrainRenderer::Impl::CreateEffect()
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

		void TerrainRenderer::Impl::ClearEffect(IDeviceContext* pd3dDeviceContext, IEffectTech* pTech)
		{
			m_pEffect->SetTexture(StrID::g_texHeightField, nullptr);
			m_pEffect->SetTexture(StrID::g_texColor, nullptr);

			m_pEffect->SetTexture(StrID::g_texDetail, nullptr);
			m_pEffect->SetTexture(StrID::g_texDetailNormal, nullptr);

			m_pEffect->ClearState(pd3dDeviceContext, pTech);
		}

		TerrainRenderer::TerrainRenderer()
			: m_pImpl{ std::make_unique<Impl>() }
		{
		}

		TerrainRenderer::~TerrainRenderer()
		{
		}

		void TerrainRenderer::AddRender(const RenderSubsetTerrain& renderPiece)
		{
			m_pImpl->AddRender(renderPiece);
		}

		void TerrainRenderer::Render(IDevice* pDevice, IDeviceContext* pDeviceContext, Camera* pCamera, uint32_t nRenderGroupFlag)
		{
			m_pImpl->Render(pDevice, pDeviceContext, pCamera, nRenderGroupFlag);
		}

		void TerrainRenderer::Flush()
		{
			m_pImpl->Flush();
		}
	}
}