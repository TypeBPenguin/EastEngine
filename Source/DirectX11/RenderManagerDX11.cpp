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
				void Flush();
				void Render();

			public:
				void PushJob(const RenderJobStatic& renderJob) { m_pModelRenderer->PushJob(renderJob); }
				void PushJob(const RenderJobSkinned& renderJob) { m_pModelRenderer->PushJob(renderJob); }
				void PushJob(const RenderJobTerrain& renderJob) { m_pTerrainRenderer->PushJob(renderJob); }

			private:
				void UpdateOptions(const Options& curOptions);

			private:
				std::unique_ptr<ModelRenderer> m_pModelRenderer;
				std::unique_ptr<DeferredRenderer> m_pDeferredRenderer;
				std::unique_ptr<EnvironmentRenderer> m_pEnvironmentRenderer;
				std::unique_ptr<TerrainRenderer> m_pTerrainRenderer;

				// PostProcessing
				std::unique_ptr<Fxaa> m_pFxaa;
				std::unique_ptr<DownScale> m_pDownScale;
				std::unique_ptr<GaussianBlur> m_pGaussianBlur;
				std::unique_ptr<DepthOfField> m_pDepthOfField;
				std::unique_ptr<Assao> m_pAssao;
				std::unique_ptr<ColorGrading> m_pColorGrading;
				std::unique_ptr<BloomFilter> m_pBloomFilter;
				std::unique_ptr<SSS> m_pSSS;

				Options m_prevOptions;
			};

			RenderManager::Impl::Impl()
				: m_pModelRenderer{ std::make_unique<ModelRenderer>() }
				, m_pDeferredRenderer{ std::make_unique<DeferredRenderer>() }
				, m_pEnvironmentRenderer{ std::make_unique<EnvironmentRenderer>() }
				, m_pTerrainRenderer{ std::make_unique<TerrainRenderer>() }
			{
			}

			RenderManager::Impl::~Impl()
			{
			}

			void RenderManager::Impl::Flush()
			{
				m_pModelRenderer->Flush();
				m_pDeferredRenderer->Flush();
				m_pEnvironmentRenderer->Flush();
				m_pTerrainRenderer->Flush();
			}

			void RenderManager::Impl::Render()
			{
				Device* pDeviceInstance = Device::GetInstance();
				ID3D11Device* pDevice = pDeviceInstance->GetInterface();
				ID3D11DeviceContext* pImmediateContext = pDeviceInstance->GetImmediateContext();

				const GBuffer* pGBuffer = pDeviceInstance->GetGBuffer();

				Camera* pCamera = Camera::GetInstance();
				const Options& options = GetOptions();

				UpdateOptions(options);

				m_pEnvironmentRenderer->Render(pDevice, pImmediateContext, pCamera);
				m_pTerrainRenderer->Render(pDevice, pImmediateContext, pCamera);

				m_pModelRenderer->Render(pDevice, pImmediateContext, pCamera, ModelRenderer::eDeferred);
				m_pDeferredRenderer->Render(pDevice, pImmediateContext, pCamera);

				D3D11_TEXTURE2D_DESC swapchainDesc{};
				pDeviceInstance->GetSwapChainRenderTarget()->GetDesc2D(&swapchainDesc);
				swapchainDesc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;

				// PostProcessing
				{
					if (options.OnSSS == true)
					{
						RenderTarget* pSSS = pDeviceInstance->GetRenderTarget(&swapchainDesc, false);
						pImmediateContext->ClearRenderTargetView(pSSS->GetRenderTargetView(), math::Color::Transparent);

						const RenderTarget* pSource = pDeviceInstance->GetLastUsedRenderTarget();
						const DepthStencil* pDepth = pGBuffer->GetDepthStencil();
						m_pSSS->Apply(pSource, pDepth, pSSS);

						pDeviceInstance->ReleaseRenderTargets(&pSSS);
					}

					if (options.OnASSAO == true)
					{
						RenderTarget* pLastUseRenderTarget = pDeviceInstance->GetLastUsedRenderTarget();
						const RenderTarget* pNormalMap = pGBuffer->GetRenderTarget(EmGBuffer::eNormals);
						const DepthStencil* pDepth = pGBuffer->GetDepthStencil();

						m_pAssao->Apply(pCamera, pNormalMap, pDepth, pLastUseRenderTarget);
					}
				}

				m_pModelRenderer->Render(pDevice, pImmediateContext, pCamera, ModelRenderer::eAlphaBlend);

				{
					// HDR
					if (options.OnBloomFilter == true)
					{
						RenderTarget* pSource = pDeviceInstance->GetLastUsedRenderTarget();
						m_pBloomFilter->Apply(pSource);
					}

					if (options.OnColorGrading == true)
					{
						RenderTarget* pColorGrading = pDeviceInstance->GetRenderTarget(&swapchainDesc, false);
						const RenderTarget* pSource = pDeviceInstance->GetLastUsedRenderTarget();

						m_pColorGrading->Apply(pCamera, pSource, pColorGrading);

						pDeviceInstance->ReleaseRenderTargets(&pColorGrading);
					}

					if (options.OnDOF == true)
					{
						RenderTarget* pDepthOfField = pDeviceInstance->GetRenderTarget(&swapchainDesc, false);
						const RenderTarget* pSource = pDeviceInstance->GetLastUsedRenderTarget();
						const DepthStencil* pDepth = pGBuffer->GetDepthStencil();

						m_pDepthOfField->Apply(pCamera, pSource, pDepth, pDepthOfField);

						pDeviceInstance->ReleaseRenderTargets(&pDepthOfField);
					}

					if (options.OnFXAA == true)
					{
						RenderTarget* pFxaa = pDeviceInstance->GetRenderTarget(&swapchainDesc, false);
						const RenderTarget* pSource = pDeviceInstance->GetLastUsedRenderTarget();

						m_pFxaa->Apply(pSource, pFxaa);

						pDeviceInstance->ReleaseRenderTargets(&pFxaa);
					}
				}

				RenderTarget* pSwapChainRenderTarget = pDeviceInstance->GetSwapChainRenderTarget();
				RenderTarget* pLastUseRenderTarget = pDeviceInstance->GetLastUsedRenderTarget();
				pImmediateContext->CopyResource(pSwapChainRenderTarget->GetTexture2D(), pLastUseRenderTarget->GetTexture2D());
			}

			void RenderManager::Impl::UpdateOptions(const Options& curOptions)
			{
				if (m_prevOptions.OnHDR != curOptions.OnHDR)
				{
					if (curOptions.OnHDR == true)
					{
					}
					else
					{
					}
				}

				if (m_prevOptions.OnFXAA != curOptions.OnFXAA)
				{
					if (curOptions.OnFXAA == true)
					{
						m_pFxaa = std::make_unique<Fxaa>();
					}
					else
					{
						m_pFxaa.reset();
					}
				}

				if (m_prevOptions.OnDOF != curOptions.OnDOF)
				{
					if (curOptions.OnDOF == true)
					{
						m_pDepthOfField = std::make_unique<DepthOfField>();
					}
					else
					{
						m_pDepthOfField.reset();
					}
				}

				if (m_prevOptions.OnASSAO != curOptions.OnASSAO)
				{
					if (curOptions.OnASSAO == true)
					{
						m_pAssao = std::make_unique<Assao>();
					}
					else
					{
						m_pAssao.reset();
					}
				}

				if (m_prevOptions.OnColorGrading != curOptions.OnColorGrading)
				{
					if (curOptions.OnColorGrading == true)
					{
						m_pColorGrading = std::make_unique<ColorGrading>();
					}
					else
					{
						m_pColorGrading.reset();
					}
				}

				if (m_prevOptions.OnBloomFilter != curOptions.OnBloomFilter)
				{
					if (curOptions.OnBloomFilter == true)
					{
						m_pBloomFilter = std::make_unique<BloomFilter>();
					}
					else
					{
						m_pBloomFilter.reset();
					}
				}

				if (m_prevOptions.OnSSS != curOptions.OnSSS)
				{
					if (curOptions.OnSSS == true)
					{
						m_pSSS = std::make_unique<SSS>();
					}
					else
					{
						m_pSSS.reset();
					}
				}

				m_prevOptions = curOptions;
			}

			RenderManager::RenderManager()
				: m_pImpl{ std::make_unique<Impl>() }
			{
			}

			RenderManager::~RenderManager()
			{
			}

			void RenderManager::Flush()
			{
				m_pImpl->Flush();
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
		}
	}
}