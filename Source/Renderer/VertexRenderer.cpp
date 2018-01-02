#include "stdafx.h"
#include "VertexRenderer.h"

#include "CommonLib/FileUtil.h"
#include "CommonLib/Config.h"

#include "DirectX/CameraManager.h"

namespace StrID
{
	RegisterStringID(EffectVertex);

	RegisterStringID(Color);
	RegisterStringID(Vertex);
	RegisterStringID(VertexInstancing);

	RegisterStringID(g_Instances);
	RegisterStringID(g_matWVP);
	RegisterStringID(g_matViewProjection);
	RegisterStringID(g_color);
}

namespace EastEngine
{
	namespace Graphics
	{
		VertexRenderer::VertexRenderer()
			: m_pEffect(nullptr)
			, m_pLineSegmentVertexBuffer(nullptr)
		{
		}

		VertexRenderer::~VertexRenderer()
		{
			m_vecVertexSubset.clear();
			m_vecLineSubset.clear();
			m_vecLineSegmentSubset.clear();

			SafeDelete(m_pLineSegmentVertexBuffer);

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
			m_pEffect->CreateTechnique(StrID::VertexInstancing, EmVertexFormat::ePos);
			m_pEffect->CreateTechnique(StrID::Color, EmVertexFormat::ePosCol);

			{
				uint32_t nVertexCount = 2;
				Math::Color color = Math::Color::DarkRed;
				
				std::vector<VertexPosCol> vecVertices;
				vecVertices.reserve(nVertexCount);

				vecVertices.push_back(VertexPosCol(Math::Vector3::Zero, color));
				vecVertices.push_back(VertexPosCol(Math::Vector3(0.f, 0.f, 1.f), color));

				m_pLineSegmentVertexBuffer = IVertexBuffer::Create(VertexPosCol::Format(), vecVertices.size(), &vecVertices.front(), D3D11_USAGE_DYNAMIC);

				if (m_pLineSegmentVertexBuffer == nullptr)
					return false;
			}

			return true;
		}

		void VertexRenderer::Render(uint32_t nRenderGroupFlag)
		{
			D3D_PROFILING(VertexRenderer);

			IDevice* pDevice = GetDevice();
			IDeviceContext* pDeviceContext = pDevice->GetImmediateContext();

			pDeviceContext->ClearState();

			pDeviceContext->SetDefaultViewport();

			pDeviceContext->SetBlendState(EmBlendState::eOff);
			pDeviceContext->SetDepthStencilState(EmDepthStencilState::eOn);

			auto desc = pDevice->GetMainRenderTarget()->GetDesc2D();
			if (Config::IsEnable("HDRFilter"_s) == true)
			{
				desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
				desc.Build();
			}

			IRenderTarget* pRenderTarget = pDevice->GetRenderTarget(desc);
			pDeviceContext->SetRenderTargets(&pRenderTarget, 1, pDevice->GetMainDepthStencil());

			RenderVertex(pDevice, pDeviceContext);

			pDeviceContext->SetDepthStencilState(EmDepthStencilState::eOn);

			RenderLine(pDevice, pDeviceContext);
			RenderLineSegment(pDevice, pDeviceContext);

			pDevice->ReleaseRenderTargets(&pRenderTarget);
		}

		void VertexRenderer::Flush()
		{
			m_vecVertexSubset.clear();
			m_vecLineSubset.clear();
			m_vecLineSegmentSubset.clear();
		}

		void VertexRenderer::SetRenderState(IDevice* pDevice, IDeviceContext* pDeviceContext)
		{
		}

		void VertexRenderer::ClearEffect(IDeviceContext* pDeviceContext, IEffectTech* pTech)
		{
			m_pEffect->ClearState(pDeviceContext, pTech);
		}

		void VertexRenderer::RenderVertex(IDevice* pDevice, IDeviceContext* pDeviceContext)
		{
			D3D_PROFILING(Vertex);

			if (m_vecVertexSubset.empty())
				return;

			Camera* pCamera = CameraManager::GetInstance()->GetMainCamera();
			if (pCamera == nullptr)
			{
				LOG_ERROR("Not Exist Main Camera !!");
				return;
			}

			IEffectTech* pEffectTech = nullptr;

			pDeviceContext->SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			const Math::Matrix matViewProjection = pCamera->GetViewMatrix() * pCamera->GetProjMatrix();

			std::map<std::tuple<IVertexBuffer*, IIndexBuffer*, bool>, RenderSubsetVertexBatch> mapVertex;
			{
				const size_t nSize = m_vecVertexSubset.size();
				for (size_t i = 0; i < nSize; ++i)
				{
					RenderSubsetVertex& renderSubset = m_vecVertexSubset[i];

					std::tuple<IVertexBuffer*, IIndexBuffer*, bool> key = std::make_tuple(renderSubset.pVertexBuffer, renderSubset.pIndexBuffer, renderSubset.isWireframe);
					auto iter = mapVertex.find(key);
					if (iter != mapVertex.end())
					{
						iter->second.vecInstData.emplace_back(renderSubset.matWorld, renderSubset.color);
					}
					else
					{
						mapVertex.emplace(key, RenderSubsetVertexBatch(&renderSubset, renderSubset.matWorld, renderSubset.color));
					}
				}
			}

			{
				for (auto& iter : mapVertex)
				{
					RenderSubsetVertexBatch& renderSubsetBatch = iter.second;
					const RenderSubsetVertex* pRenderSubset = renderSubsetBatch.pSubset;

					if (pRenderSubset->isWireframe == true)
					{
						pDeviceContext->SetRasterizerState(EmRasterizerState::eWireframeCullNone);
					}
					else
					{
						pDeviceContext->SetRasterizerState(EmRasterizerState::eSolidCCW);
					}

					if (pRenderSubset->isIgnoreDepth == true)
					{
						pDeviceContext->SetDepthStencilState(EmDepthStencilState::eOff);
					}
					else
					{
						pDeviceContext->SetDepthStencilState(EmDepthStencilState::eOn);

					}

					pDeviceContext->SetVertexBuffers(pRenderSubset->pVertexBuffer, pRenderSubset->pVertexBuffer->GetFormatSize(), 0);
					pDeviceContext->SetIndexBuffer(pRenderSubset->pIndexBuffer, 0);

					if (renderSubsetBatch.vecInstData.size() == 1)
					{
						pEffectTech = m_pEffect->GetTechnique(StrID::Vertex);
						if (pEffectTech == nullptr)
						{
							LOG_ERROR("Not Exist EffectTech !!");
							continue;
						}

						if (pDeviceContext->SetInputLayout(pEffectTech->GetLayoutFormat()) == false)
							continue;

						m_pEffect->SetMatrix(StrID::g_matWVP, pRenderSubset->matWorld * pCamera->GetViewMatrix() * pCamera->GetProjMatrix());
						m_pEffect->SetVector(StrID::g_color, *reinterpret_cast<const Math::Vector4*>(&pRenderSubset->color));

						uint32_t nPassCount = pEffectTech->GetPassCount();
						for (uint32_t p = 0; p < nPassCount; ++p)
						{
							pEffectTech->PassApply(p, pDeviceContext);

							if (pRenderSubset->pIndexBuffer != nullptr)
							{
								pDeviceContext->DrawIndexed(pRenderSubset->pIndexBuffer->GetIndexNum(), 0, 0);
							}
							else
							{
								pDeviceContext->Draw(pRenderSubset->pVertexBuffer->GetVertexNum(), 0);
							}
						}
					}
					else
					{
						pEffectTech = m_pEffect->GetTechnique(StrID::VertexInstancing);
						if (pEffectTech == nullptr)
						{
							LOG_ERROR("Not Exist EffectTech !!");
							continue;
						}

						if (pDeviceContext->SetInputLayout(pEffectTech->GetLayoutFormat()) == false)
							continue;

						m_pEffect->SetMatrix(StrID::g_matViewProjection, matViewProjection);

						const std::vector<RenderSubsetVertexBatch::InstVertexData>& vecInstData = renderSubsetBatch.vecInstData;
						size_t nInstanceSize = vecInstData.size();
						size_t nLoopCount = nInstanceSize / MAX_INSTANCE_NUM + 1;
						for (size_t j = 0; j < nLoopCount; ++j)
						{
							int nMax = std::min(MAX_INSTANCE_NUM * (j + 1), nInstanceSize);
							int nNum = nMax - j * MAX_INSTANCE_NUM;

							if (nNum <= 0)
								break;

							m_pEffect->SetRawValue(StrID::g_Instances, &vecInstData[j * MAX_INSTANCE_NUM], 0, nNum * sizeof(RenderSubsetVertexBatch::InstVertexData));

							uint32_t nPassCount = pEffectTech->GetPassCount();
							for (uint32_t p = 0; p < nPassCount; ++p)
							{
								pEffectTech->PassApply(p, pDeviceContext);

								if (pRenderSubset->pIndexBuffer != nullptr)
								{
									pDeviceContext->DrawIndexedInstanced(pRenderSubset->pIndexBuffer->GetIndexNum(), nNum, 0, 0, 0);
								}
								else
								{
									pDeviceContext->DrawInstanced(pRenderSubset->pVertexBuffer->GetVertexNum(), nNum, 0, 0);
								}
							}
						}
					}
				}
			}

			ClearEffect(pDeviceContext, pEffectTech);
		}

		void VertexRenderer::RenderLine(IDevice* pDevice, IDeviceContext* pDeviceContext)
		{
			D3D_PROFILING(Line);

			if (m_vecLineSubset.empty())
				return;

			IEffectTech* pEffectTech = m_pEffect->GetTechnique(StrID::Color);
			if (pEffectTech == nullptr)
			{
				LOG_ERROR("Not Exist EffectTech !!");
				return;
			}

			Camera* pCamera = CameraManager::GetInstance()->GetMainCamera();
			if (pCamera == nullptr)
			{
				LOG_ERROR("Not Exist Main Camera !!");
				return;
			}

			if (pDeviceContext->SetInputLayout(pEffectTech->GetLayoutFormat()) == false)
				return;

			pDeviceContext->SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

			const size_t nSize = m_vecLineSubset.size();
			for (size_t i= 0; i < nSize; ++i)
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
		}

		void VertexRenderer::RenderLineSegment(IDevice* pDevice, IDeviceContext* pDeviceContext)
		{
			D3D_PROFILING(LineSegment);

			if (m_vecLineSegmentSubset.empty())
				return;

			IEffectTech* pEffectTech = m_pEffect->GetTechnique(StrID::Color);
			if (pEffectTech == nullptr)
			{
				LOG_ERROR("Not Exist EffectTech !!");
				return;
			}

			Camera* pCamera = CameraManager::GetInstance()->GetMainCamera();
			if (pCamera == nullptr)
			{
				LOG_ERROR("Not Exist Main Camera !!");
				return;
			}

			if (pDeviceContext->SetInputLayout(pEffectTech->GetLayoutFormat()) == false)
				return;

			pDeviceContext->SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

			const size_t nSize = m_vecLineSegmentSubset.size();
			for (size_t i= 0; i < nSize; ++i)
			{
				RenderSubsetLineSegment& renderSubset = m_vecLineSegmentSubset[i];

				VertexPosCol* pVertices = nullptr;
				if (m_pLineSegmentVertexBuffer->Map(0, D3D11_MAP_WRITE_DISCARD, reinterpret_cast<void**>(&pVertices)) == false)
					continue;

				pVertices[0] = renderSubset.vertexLineSegment[0];
				pVertices[1] = renderSubset.vertexLineSegment[1];

				m_pLineSegmentVertexBuffer->Unmap(0);

				if (renderSubset.isIgnoreDepth == true)
				{
					pDeviceContext->SetDepthStencilState(EmDepthStencilState::eOff);
				}
				else
				{
					pDeviceContext->SetDepthStencilState(EmDepthStencilState::eOn);
				}

				pDeviceContext->SetVertexBuffers(m_pLineSegmentVertexBuffer, m_pLineSegmentVertexBuffer->GetFormatSize(), 0);
				pDeviceContext->SetIndexBuffer(nullptr, 0);

				m_pEffect->SetMatrix(StrID::g_matWVP, pCamera->GetViewMatrix() * pCamera->GetProjMatrix());

				uint32_t nPassCount = pEffectTech->GetPassCount();
				for (uint32_t p = 0; p < nPassCount; ++p)
				{
					pEffectTech->PassApply(p, pDeviceContext);

					pDeviceContext->Draw(2, 0);
				}
			}

			ClearEffect(pDeviceContext, pEffectTech);
		}
	}
}