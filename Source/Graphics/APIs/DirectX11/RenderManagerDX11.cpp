#include "stdafx.h"
#include "RenderManagerDX11.h"

#include "Graphics/Interface/Camera.h"
#include "Graphics/Interface/LightManager.h"

#include "UtilDX11.h"
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
#include "SSRDX11.h"
#include "HDRFilterDX11.h"
#include "VertexRendererDX11.h"
#include "MotionBlurDX11.h"
#include "CopyRendererDX11.h"

namespace est
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
				void Copy_RGBA(const RenderTarget* pSource, RenderTarget* pResult);
				void Copy_RGB(const RenderTarget* pSource, RenderTarget* pResult);

			public:
				void PushJob(const RenderJobStatic& renderJob) { GetModelRenderer()->PushJob(renderJob); }
				void PushJob(const RenderJobSkinned& renderJob) { GetModelRenderer()->PushJob(renderJob); }
				void PushJob(const RenderJobTerrain& renderJob) { GetTerrainRenderer()->PushJob(renderJob); }
				void PushJob(const RenderJobVertex& renderJob) { GetVertexRenderer()->PushJob(renderJob); }

			public:
				IRenderer* GetRenderer(IRenderer::Type emType) const { return m_pRenderers[emType].get(); }

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
				SSR* GetSSR() const { return static_cast<SSR*>(m_pRenderers[IRenderer::eSSR].get()); }
				HDRFilter* GetHDRFilter() const { return static_cast<HDRFilter*>(m_pRenderers[IRenderer::eHDR].get()); }
				MotionBlur* GetMotionBlur() const { return static_cast<MotionBlur*>(m_pRenderers[IRenderer::eMotionBlur].get()); }
				CopyRenderer* GetCopy() const { return static_cast<CopyRenderer*>(m_pRenderers[IRenderer::eCopy].get()); }

			private:
				std::array<std::unique_ptr<IRenderer>, IRenderer::TypeCount> m_pRenderers{ nullptr };

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
				TRACER_EVENT(__FUNCTIONW__);
				Device* pDeviceInstance = Device::GetInstance();
				ID3D11Device* pDevice = pDeviceInstance->GetInterface();
				ID3D11DeviceContext* pImmediateContext = pDeviceInstance->GetImmediateContext();

				const GBuffer* pGBuffer = pDeviceInstance->GetGBuffer();

				Camera* pCamera = Camera::GetInstance();
				const math::Matrix matViewProjection = pCamera->GetViewMatrix() * pCamera->GetProjectionMatrix();

				const Options& options = GetOptions();

				UpdateOptions(options);

				RenderElement renderElement;
				renderElement.pDevice = pDevice;
				renderElement.pDeviceContext = pImmediateContext;
				renderElement.pCamera = pCamera;

				RenderTarget* pLastUsedRenderTarget{ nullptr };
				{
					D3D11_TEXTURE2D_DESC swapchainDesc{};
					pDeviceInstance->GetSwapChainRenderTarget()->GetDesc2D(&swapchainDesc);
					swapchainDesc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;

					if (GetOptions().OnHDR == true)
					{
						swapchainDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
					}

					pLastUsedRenderTarget = pDeviceInstance->GetRenderTarget(&swapchainDesc);
					if (pLastUsedRenderTarget == nullptr)
					{
						throw_line("failed to get render target");
					}
					pImmediateContext->ClearRenderTargetView(pLastUsedRenderTarget->GetRenderTargetView(), &math::Color::Transparent.r);

					renderElement.pRTVs[0] = pLastUsedRenderTarget->GetRenderTargetView();
					renderElement.rtvCount = 1;
				}

				GetEnvironmentRenderer()->Render(renderElement);

				{
					renderElement.pRTVs[GBufferType::eNormals] = pGBuffer->GetRenderTarget(GBufferType::eNormals)->GetRenderTargetView();
					renderElement.pRTVs[GBufferType::eColors] = pGBuffer->GetRenderTarget(GBufferType::eColors)->GetRenderTargetView();
					renderElement.pRTVs[GBufferType::eDisneyBRDF] = pGBuffer->GetRenderTarget(GBufferType::eDisneyBRDF)->GetRenderTargetView();

					renderElement.pDSV = pGBuffer->GetDepthStencil()->GetDepthStencilView();

					if (GetOptions().OnMotionBlur == true && GetOptions().motionBlurConfig.IsVelocityMotionBlur() == true)
					{
						renderElement.pRTVs[GBufferType::eVelocity] = pGBuffer->GetRenderTarget(GBufferType::eVelocity)->GetRenderTargetView();
						renderElement.rtvCount = 4;
					}
					else
					{
						renderElement.rtvCount = 3;
					}
					GetTerrainRenderer()->Render(renderElement, TerrainRenderer::eDeferred, m_matPrevViewProjection);
					GetTerrainRenderer()->Render(renderElement, TerrainRenderer::eShadow, math::Matrix::Identity);

					GetModelRenderer()->Render(renderElement, ModelRenderer::eDeferred, m_matPrevViewProjection);
					GetModelRenderer()->Render(renderElement, ModelRenderer::eShadow, m_matPrevViewProjection);
				}

				{
					renderElement.pRTVs[0] = pLastUsedRenderTarget->GetRenderTargetView();
					renderElement.pRTVs[1] = nullptr;
					renderElement.pRTVs[2] = nullptr;
					renderElement.pRTVs[3] = nullptr;
					renderElement.rtvCount = 1;
					renderElement.pDSV = nullptr;

					GetDeferredRenderer()->Render(renderElement);
				}

				// PostProcessing
				{
					if (options.OnSSS == true)
					{
						TRACER_EVENT(L"SSS");
						D3D11_TEXTURE2D_DESC swapchainDesc{};
						pDeviceInstance->GetSwapChainRenderTarget()->GetDesc2D(&swapchainDesc);
						swapchainDesc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;

						if (GetOptions().OnHDR == true)
						{
							swapchainDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
						}

						RenderTarget* pSSS = pDeviceInstance->GetRenderTarget(&swapchainDesc);
						pImmediateContext->ClearRenderTargetView(pSSS->GetRenderTargetView(), math::Color::Transparent);

						const RenderTarget* pSource = pLastUsedRenderTarget;
						const DepthStencil* pDepth = pGBuffer->GetDepthStencil();
						GetSSS()->Apply(pSource, pDepth, pSSS);

						pDeviceInstance->ReleaseRenderTargets(&pLastUsedRenderTarget);
						pLastUsedRenderTarget = pSSS;
					}

					if (options.OnASSAO == true)
					{
						TRACER_EVENT(L"ASSAO");
						const RenderTarget* pNormalMap = pGBuffer->GetRenderTarget(GBufferType::eNormals);
						const DepthStencil* pDepth = pGBuffer->GetDepthStencil();
						GetAssao()->Apply(pCamera, pNormalMap, pDepth, pLastUsedRenderTarget);
					}
				}

				{
					renderElement.pRTVs[0] = pLastUsedRenderTarget->GetRenderTargetView();
					renderElement.rtvCount = 1;
					renderElement.pDSV = pGBuffer->GetDepthStencil()->GetDepthStencilView();
					GetModelRenderer()->Render(renderElement, ModelRenderer::eAlphaBlend, m_matPrevViewProjection);
				}

				{
					renderElement.pDSV = nullptr;
					GetVertexRenderer()->Render(renderElement);
				}

				{
					if (options.OnSSR == true)
					{
						TRACER_EVENT(L"SSR");
						D3D11_TEXTURE2D_DESC swapchainDesc{};
						pDeviceInstance->GetSwapChainRenderTarget()->GetDesc2D(&swapchainDesc);
						swapchainDesc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;

						const DXGI_FORMAT prevFormat = swapchainDesc.Format;
						if (GetOptions().OnHDR == true)
						{
							swapchainDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
						}

						RenderTarget* pSSR = pDeviceInstance->GetRenderTarget(&swapchainDesc);
						pImmediateContext->ClearRenderTargetView(pSSR->GetRenderTargetView(), math::Color::Transparent);

						const RenderTarget* pSource = pLastUsedRenderTarget;
						const DepthStencil* pDepth = pGBuffer->GetDepthStencil();
						GetSSR()->Apply(pCamera, pSource, pGBuffer, pDepth, pSSR);

						pDeviceInstance->ReleaseRenderTargets(&pLastUsedRenderTarget);
						pLastUsedRenderTarget = pSSR;

						if (GetOptions().OnHDR == true)
						{
							swapchainDesc.Format = prevFormat;
						}
					}

					D3D11_TEXTURE2D_DESC swapchainDesc{};
					pDeviceInstance->GetSwapChainRenderTarget()->GetDesc2D(&swapchainDesc);
					swapchainDesc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;

					if (options.OnHDR == true)
					{
						TRACER_EVENT(L"OnHDR");
						RenderTarget* pHDR = pDeviceInstance->GetRenderTarget(&swapchainDesc);
						const RenderTarget* pSource = pLastUsedRenderTarget;
						GetHDRFilter()->Apply(pSource, pHDR);

						pDeviceInstance->ReleaseRenderTargets(&pLastUsedRenderTarget);
						pLastUsedRenderTarget = pHDR;
					}

					if (options.OnBloomFilter == true)
					{
						TRACER_EVENT(L"OnBloomFilter");
						GetBloomFilter()->Apply(pLastUsedRenderTarget);
					}

					if (options.OnColorGrading == true)
					{
						TRACER_EVENT(L"OnColorGrading");
						RenderTarget* pColorGrading = pDeviceInstance->GetRenderTarget(&swapchainDesc);
						const RenderTarget* pSource = pLastUsedRenderTarget;
						GetColorGrading()->Apply(pCamera, pSource, pColorGrading);

						pDeviceInstance->ReleaseRenderTargets(&pLastUsedRenderTarget);
						pLastUsedRenderTarget = pColorGrading;
					}

					if (options.OnDOF == true)
					{
						TRACER_EVENT(L"OnDOF");
						RenderTarget* pDepthOfField = pDeviceInstance->GetRenderTarget(&swapchainDesc);
						const RenderTarget* pSource = pLastUsedRenderTarget;
						const DepthStencil* pDepth = pGBuffer->GetDepthStencil();

						GetDepthOfField()->Apply(pCamera, pSource, pDepth, pDepthOfField);

						pDeviceInstance->ReleaseRenderTargets(&pLastUsedRenderTarget);
						pLastUsedRenderTarget = pDepthOfField;
					}

					if (options.OnFXAA == true)
					{
						TRACER_EVENT(L"OnFXAA");
						RenderTarget* pFxaa = pDeviceInstance->GetRenderTarget(&swapchainDesc);
						const RenderTarget* pSource = pLastUsedRenderTarget;

						GetFxaa()->Apply(pSource, pFxaa);

						pDeviceInstance->ReleaseRenderTargets(&pLastUsedRenderTarget);
						pLastUsedRenderTarget = pFxaa;
					}

					if (options.OnMotionBlur == true)
					{
						TRACER_EVENT(L"OnMotionBlur");
						RenderTarget* pMotionBlur = pDeviceInstance->GetRenderTarget(&swapchainDesc);
						const RenderTarget* pSource = pLastUsedRenderTarget;

						switch (options.motionBlurConfig.emMode)
						{
						case Options::MotionBlurConfig::eDepthBuffer_4Samples:
						case Options::MotionBlurConfig::eDepthBuffer_8Samples:
						case Options::MotionBlurConfig::eDepthBuffer_12Samples:
						{
							const DepthStencil* pDepth = pGBuffer->GetDepthStencil();
							GetMotionBlur()->Apply(pCamera, m_matPrevViewProjection, pSource, pDepth, pMotionBlur);
						}
						break;
						case Options::MotionBlurConfig::eVelocityBuffer_4Samples:
						case Options::MotionBlurConfig::eVelocityBuffer_8Samples:
						case Options::MotionBlurConfig::eVelocityBuffer_12Samples:
						{
							const RenderTarget* pVelocity = pGBuffer->GetRenderTarget(GBufferType::eVelocity);
							GetMotionBlur()->Apply(pCamera, pSource, pVelocity, pMotionBlur);
						}
						break;
						case Options::MotionBlurConfig::eDualVelocityBuffer_4Samples:
						case Options::MotionBlurConfig::eDualVelocityBuffer_8Samples:
						case Options::MotionBlurConfig::eDualVelocityBuffer_12Samples:
						{
							const RenderTarget* pVelocity = pGBuffer->GetRenderTarget(GBufferType::eVelocity);
							const RenderTarget* pPrevVelocity = pGBuffer->GetPrevVelocityBuffer();
							GetMotionBlur()->Apply(pCamera, pSource, pVelocity, pPrevVelocity, pMotionBlur);
						}
						break;
						default:
							throw_line("unknown motion blur mode");
						}

						pDeviceInstance->ReleaseRenderTargets(&pLastUsedRenderTarget);
						pLastUsedRenderTarget = pMotionBlur;
					}
				}

				{
					TRACER_EVENT(L"CopyToSwapChain");
					DX_PROFILING(CopyToSwapChain);
					RenderTarget* pSwapChainRenderTarget = pDeviceInstance->GetSwapChainRenderTarget();
					Copy_RGB(pLastUsedRenderTarget, pSwapChainRenderTarget);
				}
				pDeviceInstance->ReleaseRenderTargets(&pLastUsedRenderTarget);

				m_matPrevViewProjection = matViewProjection;
			}

			void RenderManager::Impl::Copy_RGBA(const RenderTarget* pSource, RenderTarget* pResult)
			{
				GetCopy()->Copy_RGBA(pSource, pResult);
			}

			void RenderManager::Impl::Copy_RGB(const RenderTarget* pSource, RenderTarget* pResult)
			{
				GetCopy()->Copy_RGB(pSource, pResult);
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

				if (prevOptions.OnSSR != curOptions.OnSSR)
				{
					if (curOptions.OnSSR == true)
					{
						m_pRenderers[IRenderer::eGaussianBlur] = std::make_unique<GaussianBlur>();
						m_pRenderers[IRenderer::eSSR] = std::make_unique<SSR>();
					}
					else
					{
						m_pRenderers[IRenderer::eSSR].reset();
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

			void RenderManager::Cleanup()
			{
				m_pImpl->Cleanup();
			}

			void RenderManager::Render()
			{
				m_pImpl->Render();
			}

			void RenderManager::Copy_RGBA(const RenderTarget* pSource, RenderTarget* pResult)
			{
				m_pImpl->Copy_RGBA(pSource, pResult);
			}

			void RenderManager::Copy_RGB(const RenderTarget* pSource, RenderTarget* pResult)
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

			IRenderer* RenderManager::GetRenderer(IRenderer::Type emType) const
			{
				return m_pImpl->GetRenderer(emType);
			}
		}
	}
}