#include "stdafx.h"
#include "RenderManagerDX12.h"

#include "GraphicsInterface/Camera.h"

#include "DeviceDX12.h"
#include "GBufferDX12.h"

#include "ModelRendererDX12.h"
#include "DeferredRendererDX12.h"
#include "EnvironmentRendererDX12.h"

#include "FxaaDX12.h"
#include "DownScaleDX12.h"
#include "GaussianBlurDX12.h"
#include "DepthOfFieldDX12.h"
#include "AssaoDX12.h"
#include "ColorGradingDX12.h"
#include "BloomFilterDX12.h"
#include "SSSDX12.h"

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
				void Flush();
				void Render();

			public:
				void PushJob(const RenderJobStatic& renderJob) { m_pModelRenderer->PushJob(renderJob); }
				void PushJob(const RenderJobSkinned& renderJob) { m_pModelRenderer->PushJob(renderJob); }

			private:
				void UpdateOptions(const Options& curOptions);

			private:
				std::unique_ptr<ModelRenderer> m_pModelRenderer;
				std::unique_ptr<DeferredRenderer> m_pDeferredRenderer;
				std::unique_ptr<EnvironmentRenderer> m_pEnvironmentRenderer;

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
			}

			void RenderManager::Impl::Render()
			{
				Device* pDeviceInstance = Device::GetInstance();
				int nFrameIndex = pDeviceInstance->GetFrameIndex();

				GBuffer* pGBuffer = pDeviceInstance->GetGBuffer(nFrameIndex);
				{
					RenderTarget* pNormalsRT = pGBuffer->GetRenderTarget(EmGBuffer::eNormals);
					RenderTarget* pColorsRT = pGBuffer->GetRenderTarget(EmGBuffer::eColors);
					RenderTarget* pDisneyBRDFRT = pGBuffer->GetRenderTarget(EmGBuffer::eDisneyBRDF);
					DepthStencil* pDepthStencil = pGBuffer->GetDepthStencil();

					ID3D12GraphicsCommandList2* pCommandList = pDeviceInstance->GetCommandList(0);
					pDeviceInstance->ResetCommandList(0, nullptr);

					const D3D12_RESOURCE_BARRIER transition[] =
					{
						pNormalsRT->Transition(D3D12_RESOURCE_STATE_RENDER_TARGET),
						pColorsRT->Transition(D3D12_RESOURCE_STATE_RENDER_TARGET),
						pDisneyBRDFRT->Transition(D3D12_RESOURCE_STATE_RENDER_TARGET),
						pDepthStencil->Transition(D3D12_RESOURCE_STATE_DEPTH_WRITE),
					};
					pCommandList->ResourceBarrier(_countof(transition), transition);

					pGBuffer->Clear(pCommandList);

					pCommandList->Close();
					pDeviceInstance->ExecuteCommandList(pCommandList);
				}

				Camera* pCamera = Camera::GetInstance();
				const Options& options = GetOptions();

				UpdateOptions(options);

				m_pEnvironmentRenderer->Render(pCamera);

				m_pModelRenderer->Render(pCamera, ModelRenderer::eDeferred);
				m_pDeferredRenderer->Render(pCamera);

				const D3D12_RESOURCE_DESC swapchainDesc = pDeviceInstance->GetSwapChainRenderTarget(nFrameIndex)->GetDesc();

				// PostProcessing
				{
					if (options.OnSSS == true)
					{
						RenderTarget* pSSS = pDeviceInstance->GetRenderTarget(&swapchainDesc, math::Color::Transparent, false);

						RenderTarget* pSource = pDeviceInstance->GetLastUsedRenderTarget();
						DepthStencil* pDepth = pGBuffer->GetDepthStencil();

						ID3D12GraphicsCommandList2* pCommandList = pDeviceInstance->GetCommandList(0);
						pDeviceInstance->ResetCommandList(0, nullptr);

						if (pSSS->GetState() != D3D12_RESOURCE_STATE_RENDER_TARGET)
						{
							const D3D12_RESOURCE_BARRIER transition[] =
							{
								pSSS->Transition(D3D12_RESOURCE_STATE_RENDER_TARGET),
							};
							pCommandList->ResourceBarrier(_countof(transition), transition);
						}
						pSSS->Clear(pCommandList);

						if (pSource->GetState() != D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE)
						{
							const D3D12_RESOURCE_BARRIER transition[] =
							{
								pSource->Transition(D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE),
							};
							pCommandList->ResourceBarrier(_countof(transition), transition);
						}

						m_pSSS->Apply(pCommandList, pSource, pDepth, pSSS);

						HRESULT hr = pCommandList->Close();
						if (FAILED(hr))
						{
							throw_line("failed to close command list");
						}
						pDeviceInstance->ExecuteCommandList(pCommandList);

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

				m_pModelRenderer->Render(pCamera, ModelRenderer::eAlphaBlend);

				{
					// HDR
					if (options.OnBloomFilter == true)
					{
						RenderTarget* pSource = pDeviceInstance->GetLastUsedRenderTarget();
						m_pBloomFilter->Apply(pSource);
					}

					if (options.OnColorGrading == true)
					{
						RenderTarget* pColorGrading = pDeviceInstance->GetRenderTarget(&swapchainDesc, math::Color::Transparent, false);
						const RenderTarget* pSource = pDeviceInstance->GetLastUsedRenderTarget();

						m_pColorGrading->Apply(pCamera, pSource, pColorGrading);

						pDeviceInstance->ReleaseRenderTargets(&pColorGrading);
					}

					if (options.OnDOF == true)
					{
						RenderTarget* pDepthOfField = pDeviceInstance->GetRenderTarget(&swapchainDesc, math::Color::Transparent, false);
						RenderTarget* pSource = pDeviceInstance->GetLastUsedRenderTarget();
						DepthStencil* pDepth = pGBuffer->GetDepthStencil();

						ID3D12GraphicsCommandList2* pCommandList = pDeviceInstance->GetCommandList(0);
						pDeviceInstance->ResetCommandList(0, nullptr);

						if (pSource->GetState() != D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE)
						{
							const D3D12_RESOURCE_BARRIER transition[] =
							{
								pSource->Transition(D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE),
							};
							pCommandList->ResourceBarrier(_countof(transition), transition);
						}

						m_pDepthOfField->Apply(pCommandList, pCamera, pSource, pDepth, pDepthOfField);

						if (pSource->GetState() != D3D12_RESOURCE_STATE_RENDER_TARGET)
						{
							const D3D12_RESOURCE_BARRIER transition[] =
							{
								pSource->Transition(D3D12_RESOURCE_STATE_RENDER_TARGET),
							};
							pCommandList->ResourceBarrier(_countof(transition), transition);
						}

						HRESULT hr = pCommandList->Close();
						if (FAILED(hr))
						{
							throw_line("failed to close command list");
						}
						pDeviceInstance->ExecuteCommandList(pCommandList);

						pDeviceInstance->ReleaseRenderTargets(&pDepthOfField);
					}

					if (options.OnFXAA == true)
					{
						RenderTarget* pFxaa = pDeviceInstance->GetRenderTarget(&swapchainDesc, math::Color::Transparent, false);
						const RenderTarget* pSource = pDeviceInstance->GetLastUsedRenderTarget();

						m_pFxaa->Apply(pSource, pFxaa);

						pDeviceInstance->ReleaseRenderTargets(&pFxaa);
					}
				}

				// copy to swapchain
				{
					RenderTarget* pSwapChainRenderTarget = pDeviceInstance->GetSwapChainRenderTarget(nFrameIndex);
					RenderTarget* pLastUseRenderTarget = pDeviceInstance->GetLastUsedRenderTarget();

					ID3D12GraphicsCommandList2* pCommandList = pDeviceInstance->GetCommandList(0);
					pDeviceInstance->ResetCommandList(0, nullptr);
					{
						const D3D12_RESOURCE_BARRIER transition[] =
						{
							pSwapChainRenderTarget->Transition(D3D12_RESOURCE_STATE_COPY_DEST),
							pLastUseRenderTarget->Transition(D3D12_RESOURCE_STATE_COPY_SOURCE),
						};
						pCommandList->ResourceBarrier(_countof(transition), transition);
					}

					pCommandList->CopyResource(pSwapChainRenderTarget->GetResource(), pLastUseRenderTarget->GetResource());

					{
						const D3D12_RESOURCE_BARRIER transition[] =
						{
							pSwapChainRenderTarget->Transition(D3D12_RESOURCE_STATE_PRESENT),
							pLastUseRenderTarget->Transition(D3D12_RESOURCE_STATE_RENDER_TARGET),
						};
						pCommandList->ResourceBarrier(_countof(transition), transition);
					}

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
		}
	}
}