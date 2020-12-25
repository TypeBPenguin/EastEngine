#include "stdafx.h"
#include "RenderManagerDX12.h"

#include "Graphics/Interface/Camera.h"

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
#include "MotionBlurDX12.h"
#include "CopyRendererDX12.h"

namespace est
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
				void AllCleanup();
				void Cleanup();
				void Render();

			public:
				void Copy_RGBA(RenderTarget* pSource, RenderTarget* pResult);
				void Copy_RGB(RenderTarget* pSource, RenderTarget* pResult);

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
				MotionBlur* GetMotionBlur() const { return static_cast<MotionBlur*>(m_pRenderers[IRenderer::eMotionBlur].get()); }
				CopyRenderer* GetCopy() const { return static_cast<CopyRenderer*>(m_pRenderers[IRenderer::eCopy].get()); }

			private:
				std::array<std::unique_ptr<IRendererDX12>, IRenderer::TypeCount> m_pRenderers{ nullptr };

				math::Matrix m_matPrevViewProjection;
			};

			RenderManager::Impl::Impl()
			{
				m_pRenderers[IRenderer::eModel] = std::make_unique<ModelRenderer>();
				m_pRenderers[IRenderer::eDeferred] = std::make_unique<DeferredRenderer>();
				m_pRenderers[IRenderer::eEnvironment] = std::make_unique<EnvironmentRenderer>();
				m_pRenderers[IRenderer::eTerrain] = std::make_unique<TerrainRenderer>();
				m_pRenderers[IRenderer::eVertex] = std::make_unique<VertexRenderer>();
				m_pRenderers[IRenderer::eCopy] = std::make_unique<CopyRenderer>();
			}

			RenderManager::Impl::~Impl()
			{
			}

			void RenderManager::Impl::AllCleanup()
			{
				TRACER_EVENT(__FUNCTIONW__);
				GetModelRenderer()->AllCleanup();
				GetTerrainRenderer()->AllCleanup();
				GetVertexRenderer()->AllCleanup();
			}

			void RenderManager::Impl::Cleanup()
			{
				TRACER_EVENT(__FUNCTIONW__);
				GetModelRenderer()->Cleanup();
				GetTerrainRenderer()->Cleanup();
				GetVertexRenderer()->Cleanup();
			}

			void RenderManager::Impl::Render()
			{
				TRACER_EVENT(__FUNCTIONW__);
				Device* pDeviceInstance = Device::GetInstance();
				const uint32_t frameIndex = pDeviceInstance->GetFrameIndex();

				Camera& camera = RenderCamera();
				const math::Matrix matViewProjection = camera.GetViewMatrix() * camera.GetProjectionMatrix();

				const Options& options = RenderOptions();

				UpdateOptions(options);

				RenderElement renderElement;
				renderElement.pCamera = &camera;

				RenderTarget* pLastUsedRenderTarget{ nullptr };

				GBuffer* pGBuffer = pDeviceInstance->GetGBuffer(frameIndex);
				{
					TRACER_EVENT(L"ClearBuffer");
					RenderTarget* pNormalsRT = pGBuffer->GetRenderTarget(GBufferType::eNormals);
					RenderTarget* pColorsRT = pGBuffer->GetRenderTarget(GBufferType::eColors);
					RenderTarget* pDisneyBRDFRT = pGBuffer->GetRenderTarget(GBufferType::eDisneyBRDF);
					RenderTarget* pVelocity = pGBuffer->GetRenderTarget(GBufferType::eVelocity);
					DepthStencil* pDepthStencil = pGBuffer->GetDepthStencil();

					ID3D12GraphicsCommandList2* pCommandList = pDeviceInstance->GetCommandList(0);
					pDeviceInstance->ResetCommandList(0, nullptr);

					util::ChangeResourceState(pCommandList, pNormalsRT, D3D12_RESOURCE_STATE_RENDER_TARGET);
					util::ChangeResourceState(pCommandList, pColorsRT, D3D12_RESOURCE_STATE_RENDER_TARGET);
					util::ChangeResourceState(pCommandList, pDisneyBRDFRT, D3D12_RESOURCE_STATE_RENDER_TARGET);

					if (pVelocity != nullptr)
					{
						util::ChangeResourceState(pCommandList, pVelocity, D3D12_RESOURCE_STATE_RENDER_TARGET);
					}
					util::ChangeResourceState(pCommandList, pDepthStencil, D3D12_RESOURCE_STATE_DEPTH_WRITE);

					pGBuffer->Clear(pCommandList);

					D3D12_RESOURCE_DESC desc = pDeviceInstance->GetSwapChainRenderTarget(frameIndex)->GetDesc();
					if (options.OnHDR == true)
					{
						desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
					}

					pLastUsedRenderTarget = pDeviceInstance->GetRenderTarget(&desc, math::Color::Transparent);
					if (pLastUsedRenderTarget == nullptr)
					{
						throw_line("failed to get render target");
					}

					util::ChangeResourceState(pCommandList, pLastUsedRenderTarget, D3D12_RESOURCE_STATE_RENDER_TARGET);
					pLastUsedRenderTarget->Clear(pCommandList);

					pCommandList->Close();
					pDeviceInstance->ExecuteCommandList(pCommandList);

					renderElement.pRTVs[0] = pLastUsedRenderTarget;
					renderElement.rtvHandles[0] = pLastUsedRenderTarget->GetCPUHandle();
					renderElement.rtvCount = 1;
					renderElement.pDepthStencil = pGBuffer->GetDepthStencil();
				}

				GetEnvironmentRenderer()->Render(renderElement);

				{
					renderElement.pRTVs[GBufferType::eNormals] = pGBuffer->GetRenderTarget(GBufferType::eNormals);
					renderElement.pRTVs[GBufferType::eColors] = pGBuffer->GetRenderTarget(GBufferType::eColors);
					renderElement.pRTVs[GBufferType::eDisneyBRDF] = pGBuffer->GetRenderTarget(GBufferType::eDisneyBRDF);

					renderElement.rtvHandles[GBufferType::eNormals] = pGBuffer->GetRenderTarget(GBufferType::eNormals)->GetCPUHandle();
					renderElement.rtvHandles[GBufferType::eColors] = pGBuffer->GetRenderTarget(GBufferType::eColors)->GetCPUHandle();
					renderElement.rtvHandles[GBufferType::eDisneyBRDF] = pGBuffer->GetRenderTarget(GBufferType::eDisneyBRDF)->GetCPUHandle();

					renderElement.SetDSVHandle(pGBuffer->GetDepthStencil()->GetCPUHandle());

					if (options.OnMotionBlur == true && options.motionBlurConfig.IsVelocityMotionBlur() == true)
					{
						renderElement.pRTVs[GBufferType::eVelocity] = pGBuffer->GetRenderTarget(GBufferType::eVelocity);
						renderElement.rtvHandles[GBufferType::eVelocity] = pGBuffer->GetRenderTarget(GBufferType::eVelocity)->GetCPUHandle();
						renderElement.rtvCount = 4;
					}
					else
					{
						renderElement.rtvCount = 3;
					}
					GetTerrainRenderer()->Render(renderElement, m_matPrevViewProjection);

					GetModelRenderer()->Render(renderElement, ModelRenderer::eDeferred, m_matPrevViewProjection);
				}

				{
					renderElement.pRTVs[0] = pLastUsedRenderTarget;
					renderElement.pRTVs[1] = nullptr;
					renderElement.pRTVs[2] = nullptr;
					renderElement.pRTVs[3] = nullptr;
					renderElement.rtvHandles[0] = pLastUsedRenderTarget->GetCPUHandle();
					renderElement.rtvHandles[1] = {};
					renderElement.rtvHandles[2] = {};
					renderElement.rtvHandles[3] = {};
					renderElement.rtvCount = 1;
					renderElement.ResetDSVHandle();

					GetDeferredRenderer()->Render(renderElement);
				}

				// PostProcessing
				{
					if (options.OnSSS == true)
					{
						TRACER_EVENT(L"SSS");
						D3D12_RESOURCE_DESC swapchainDesc = pDeviceInstance->GetSwapChainRenderTarget(frameIndex)->GetDesc();
						if (options.OnHDR == true)
						{
							swapchainDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
						}

						RenderTarget* pSSS = pDeviceInstance->GetRenderTarget(&swapchainDesc, math::Color::Transparent);

						RenderTarget* pSource = pLastUsedRenderTarget;
						DepthStencil* pDepth = pGBuffer->GetDepthStencil();

						GetSSS()->Apply(pSource, pDepth, pSSS);

						pDeviceInstance->ReleaseRenderTargets(&pLastUsedRenderTarget);
						pLastUsedRenderTarget = pSSS;
					}

					if (options.OnASSAO == true)
					{
						TRACER_EVENT(L"ASSAO");
						const RenderTarget* pNormalMap = pGBuffer->GetRenderTarget(GBufferType::eNormals);
						const DepthStencil* pDepth = pGBuffer->GetDepthStencil();

						GetAssao()->Apply(&camera, pNormalMap, pDepth, pLastUsedRenderTarget);
					}
				}

				{
					renderElement.pRTVs[0] = pLastUsedRenderTarget;
					renderElement.rtvHandles[0] = pLastUsedRenderTarget->GetCPUHandle();
					renderElement.rtvCount = 1;
					renderElement.SetDSVHandle(pGBuffer->GetDepthStencil()->GetCPUHandle());
					GetModelRenderer()->Render(renderElement, ModelRenderer::eAlphaBlend, m_matPrevViewProjection);
				}

				{
					renderElement.ResetDSVHandle();
					GetVertexRenderer()->Render(renderElement);
				}

				{
					const D3D12_RESOURCE_DESC swapchainDesc = pDeviceInstance->GetSwapChainRenderTarget(frameIndex)->GetDesc();

					if (options.OnHDR == true)
					{
						TRACER_EVENT(L"HDR");
						RenderTarget* pHDR = pDeviceInstance->GetRenderTarget(&swapchainDesc, math::Color::Transparent);
						RenderTarget* pSource = pLastUsedRenderTarget;
						GetHDRFilter()->Apply(pSource, pHDR);

						pDeviceInstance->ReleaseRenderTargets(&pLastUsedRenderTarget);
						pLastUsedRenderTarget = pHDR;
					}

					if (options.OnBloomFilter == true)
					{
						TRACER_EVENT(L"BloomFilter");
						GetBloomFilter()->Apply(pLastUsedRenderTarget);
					}

					if (options.OnColorGrading == true)
					{
						TRACER_EVENT(L"ColorGrading");
						RenderTarget* pColorGrading = pDeviceInstance->GetRenderTarget(&swapchainDesc, math::Color::Transparent);
						RenderTarget* pSource = pLastUsedRenderTarget;
						GetColorGrading()->Apply(&camera, pSource, pColorGrading);

						pDeviceInstance->ReleaseRenderTargets(&pLastUsedRenderTarget);
						pLastUsedRenderTarget = pColorGrading;
					}

					if (options.OnDOF == true)
					{
						TRACER_EVENT(L"DOF");
						RenderTarget* pDepthOfField = pDeviceInstance->GetRenderTarget(&swapchainDesc, math::Color::Transparent);
						RenderTarget* pSource = pLastUsedRenderTarget;
						DepthStencil* pDepth = pGBuffer->GetDepthStencil();

						GetDepthOfField()->Apply(&camera, pSource, pDepth, pDepthOfField);

						pDeviceInstance->ReleaseRenderTargets(&pLastUsedRenderTarget);
						pLastUsedRenderTarget = pDepthOfField;
					}

					if (options.OnFXAA == true)
					{
						TRACER_EVENT(L"FXAA");
						RenderTarget* pFxaa = pDeviceInstance->GetRenderTarget(&swapchainDesc, math::Color::Transparent);
						RenderTarget* pSource = pLastUsedRenderTarget;

						GetFxaa()->Apply(pSource, pFxaa);

						pDeviceInstance->ReleaseRenderTargets(&pLastUsedRenderTarget);
						pLastUsedRenderTarget = pFxaa;
					}
					if (options.OnMotionBlur == true)
					{
						RenderTarget* pMotionBlur = pDeviceInstance->GetRenderTarget(&swapchainDesc, math::Color::Transparent);
						RenderTarget* pSource = pLastUsedRenderTarget;

						switch (options.motionBlurConfig.emMode)
						{
						case Options::MotionBlurConfig::eDepthBuffer_4Samples:
						case Options::MotionBlurConfig::eDepthBuffer_8Samples:
						case Options::MotionBlurConfig::eDepthBuffer_12Samples:
						{
							DepthStencil* pDepth = pGBuffer->GetDepthStencil();
							GetMotionBlur()->Apply(&camera, m_matPrevViewProjection, pSource, pDepth, pMotionBlur);
						}
						break;
						case Options::MotionBlurConfig::eVelocityBuffer_4Samples:
						case Options::MotionBlurConfig::eVelocityBuffer_8Samples:
						case Options::MotionBlurConfig::eVelocityBuffer_12Samples:
						{
							RenderTarget* pVelocity = pGBuffer->GetRenderTarget(GBufferType::eVelocity);
							GetMotionBlur()->Apply(&camera, pSource, pVelocity, pMotionBlur);
						}
						break;
						case Options::MotionBlurConfig::eDualVelocityBuffer_4Samples:
						case Options::MotionBlurConfig::eDualVelocityBuffer_8Samples:
						case Options::MotionBlurConfig::eDualVelocityBuffer_12Samples:
						{
							RenderTarget* pVelocity = pGBuffer->GetRenderTarget(GBufferType::eVelocity);
							RenderTarget* pPrevVelocity = pGBuffer->GetPrevVelocityBuffer();
							GetMotionBlur()->Apply(&camera, pSource, pVelocity, pPrevVelocity, pMotionBlur);
						}
						break;
						default:
							throw_line("unknown motion blur mode");
						}

						pDeviceInstance->ReleaseRenderTargets(&pLastUsedRenderTarget);
						pLastUsedRenderTarget = pMotionBlur;
					}
				}

				// copy to swapchain
				{
					TRACER_EVENT(L"CopyToSwapchain");
					RenderTarget* pSwapChainRenderTarget = pDeviceInstance->GetSwapChainRenderTarget(frameIndex);
					Copy_RGB(pLastUsedRenderTarget, pSwapChainRenderTarget);

					//RenderTarget* pSwapChainRenderTarget = pDeviceInstance->GetSwapChainRenderTarget(frameIndex);
					//
					//ID3D12GraphicsCommandList2* pCommandList = pDeviceInstance->GetCommandList(0);
					//pDeviceInstance->ResetCommandList(0, nullptr);
					//
					//util::ChangeResourceState(pCommandList, pSwapChainRenderTarget, D3D12_RESOURCE_STATE_COPY_DEST);
					//util::ChangeResourceState(pCommandList, pLastUsedRenderTarget, D3D12_RESOURCE_STATE_COPY_SOURCE);
					//
					//pCommandList->CopyResource(pSwapChainRenderTarget->GetResource(), pLastUsedRenderTarget->GetResource());
					//
					//util::ChangeResourceState(pCommandList, pSwapChainRenderTarget, D3D12_RESOURCE_STATE_PRESENT);
					//util::ChangeResourceState(pCommandList, pLastUsedRenderTarget, D3D12_RESOURCE_STATE_RENDER_TARGET);
					//
					//HRESULT hr = pCommandList->Close();
					//if (FAILED(hr))
					//{
					//	throw_line("failed to close command list");
					//}
					//
					//pDeviceInstance->ExecuteCommandList(pCommandList);
				}
				pDeviceInstance->ReleaseRenderTargets(&pLastUsedRenderTarget);

				m_matPrevViewProjection = matViewProjection;
			}

			void RenderManager::Impl::Copy_RGBA(RenderTarget* pSource, RenderTarget* pResult)
			{
				GetCopy()->Copy_RGBA(pSource, pResult);
			}

			void RenderManager::Impl::Copy_RGB(RenderTarget* pSource, RenderTarget* pResult)
			{
				GetCopy()->Copy_RGB(pSource, pResult);
			}

			void RenderManager::Impl::UpdateOptions(const Options& curOptions)
			{
				const Options& prevOptions = PrevRenderOptions();
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

				if (prevOptions.OnMotionBlur != curOptions.OnMotionBlur)
				{
					if (curOptions.OnMotionBlur == true)
					{
						m_pRenderers[IRenderer::eMotionBlur] = std::make_unique<MotionBlur>();
					}
					else
					{
						m_pRenderers[IRenderer::eMotionBlur].reset();
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

			void RenderManager::AllCleanup()
			{
				m_pImpl->AllCleanup();
			}

			void RenderManager::Cleanup()
			{
				m_pImpl->Cleanup();
			}

			void RenderManager::Render()
			{
				m_pImpl->Render();
			}

			void RenderManager::Copy_RGBA(RenderTarget* pSource, RenderTarget* pResult)
			{
				m_pImpl->Copy_RGBA(pSource, pResult);
			}

			void RenderManager::Copy_RGB(RenderTarget* pSource, RenderTarget* pResult)
			{
				m_pImpl->Copy_RGB(pSource, pResult);
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