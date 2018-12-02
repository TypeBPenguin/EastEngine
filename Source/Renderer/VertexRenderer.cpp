#include "stdafx.h"
#include "VertexRenderer.h"

#include "CommonLib/FileUtil.h"
#include "CommonLib/Config.h"

#include "DirectX/Camera.h"

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

namespace eastengine
{
	namespace graphics
	{
		class VertexRenderer::Impl
		{
		public:
			Impl();
			~Impl();

		public:
			void AddRender(const RenderSubsetVertex& renderSubset);
			void AddRender(const RenderSubsetLine& renderSubset);
			void AddRender(const RenderSubsetLineSegment& renderSubset);

		public:
			void Render(IDevice* pDevice, IDeviceContext* pDeviceContext, Camera* pCamera, uint32_t nRenderGroupFlag);
			void Flush();

		private:
			bool CreateEffect();
			void ClearEffect(IDeviceContext* pDeviceContext, IEffectTech* pTech);

			void RenderVertex(IDevice* pDevice, IDeviceContext* pDeviceContext, Camera* pCamera);
			void RenderLine(IDevice* pDevice, IDeviceContext* pDeviceContext, Camera* pCamera);
			void RenderLineSegment(IDevice* pDevice, IDeviceContext* pDeviceContext, Camera* pCamera);

		private:
			IEffect* m_pEffect{ nullptr };

			std::array<std::vector<RenderSubsetVertex>, ThreadCount> m_vecVertexSubset;
			std::array<std::vector<RenderSubsetLine>, ThreadCount> m_vecLineSubset;
			std::array<std::vector<RenderSubsetLineSegment>, ThreadCount> m_vecLineSegmentSubset;

			IVertexBuffer* m_pLineSegmentVertexBuffer{ nullptr };

			struct RenderSubsetVertexBatch
			{
				struct InstVertexData
				{
					InstStaticData worldData;
					math::Color colorData;

					InstVertexData(const math::Matrix& matWorld, const math::Color& color)
						: worldData(matWorld)
						, colorData(color)
					{
					}
				};

				const RenderSubsetVertex* pSubset = nullptr;
				std::vector<InstVertexData> vecInstData;

				RenderSubsetVertexBatch(const RenderSubsetVertex* pSubset, const math::Matrix& matWorld, const math::Color& color)
					: pSubset(pSubset)
				{
					vecInstData.emplace_back(matWorld, color);
				}
			};
		};

		VertexRenderer::Impl::Impl()
		{
			if (CreateEffect() == false)
			{
				assert(false);
			}
		}

		VertexRenderer::Impl::~Impl()
		{
			SafeDelete(m_pLineSegmentVertexBuffer);

			IEffect::Destroy(&m_pEffect);
		}

		void VertexRenderer::Impl::AddRender(const RenderSubsetVertex& renderSubset)
		{
			m_vecVertexSubset[GetThreadID(ThreadType::eUpdate)].emplace_back(renderSubset);
		}

		void VertexRenderer::Impl::AddRender(const RenderSubsetLine& renderSubset)
		{
			m_vecLineSubset[GetThreadID(ThreadType::eUpdate)].emplace_back(renderSubset);
		}

		void VertexRenderer::Impl::AddRender(const RenderSubsetLineSegment& renderSubset)
		{
			m_vecLineSegmentSubset[GetThreadID(ThreadType::eUpdate)].emplace_back(renderSubset);
		}

		void VertexRenderer::Impl::Render(IDevice* pDevice, IDeviceContext* pDeviceContext, Camera* pCamera, uint32_t nRenderGroupFlag)
		{
			TRACER_EVENT("VertexRenderer::Render");
			D3D_PROFILING(pDeviceContext, VertexRenderer);

			pDeviceContext->ClearState();

			pDeviceContext->SetDefaultViewport();

			pDeviceContext->SetBlendState(EmBlendState::eOff);
			pDeviceContext->SetDepthStencilState(EmDepthStencilState::eRead_Write_On);

			auto desc = pDevice->GetMainRenderTarget()->GetDesc2D();
			if (Config::IsEnable("HDRFilter"_s) == true)
			{
				desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
				desc.Build();
			}

			IRenderTarget* pRenderTarget = pDevice->GetRenderTarget(desc);
			pDeviceContext->SetRenderTargets(&pRenderTarget, 1, pDevice->GetMainDepthStencil());

			RenderVertex(pDevice, pDeviceContext, pCamera);

			pDeviceContext->SetDepthStencilState(EmDepthStencilState::eRead_Write_On);

			RenderLine(pDevice, pDeviceContext, pCamera);
			RenderLineSegment(pDevice, pDeviceContext, pCamera);

			pDevice->ReleaseRenderTargets(&pRenderTarget);
		}

		void VertexRenderer::Impl::Flush()
		{
			int nThreadID = GetThreadID(ThreadType::eRender);
			m_vecVertexSubset[nThreadID].clear();
			m_vecLineSubset[nThreadID].clear();
			m_vecLineSegmentSubset[nThreadID].clear();
		}

		bool VertexRenderer::Impl::CreateEffect()
		{
			std::string strPath(file::GetPath(file::EmPath::eFx));

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
				math::Color color = math::Color::DarkRed;

				std::vector<VertexPosCol> vecVertices;
				vecVertices.reserve(nVertexCount);

				vecVertices.push_back(VertexPosCol(math::float3::Zero, color));
				vecVertices.push_back(VertexPosCol(math::float3(0.f, 0.f, 1.f), color));

				m_pLineSegmentVertexBuffer = IVertexBuffer::Create(VertexPosCol::Format(), vecVertices.size(), &vecVertices.front(), D3D11_USAGE_DYNAMIC);

				if (m_pLineSegmentVertexBuffer == nullptr)
					return false;
			}

			return true;
		}

		void VertexRenderer::Impl::ClearEffect(IDeviceContext* pDeviceContext, IEffectTech* pTech)
		{
			m_pEffect->ClearState(pDeviceContext, pTech);
		}

		void VertexRenderer::Impl::RenderVertex(IDevice* pDevice, IDeviceContext* pDeviceContext, Camera* pCamera)
		{
			D3D_PROFILING(pDeviceContext, Vertex);

			if (m_vecVertexSubset.empty())
				return;

			int nThreadID = GetThreadID(ThreadType::eRender);

			IEffectTech* pEffectTech = nullptr;

			pDeviceContext->SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			const math::Matrix matViewProjection = pCamera->GetViewMatrix(nThreadID) * pCamera->GetProjMatrix(nThreadID);

			std::map<std::tuple<IVertexBuffer*, IIndexBuffer*, bool>, RenderSubsetVertexBatch> mapVertex;
			{
				const size_t nSize = m_vecVertexSubset[nThreadID].size();
				for (size_t i = 0; i < nSize; ++i)
				{
					RenderSubsetVertex& renderSubset = m_vecVertexSubset[nThreadID][i];

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
						pDeviceContext->SetDepthStencilState(EmDepthStencilState::eRead_Write_Off);
					}
					else
					{
						pDeviceContext->SetDepthStencilState(EmDepthStencilState::eRead_Write_On);

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

						m_pEffect->SetMatrix(StrID::g_matWVP, pRenderSubset->matWorld * pCamera->GetViewMatrix(nThreadID) * pCamera->GetProjMatrix(nThreadID));
						m_pEffect->SetVector(StrID::g_color, *reinterpret_cast<const math::float4*>(&pRenderSubset->color));

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
						size_t loopCount = nInstanceSize / MAX_INSTANCE_NUM + 1;
						for (size_t j = 0; j < loopCount; ++j)
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

		void VertexRenderer::Impl::RenderLine(IDevice* pDevice, IDeviceContext* pDeviceContext, Camera* pCamera)
		{
			D3D_PROFILING(pDeviceContext, Line);

			if (m_vecLineSubset.empty())
				return;

			IEffectTech* pEffectTech = m_pEffect->GetTechnique(StrID::Color);
			if (pEffectTech == nullptr)
			{
				LOG_ERROR("Not Exist EffectTech !!");
				return;
			}

			if (pDeviceContext->SetInputLayout(pEffectTech->GetLayoutFormat()) == false)
				return;

			int nThreadID = GetThreadID(ThreadType::eRender);

			pDeviceContext->SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

			const size_t nSize = m_vecLineSubset[nThreadID].size();
			for (size_t i = 0; i < nSize; ++i)
			{
				RenderSubsetLine& renderSubset = m_vecLineSubset[nThreadID][i];

				pDeviceContext->SetVertexBuffers(renderSubset.pVertexBuffer, renderSubset.pVertexBuffer->GetFormatSize(), 0);
				pDeviceContext->SetIndexBuffer(renderSubset.pIndexBuffer, 0);

				m_pEffect->SetMatrix(StrID::g_matWVP, renderSubset.matWorld * pCamera->GetViewMatrix(nThreadID) * pCamera->GetProjMatrix(nThreadID));

				uint32_t nPassCount = pEffectTech->GetPassCount();
				for (uint32_t p = 0; p < nPassCount; ++p)
				{
					pEffectTech->PassApply(p, pDeviceContext);

					pDeviceContext->DrawIndexed(renderSubset.pIndexBuffer->GetIndexNum(), 0, 0);
				}
			}

			ClearEffect(pDeviceContext, pEffectTech);
		}

		void VertexRenderer::Impl::RenderLineSegment(IDevice* pDevice, IDeviceContext* pDeviceContext, Camera* pCamera)
		{
			D3D_PROFILING(pDeviceContext, LineSegment);

			if (m_vecLineSegmentSubset.empty())
				return;

			IEffectTech* pEffectTech = m_pEffect->GetTechnique(StrID::Color);
			if (pEffectTech == nullptr)
			{
				LOG_ERROR("Not Exist EffectTech !!");
				return;
			}

			if (pDeviceContext->SetInputLayout(pEffectTech->GetLayoutFormat()) == false)
				return;

			int nThreadID = GetThreadID(ThreadType::eRender);

			pDeviceContext->SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

			const size_t nSize = m_vecLineSegmentSubset[nThreadID].size();
			for (size_t i = 0; i < nSize; ++i)
			{
				RenderSubsetLineSegment& renderSubset = m_vecLineSegmentSubset[nThreadID][i];

				VertexPosCol* pVertices = nullptr;
				if (m_pLineSegmentVertexBuffer->Map(ThreadType::eRender, 0, D3D11_MAP_WRITE_DISCARD, reinterpret_cast<void**>(&pVertices)) == false)
					continue;

				pVertices[0] = renderSubset.vertexLineSegment[0];
				pVertices[1] = renderSubset.vertexLineSegment[1];

				m_pLineSegmentVertexBuffer->Unmap(ThreadType::eRender, 0);

				if (renderSubset.isIgnoreDepth == true)
				{
					pDeviceContext->SetDepthStencilState(EmDepthStencilState::eRead_Write_Off);
				}
				else
				{
					pDeviceContext->SetDepthStencilState(EmDepthStencilState::eRead_Write_On);
				}

				pDeviceContext->SetVertexBuffers(m_pLineSegmentVertexBuffer, m_pLineSegmentVertexBuffer->GetFormatSize(), 0);
				pDeviceContext->SetIndexBuffer(nullptr, 0);

				m_pEffect->SetMatrix(StrID::g_matWVP, pCamera->GetViewMatrix(nThreadID) * pCamera->GetProjMatrix(nThreadID));

				uint32_t nPassCount = pEffectTech->GetPassCount();
				for (uint32_t p = 0; p < nPassCount; ++p)
				{
					pEffectTech->PassApply(p, pDeviceContext);

					pDeviceContext->Draw(2, 0);
				}
			}

			ClearEffect(pDeviceContext, pEffectTech);
		}

		VertexRenderer::VertexRenderer()
			: m_pImpl{ std::make_unique<Impl>() }
		{
		}

		VertexRenderer::~VertexRenderer()
		{
		}

		void VertexRenderer::AddRender(const RenderSubsetVertex& renderSubset)
		{
			m_pImpl->AddRender(renderSubset);
		}

		void VertexRenderer::AddRender(const RenderSubsetLine& renderSubset)
		{
			m_pImpl->AddRender(renderSubset);
		}

		void VertexRenderer::AddRender(const RenderSubsetLineSegment& renderSubset)
		{
			m_pImpl->AddRender(renderSubset);
		}

		void VertexRenderer::Render(IDevice* pDevice, IDeviceContext* pDeviceContext, Camera* pCamera, uint32_t nRenderGroupFlag)
		{
			m_pImpl->Render(pDevice, pDeviceContext, pCamera, nRenderGroupFlag);
		}

		void VertexRenderer::Flush()
		{
			m_pImpl->Flush();
		}
	}
}