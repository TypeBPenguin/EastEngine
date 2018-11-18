#include "stdafx.h"
#include "RenderManagerDX12.h"

#include "GraphicsInterface/Camera.h"

#include "UtilDX12.h"
#include "DeviceDX12.h"
#include "GBufferDX12.h"

#include "ModelRendererDX12.h"
#include "DeferredRendererDX12.h"
#include "EnvironmentRendererDX12.h"
#include "TerrainRendererDX12.h"

#include "FxaaDX12.h"
#include "DownScaleDX12.h"
#include "GaussianBlurDX12.h"
#include "DepthOfFieldDX12.h"
#include "AssaoDX12.h"
#include "ColorGradingDX12.h"
#include "BloomFilterDX12.h"
#include "SSSDX12.h"
#include "HDRFilterDX12.h"
#include "VertexRendererDX12.h"

namespace eastengine
{
	namespace graphics
	{
		namespace dx12
		{
			class RenderManager::Impl
			{
			public:
				Impl();
				~Impl();

			public:
				void Cleanup();
				void Render();

			public:
				void PushJob(const RenderJobStatic& renderJob) { GetModelRenderer()->PushJob(renderJob); }
				void PushJob(const RenderJobSkinned& renderJob) { GetModelRenderer()->PushJob(renderJob); }
				void PushJob(const RenderJobTerrain& renderJob) { GetTerrainRenderer()->PushJob(renderJob); }
				void PushJob(const RenderJobVertex& renderJob) { GetVertexRenderer()->PushJob(renderJob); };

			private:
				void UpdateOptions(const Options& curOptions);

			private:
				ModelRenderer* GetModelRenderer() const { return static_cast<ModelRenderer*>(m_pRenderers[IRenderer::eModel].get()); }
				DeferredRenderer* GetDeferredRenderer() const { return static_cast<DeferredRenderer*>(m_pRenderers[IRenderer::eDeferred].get()); }
				EnvironmentRenderer* GetEnvironmentRenderer() const { return static_cast<EnvironmentRenderer*>(m_pRenderers[IRenderer::eEnvironment].get()); }
				TerrainRenderer* GetTerrainRenderer() const { return static_cast<TerrainRenderer*>(m_pRenderers[IRenderer::eTerrain].get()); }
				VertexRenderer* GetVertexRenderer() const { return static_cast<VertexRenderer*>(m_pRenderers[IRenderer::eVertex].get()); }
				Fxaa* GetFxaa() const { return static_cast<Fxaa*>(m_pRenderers[IRenderer::eFxaa].get()); }
				DownScale* GetDownScale() const { return static_cast<DownScale*>(m_pRenderers[IRenderer::eDownScale].get()); }
				GaussianBlur* GetGaussianBlur() const { return static_cast<GaussianBlur*>(m_pRenderers[IRenderer::eGaussianBlur].get()); }
				DepthOfField* GetDepthOfField() const { return static_cast<DepthOfField*>(m_pRenderers[IRenderer::eDepthOfField].get()); }
				Assao* GetAssao() const { return static_cast<Assao*>(m_pRenderers[IRenderer::eAssao].get()); }
				ColorGrading* GetColorGrading() const { return static_cast<ColorGrading*>(m_pRenderers[IRenderer::eColorGrading].get()); }
				BloomFilter* GetBloomFilter() const { return static_cast<BloomFilter*>(m_pRenderers[IRenderer::eBloomFilter].get()); }
				SSS* GetSSS() const { return static_cast<SSS*>(m_pRenderers[IRenderer::eSSS].get()); }
				HDRFilter* GetHDRFilter() const { return static_cast<HDRFilter*>(m_pRenderers[IRenderer::eHDR].get()); }

			private:
				std::array<std::unique_ptr<IRendererDX12>, IRenderer::TypeCount> m_pRenderers{ nullptr };
			};

			RenderManager::Impl::Impl()
			{
				m_pRenderers[IRenderer::eModel] = std::make_unique<ModelRenderer>();
				m_pRenderers[IRenderer::eDeferred] = std::make_unique<DeferredRenderer>();
				m_pRenderers[IRenderer::eEnvironment] = std::make_unique<EnvironmentRenderer>();
				m_pRenderers[IRenderer::eTerrain] = std::make_unique<TerrainRenderer>();
				m_pRenderers[IRenderer::eVertex] = std::make_unique<VertexRenderer>();
			}

			RenderManager::Impl::~Impl()
			{
			}

			void RenderManager::Impl::Cleanup()
			{
				GetModelRenderer()->Cleanup();
				GetDeferredRenderer()->Cleanup();
				GetEnvironmentRenderer()->Cleanup();
				GetTerrainRenderer()->Cleanup();
				GetVertexRenderer()->Cleanup();
			}

			void RenderManager::Impl::Render()
			{
				TRACER_EVENT(__FUNCTION__);
				Device* pDeviceInstance = Device::GetInstance();
				const uint32_t nFrameIndex = pDeviceInstance->GetFrameIndex();

				GBuffer* pGBuffer = pDeviceInstance->GetGBuffer(nFrameIndex);
				{
					TRACER_EVENT("ClearBuffer");
					RenderTarget* pNormalsRT = pGBuffer->GetRenderTarget(EmGBuffer::eNormals);
					RenderTarget* pColorsRT = pGBuffer->GetRenderTarget(EmGBuffer::eColors);
					RenderTarget* pDisneyBRDFRT = pGBuffer->GetRenderTarget(EmGBuffer::eDisneyBRDF);
					DepthStencil* pDepthStencil = pGBuffer->GetDepthStencil();

					ID3D12GraphicsCommandList2* pCommandList = pDeviceInstance->GetCommandList(0);
					pDeviceInstance->ResetCommandList(0, nullptr);

					util::ChangeResourceState(pCommandList, pNormalsRT, D3D12_RESOURCE_STATE_RENDER_TARGET);
					util::ChangeResourceState(pCommandList, pColorsRT, D3D12_RESOURCE_STATE_RENDER_TARGET);
					util::ChangeResourceState(pCommandList, pDisneyBRDFRT, D3D12_RESOURCE_STATE_RENDER_TARGET);
					util::ChangeResourceState(pCommandList, pDepthStencil, D3D12_RESOURCE_STATE_DEPTH_WRITE);

					pGBuffer->Clear(pCommandList);

					pCommandList->Close();
					pDeviceInstance->ExecuteCommandList(pCommandList);
				}

				Camera* pCamera = Camera::GetInstance();
				const Options& options = GetOptions();

				UpdateOptions(options);

				GetEnvironmentRenderer()->Render(pCamera);
				GetTerrainRenderer()->Render(pCamera);

				GetModelRenderer()->Render(pCamera, ModelRenderer::eDeferred);
				GetDeferredRenderer()->Render(pCamera);

				D3D12_RESOURCE_DESC swapchainDesc = pDeviceInstance->GetSwapChainRenderTarget(nFrameIndex)->GetDesc();
				
				// PostProcessing
				{
					if (options.OnSSS == true)
					{
						TRACER_EVENT("SSS");
						D3D12_RESOURCE_DESC swapchainDesc_forSSS = swapchainDesc;
						if (options.OnHDR == true)
						{
							swapchainDesc_forSSS.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
						}

						RenderTarget* pSSS = pDeviceInstance->GetRenderTarget(&swapchainDesc_forSSS, math::Color::Transparent, false);

						RenderTarget* pSource = pDeviceInstance->GetLastUsedRenderTarget();
						DepthStencil* pDepth = pGBuffer->GetDepthStencil();

						GetSSS()->Apply(pSource, pDepth, pSSS);

						pDeviceInstance->ReleaseRenderTargets(&pSSS);
					}

					if (options.OnASSAO == true)
					{
						TRACER_EVENT("ASSAO");
						RenderTarget* pLastUseRenderTarget = pDeviceInstance->GetLastUsedRenderTarget();
						const RenderTarget* pNormalMap = pGBuffer->GetRenderTarget(EmGBuffer::eNormals);
						const DepthStencil* pDepth = pGBuffer->GetDepthStencil();

						GetAssao()->Apply(pCamera, pNormalMap, pDepth, pLastUseRenderTarget);
					}
				}

				GetModelRenderer()->Render(pCamera, ModelRenderer::eAlphaBlend);
				GetVertexRenderer()->Render(pCamera);

				{
					if (options.OnHDR == true)
					{
						TRACER_EVENT("HDR");
						RenderTarget* pHDR = pDeviceInstance->GetRenderTarget(&swapchainDesc, math::Color::Transparent, false);
						RenderTarget* pSource = pDeviceInstance->GetLastUsedRenderTarget();
						GetHDRFilter()->Apply(pSource, pHDR);

						pDeviceInstance->ReleaseRenderTargets(&pHDR);
					}

					if (options.OnBloomFilter == true)
					{
						TRACER_EVENT("BloomFilter");
						RenderTarget* pSource = pDeviceInstance->GetLastUsedRenderTarget();
						GetBloomFilter()->Apply(pSource);
					}

					if (options.OnColorGrading == true)
					{
						TRACER_EVENT("ColorGrading");
						RenderTarget* pColorGrading = pDeviceInstance->GetRenderTarget(&swapchainDesc, math::Color::Transparent, false);
						RenderTarget* pSource = pDeviceInstance->GetLastUsedRenderTarget();

						GetColorGrading()->Apply(pCamera, pSource, pColorGrading);

						pDeviceInstance->ReleaseRenderTargets(&pColorGrading);
					}

					if (options.OnDOF == true)
					{
						TRACER_EVENT("DOF");
						RenderTarget* pDepthOfField = pDeviceInstance->GetRenderTarget(&swapchainDesc, math::Color::Transparent, false);
						RenderTarget* pSource = pDeviceInstance->GetLastUsedRenderTarget();
						DepthStencil* pDepth = pGBuffer->GetDepthStencil();

						GetDepthOfField()->Apply(pCamera, pSource, pDepth, pDepthOfField);

						pDeviceInstance->ReleaseRenderTargets(&pDepthOfField);
					}

					if (options.OnFXAA == true)
					{
						TRACER_EVENT("FXAA");
						RenderTarget* pFxaa = pDeviceInstance->GetRenderTarget(&swapchainDesc, math::Color::Transparent, false);
						RenderTarget* pSource = pDeviceInstance->GetLastUsedRenderTarget();

						GetFxaa()->Apply(pSource, pFxaa);

						pDeviceInstance->ReleaseRenderTargets(&pFxaa);
					}
				}

				// copy to swapchain
				{
					TRACER_EVENT("CopyToSwapchain");
					RenderTarget* pSwapChainRenderTarget = pDeviceInstance->GetSwapChainRenderTarget(nFrameIndex);
					RenderTarget* pLastUseRenderTarget = pDeviceInstance->GetLastUsedRenderTarget();

					ID3D12GraphicsCommandList2* pCommandList = pDeviceInstance->GetCommandList(0);
					pDeviceInstance->ResetCommandList(0, nullptr);

					util::ChangeResourceState(pCommandList, pSwapChainRenderTarget, D3D12_RESOURCE_STATE_COPY_DEST);
					util::ChangeResourceState(pCommandList, pLastUseRenderTarget, D3D12_RESOURCE_STATE_COPY_SOURCE);

					pCommandList->CopyResource(pSwapChainRenderTarget->GetResource(), pLastUseRenderTarget->GetResource());

					util::ChangeResourceState(pCommandList, pSwapChainRenderTarget, D3D12_RESOURCE_STATE_PRESENT);
					util::ChangeResourceState(pCommandList, pLastUseRenderTarget, D3D12_RESOURCE_STATE_RENDER_TARGET);

					HRESULT hr = pCommandList->Close();
					if (FAILED(hr))
					{
						throw_line("failed to close command list");
					}

					pDeviceInstance->ExecuteCommandList(pCommandList);
				}
			}

			void RenderManager::Impl::UpdateOptions(const Options& curOptions)
			{
				const Options& prevOptions = GetPrevOptions();
				if (prevOptions.OnHDR != curOptions.OnHDR)
				{
					if (curOptions.OnHDR == true)
					{
						m_pRenderers[IRenderer::eHDR] = std::make_unique<HDRFilter>();
					}
					else
					{
						m_pRenderers[IRenderer::eHDR].reset();
					}

					ID3D12Device* pDevice = Device::GetInstance()->GetInterface();
					for (auto& pRenderer : m_pRenderers)
					{
						if (pRenderer != nullptr)
						{
							pRenderer->RefreshPSO(pDevice);
						}
					}
				}

				if (prevOptions.OnFXAA != curOptions.OnFXAA)
				{
					if (curOptions.OnFXAA == true)
					{
						m_pRenderers[IRenderer::eFxaa] = std::make_unique<Fxaa>();
					}
					else
					{
						m_pRenderers[IRenderer::eFxaa].reset();
					}
				}

				if (prevOptions.OnDOF != curOptions.OnDOF)
				{
					if (curOptions.OnDOF == true)
					{
						m_pRenderers[IRenderer::eDepthOfField] = std::make_unique<DepthOfField>();
					}
					else
					{
						m_pRenderers[IRenderer::eDepthOfField].reset();
					}
				}

				if (prevOptions.OnASSAO != curOptions.OnASSAO)
				{
					if (curOptions.OnASSAO == true)
					{
						m_pRenderers[IRenderer::eAssao] = std::make_unique<Assao>();
					}
					else
					{
						m_pRenderers[IRenderer::eAssao].reset();
					}
				}

				if (prevOptions.OnColorGrading != curOptions.OnColorGrading)
				{
					if (curOptions.OnColorGrading == true)
					{
						m_pRenderers[IRenderer::eColorGrading] = std::make_unique<ColorGrading>();
					}
					else
					{
						m_pRenderers[IRenderer::eColorGrading].reset();
					}
				}

				if (prevOptions.OnBloomFilter != curOptions.OnBloomFilter)
				{
					if (curOptions.OnBloomFilter == true)
					{
						m_pRenderers[IRenderer::eBloomFilter] = std::make_unique<BloomFilter>();
					}
					else
					{
						m_pRenderers[IRenderer::eBloomFilter].reset();
					}
				}

				if (prevOptions.OnSSS != curOptions.OnSSS)
				{
					if (curOptions.OnSSS == true)
					{
						m_pRenderers[IRenderer::eSSS] = std::make_unique<SSS>();
					}
					else
					{
						m_pRenderers[IRenderer::eSSS].reset();
					}
				}
			}

			RenderManager::RenderManager()
				: m_pImpl{ std::make_unique<Impl>() }
			{
			}

			RenderManager::~RenderManager()
			{
			}

			void RenderManager::Cleanup()
			{
				m_pImpl->Cleanup();
			}

			void RenderManager::Render()
			{
				m_pImpl->Render();
			}

			void RenderManager::PushJob(const RenderJobStatic& renderJob)
			{
				m_pImpl->PushJob(renderJob);
			}

			void RenderManager::PushJob(const RenderJobSkinned& renderJob)
			{
				m_pImpl->PushJob(renderJob);
			}

			void RenderManager::PushJob(const RenderJobTerrain& renderJob)
			{
				m_pImpl->PushJob(renderJob);
			}

			void RenderManager::PushJob(const RenderJobVertex& renderJob)
			{
				m_pImpl->PushJob(renderJob);
			}
		}
	}
}