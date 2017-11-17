#include "stdafx.h"
#include "VertexRenderer.h"

#include "../CommonLib/FileUtil.h"
#include "../CommonLib/Config.h"

#include "../DirectX/CameraManager.h"

namespace StrID
{
	RegisterStringID(EffectVertex);

	RegisterStringID(Color);
	RegisterStringID(Vertex);

	RegisterStringID(g_matWVP);
}

namespace EastEngine
{
	namespace Graphics
	{
		VertexRenderer::VertexRenderer()
			: m_pEffect(nullptr)
		{
		}

		VertexRenderer::~VertexRenderer()
		{
			m_vecVertexSubset.clear();
			m_vecLineSubset.clear();
			m_vecLineSegmentSubset.clear();

			SafeDelete(m_pLineSegmentVertexBuffer);
			SafeDelete(m_pLineSegmentIndexBuffer);

			IEffect::Destroy(&m_pEffect);
		}

		bool VertexRenderer::Init(const Math::Viewport& viewport)
		{
			std::string strPath(File::GetPath(File::EmPath::eFx));

#if defined(DEBUG) || defined(_DEBUG)
			strPath.append("Vertex\\Vertex_D.cso");
#else
			strPath.append("Vertex\\Vertex.cso");
#endif

			m_pEffect = IEffect::Create(StrID::EffectVertex, strPath.c_str());
			if (m_pEffect == nullptr)
				return false;

			m_pEffect->CreateTechnique(StrID::Vertex, EmVertexFormat::ePos);
			m_pEffect->CreateTechnique(StrID::Color, EmVertexFormat::ePosCol);

			{
				uint32_t nVertexCount = 2;
				uint32_t nIndexCount = nVertexCount;

				Math::Color color = Math::Color::DarkRed;
				
				std::vector<VertexPosCol> vecVertices;
				vecVertices.reserve(nVertexCount);

				std::vector<uint32_t> vecIndices;
				vecIndices.reserve(nIndexCount);

				uint32_t nIdx = 0;
				vecVertices.push_back(VertexPosCol(Math::Vector3::Zero, color));
				vecIndices.push_back(nIdx);
				++nIdx;

				vecVertices.push_back(VertexPosCol(Math::Vector3(0.f, 0.f, 1.f), color));
				vecIndices.push_back(nIdx);
				++nIdx;

				m_pLineSegmentVertexBuffer = IVertexBuffer::Create(VertexPosCol::Format(), vecVertices.size(), &vecVertices.front(), D3D11_USAGE_IMMUTABLE);
				m_pLineSegmentIndexBuffer = IIndexBuffer::Create(vecIndices.size(), &vecIndices.front(), D3D11_USAGE_IMMUTABLE);

				if (m_pLineSegmentVertexBuffer == nullptr || m_pLineSegmentIndexBuffer == nullptr)
					return false;
			}

			return true;
		}

		void VertexRenderer::Render(uint32_t nRenderGroupFlag)
		{
			D3D_PROFILING(VertexRenderer);

			IDevice* pDevice = GetDevice();

			renderVertex(pDevice);
			renderLine(pDevice);
			renderLineSegment(pDevice);
		}

		void VertexRenderer::Flush()
		{
			m_vecVertexSubset.clear();
			m_vecLineSubset.clear();
			m_vecLineSegmentSubset.clear();
		}

		void VertexRenderer::ClearEffect(IDeviceContext* pd3dDeviceContext, IEffectTech* pTech)
		{
			m_pEffect->ClearState(pd3dDeviceContext, pTech);
		}

		void VertexRenderer::renderVertex(IDevice* pDevice)
		{
			D3D_PROFILING(Vertex);

			if (m_vecVertexSubset.empty())
				return;

			IEffectTech* pEffectTech = m_pEffect->GetTechnique(StrID::Vertex);
			if (pEffectTech == nullptr)
			{
				PRINT_LOG("Not Exist EffectTech !!");
				return;
			}

			Camera* pCamera = CameraManager::GetInstance()->GetMainCamera();
			if (pCamera == nullptr)
			{
				PRINT_LOG("Not Exist Main Camera !!");
				return;
			}

			IDeviceContext* pDeviceContext = pDevice->GetImmediateContext();
			if (pDeviceContext->SetInputLayout(pEffectTech->GetLayoutFormat()) == false)
				return;

			pDeviceContext->ClearState();

			pDeviceContext->SetDefaultViewport();

			pDeviceContext->SetBlendState(EmBlendState::eOff);
			pDeviceContext->SetRasterizerState(EmRasterizerState::eCCW);
			pDeviceContext->SetDepthStencilState(EmDepthStencilState::eOn);

			auto desc = pDevice->GetMainRenderTarget()->GetDesc2D();
			desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
			desc.Build();

			IRenderTarget* pRenderTarget = pDevice->GetRenderTarget(desc);
			pDeviceContext->SetRenderTargets(&pRenderTarget, 1, pDevice->GetMainDepthStencil());
			pDeviceContext->SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			uint32_t nSize = m_vecVertexSubset.size();
			for (uint32_t i = 0; i < nSize; ++i)
			{
				RenderSubsetVertex& renderSubset = m_vecVertexSubset[i];

				pDeviceContext->SetVertexBuffers(renderSubset.pVertexBuffer, renderSubset.pVertexBuffer->GetFormatSize(), 0);
				pDeviceContext->SetIndexBuffer(renderSubset.pIndexBuffer, 0);

				Math::Matrix matWorld = renderSubset.pWorldMatrix != nullptr ? *renderSubset.pWorldMatrix : Math::Matrix::Identity;
				m_pEffect->SetMatrix(StrID::g_matWVP, matWorld * pCamera->GetViewMatrix() * pCamera->GetProjMatrix());

				uint32_t nPassCount = pEffectTech->GetPassCount();
				for (uint32_t p = 0; p < nPassCount; ++p)
				{
					pEffectTech->PassApply(p, pDeviceContext);

					pDeviceContext->DrawIndexed(renderSubset.pIndexBuffer->GetIndexNum(), 0, 0);
				}
			}

			ClearEffect(pDeviceContext, pEffectTech);

			pDevice->ReleaseRenderTargets(&pRenderTarget);
		}

		void VertexRenderer::renderLine(IDevice* pDevice)
		{
			D3D_PROFILING(Line);

			if (m_vecLineSubset.empty())
				return;

			IEffectTech* pEffectTech = m_pEffect->GetTechnique(StrID::Color);
			if (pEffectTech == nullptr)
			{
				PRINT_LOG("Not Exist EffectTech !!");
				return;
			}

			Camera* pCamera = CameraManager::GetInstance()->GetMainCamera();
			if (pCamera == nullptr)
			{
				PRINT_LOG("Not Exist Main Camera !!");
				return;
			}

			IDeviceContext* pDeviceContext = pDevice->GetImmediateContext();
			if (pDeviceContext->SetInputLayout(pEffectTech->GetLayoutFormat()) == false)
				return;

			pDeviceContext->ClearState();

			pDeviceContext->SetDefaultViewport();

			pDeviceContext->SetBlendState(EmBlendState::eOff);
			pDeviceContext->SetRasterizerState(EmRasterizerState::eCCW);
			pDeviceContext->SetDepthStencilState(EmDepthStencilState::eOn);

			IRenderTarget* pRenderTarget = nullptr;
			if (Config::IsEnableHDRFilter() == true)
			{
				auto desc = pDevice->GetMainRenderTarget()->GetDesc2D();
				if (Config::IsEnableHDRFilter() == true)
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

			pDeviceContext->SetRenderTargets(&pRenderTarget, 1, pDevice->GetMainDepthStencil());
			pDeviceContext->SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

			uint32_t nSize = m_vecLineSubset.size();
			for (uint32_t i = 0; i < nSize; ++i)
			{
				RenderSubsetLine& renderSubset = m_vecLineSubset[i];

				pDeviceContext->SetVertexBuffers(renderSubset.pVertexBuffer, renderSubset.pVertexBuffer->GetFormatSize(), 0);
				pDeviceContext->SetIndexBuffer(renderSubset.pIndexBuffer, 0);

				m_pEffect->SetMatrix(StrID::g_matWVP, renderSubset.matWorld * pCamera->GetViewMatrix() * pCamera->GetProjMatrix());

				uint32_t nPassCount = pEffectTech->GetPassCount();
				for (uint32_t p = 0; p < nPassCount; ++p)
				{
					pEffectTech->PassApply(p, pDeviceContext);

					pDeviceContext->DrawIndexed(renderSubset.pIndexBuffer->GetIndexNum(), 0, 0);
				}
			}

			ClearEffect(pDeviceContext, pEffectTech);

			pDevice->ReleaseRenderTargets(&pRenderTarget);
		}

		void VertexRenderer::renderLineSegment(IDevice* pDevice)
		{
			D3D_PROFILING(LineSegment);

			if (m_vecLineSegmentSubset.empty())
				return;

			IEffectTech* pEffectTech = m_pEffect->GetTechnique(StrID::Color);
			if (pEffectTech == nullptr)
			{
				PRINT_LOG("Not Exist EffectTech !!");
				return;
			}

			Camera* pCamera = CameraManager::GetInstance()->GetMainCamera();
			if (pCamera == nullptr)
			{
				PRINT_LOG("Not Exist Main Camera !!");
				return;
			}

			IDeviceContext* pDeviceContext = pDevice->GetImmediateContext();
			if (pDeviceContext->SetInputLayout(pEffectTech->GetLayoutFormat()) == false)
				return;

			pDeviceContext->ClearState();

			pDeviceContext->SetDefaultViewport();

			pDeviceContext->SetBlendState(EmBlendState::eOff);
			pDeviceContext->SetRasterizerState(EmRasterizerState::eCCW);
			pDeviceContext->SetDepthStencilState(EmDepthStencilState::eOn);

			auto desc = pDevice->GetMainRenderTarget()->GetDesc2D();
			desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
			desc.Build();

			IRenderTarget* pRenderTarget = pDevice->GetRenderTarget(desc);
			pDeviceContext->SetRenderTargets(&pRenderTarget, 1, pDevice->GetMainDepthStencil());
			pDeviceContext->SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

			uint32_t nSize = m_vecLineSegmentSubset.size();
			for (uint32_t i = 0; i < nSize; ++i)
			{
				RenderSubsetLineSegment& renderSubset = m_vecLineSegmentSubset[i];

				VertexPosCol* pVertices = nullptr;
				if (m_pLineSegmentVertexBuffer->Map(0, D3D11_MAP_WRITE_NO_OVERWRITE, reinterpret_cast<void**>(&pVertices)) == false)
					continue;

				pVertices[0] = renderSubset.vertexLineSegment[0];
				pVertices[1] = renderSubset.vertexLineSegment[1];

				m_pLineSegmentVertexBuffer->Unmap(0);

				pDeviceContext->SetVertexBuffers(m_pLineSegmentVertexBuffer, m_pLineSegmentVertexBuffer->GetFormatSize(), 0);
				pDeviceContext->SetIndexBuffer(m_pLineSegmentIndexBuffer, 0);

				m_pEffect->SetMatrix(StrID::g_matWVP, pCamera->GetViewMatrix() * pCamera->GetProjMatrix());

				uint32_t nPassCount = pEffectTech->GetPassCount();
				for (uint32_t p = 0; p < nPassCount; ++p)
				{
					pEffectTech->PassApply(p, pDeviceContext);

					pDeviceContext->DrawIndexed(m_pLineSegmentIndexBuffer->GetIndexNum(), 0, 0);
				}
			}

			ClearEffect(pDeviceContext, pEffectTech);

			pDevice->ReleaseRenderTargets(&pRenderTarget);
		}
	}
}