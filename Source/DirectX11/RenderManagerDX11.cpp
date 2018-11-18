#include "stdafx.h"
#include "RenderManagerDX11.h"

#include "GraphicsInterface/Camera.h"

#include "DeviceDX11.h"
#include "GBufferDX11.h"

#include "ModelRendererDX11.h"
#include "DeferredRendererDX11.h"
#include "EnvironmentRendererDX11.h"
#include "TerrainRendererDX11.h"

#include "FxaaDX11.h"
#include "DownScaleDX11.h"
#include "GaussianBlurDX11.h"
#include "DepthOfFieldDX11.h"
#include "AssaoDX11.h"
#include "ColorGradingDX11.h"
#include "BloomFilterDX11.h"
#include "SSSDX11.h"
#include "HDRFilterDX11.h"
#include "VertexRendererDX11.h"

namespace eastengine
{
	namespace graphics
	{
		namespace dx11
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
				void PushJob(const RenderJobVertex& renderJob) { GetVertexRenderer()->PushJob(renderJob); }

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
				std::array<std::unique_ptr<IRenderer>, IRenderer::TypeCount> m_pRenderers{ nullptr };
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
				ID3D11Device* pDevice = pDeviceInstance->GetInterface();
				ID3D11DeviceContext* pImmediateContext = pDeviceInstance->GetImmediateContext();

				const GBuffer* pGBuffer = pDeviceInstance->GetGBuffer();

				Camera* pCamera = Camera::GetInstance();
				const Options& options = GetOptions();

				UpdateOptions(options);

				GetEnvironmentRenderer()->Render(pDevice, pImmediateContext, pCamera);
				GetTerrainRenderer()->Render(pDevice, pImmediateContext, pCamera);

				GetModelRenderer()->Render(pDevice, pImmediateContext, pCamera, ModelRenderer::eDeferred);
				GetDeferredRenderer()->Render(pDevice, pImmediateContext, pCamera);

				D3D11_TEXTURE2D_DESC swapchainDesc{};
				pDeviceInstance->GetSwapChainRenderTarget()->GetDesc2D(&swapchainDesc);
				swapchainDesc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;

				// PostProcessing
				{
					if (options.OnSSS == true)
					{
						TRACER_EVENT("SSS");
						D3D11_TEXTURE2D_DESC swapchainDesc_forSSS = swapchainDesc;
						if (options.OnHDR == true)
						{
							swapchainDesc_forSSS.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
						}

						RenderTarget* pSSS = pDeviceInstance->GetRenderTarget(&swapchainDesc_forSSS, false);
						pImmediateContext->ClearRenderTargetView(pSSS->GetRenderTargetView(), math::Color::Transparent);

						const RenderTarget* pSource = pDeviceInstance->GetLastUsedRenderTarget();
						const DepthStencil* pDepth = pGBuffer->GetDepthStencil();
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

				GetModelRenderer()->Render(pDevice, pImmediateContext, pCamera, ModelRenderer::eAlphaBlend);
				GetVertexRenderer()->Render(pDevice, pImmediateContext, pCamera);

				{
					if (options.OnHDR == true)
					{
						TRACER_EVENT("OnHDR");
						RenderTarget* pHDR = pDeviceInstance->GetRenderTarget(&swapchainDesc, false);
						const RenderTarget* pSource = pDeviceInstance->GetLastUsedRenderTarget();
						GetHDRFilter()->Apply(pSource, pHDR);

						pDeviceInstance->ReleaseRenderTargets(&pHDR);
					}

					if (options.OnBloomFilter == true)
					{
						TRACER_EVENT("OnBloomFilter");
						RenderTarget* pSourceAndResult = pDeviceInstance->GetLastUsedRenderTarget();
						GetBloomFilter()->Apply(pSourceAndResult);
					}

					if (options.OnColorGrading == true)
					{
						TRACER_EVENT("OnColorGrading");
						RenderTarget* pColorGrading = pDeviceInstance->GetRenderTarget(&swapchainDesc, false);
						const RenderTarget* pSource = pDeviceInstance->GetLastUsedRenderTarget();

						GetColorGrading()->Apply(pCamera, pSource, pColorGrading);

						pDeviceInstance->ReleaseRenderTargets(&pColorGrading);
					}

					if (options.OnDOF == true)
					{
						TRACER_EVENT("OnDOF");
						RenderTarget* pDepthOfField = pDeviceInstance->GetRenderTarget(&swapchainDesc, false);
						const RenderTarget* pSource = pDeviceInstance->GetLastUsedRenderTarget();
						const DepthStencil* pDepth = pGBuffer->GetDepthStencil();

						GetDepthOfField()->Apply(pCamera, pSource, pDepth, pDepthOfField);

						pDeviceInstance->ReleaseRenderTargets(&pDepthOfField);
					}

					if (options.OnFXAA == true)
					{
						TRACER_EVENT("OnFXAA");
						RenderTarget* pFxaa = pDeviceInstance->GetRenderTarget(&swapchainDesc, false);
						const RenderTarget* pSource = pDeviceInstance->GetLastUsedRenderTarget();

						GetFxaa()->Apply(pSource, pFxaa);

						pDeviceInstance->ReleaseRenderTargets(&pFxaa);
					}
				}

				RenderTarget* pSwapChainRenderTarget = pDeviceInstance->GetSwapChainRenderTarget();
				RenderTarget* pLastUseRenderTarget = pDeviceInstance->GetLastUsedRenderTarget();
				pImmediateContext->CopyResource(pSwapChainRenderTarget->GetTexture2D(), pLastUseRenderTarget->GetTexture2D());
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