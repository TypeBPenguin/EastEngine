#include "stdafx.h"
#include "RenderManagerDX11.h"

#include "GraphicsInterface/Camera.h"

#include "DeviceDX11.h"
#include "RenderTargetDX11.h"

#include "ModelRendererDX11.h"
#include "DeferredRendererDX11.h"

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

			private:
				std::unique_ptr<ModelRenderer> m_pModelRenderer;
				std::unique_ptr<DeferredRenderer> m_pDeferredRenderer;
			};

			RenderManager::Impl::Impl()
			{
				m_pModelRenderer = std::make_unique<ModelRenderer>();
				m_pDeferredRenderer = std::make_unique<DeferredRenderer>();
			}

			RenderManager::Impl::~Impl()
			{
			}

			void RenderManager::Impl::Flush()
			{
				m_pModelRenderer->Flush();
				m_pDeferredRenderer->Flush();
			}

			void RenderManager::Impl::Render()
			{
				ID3D11Device* pDevice = Device::GetInstance()->GetInterface();
				ID3D11DeviceContext* pImmediateContext = Device::GetInstance()->GetImmediateContext();

				Camera* pCamera = Camera::GetInstance();

				m_pModelRenderer->Render(pDevice, pImmediateContext, pCamera, ModelRenderer::eDeferred);
				m_pDeferredRenderer->Render(pDevice, pImmediateContext, pCamera);

				RenderTarget* pSwapChainRenderTarget = Device::GetInstance()->GetSwapChainRenderTarget();
				RenderTarget* pLastUseRenderTarget = Device::GetInstance()->GetLastUseRenderTarget();
				pImmediateContext->CopyResource(pSwapChainRenderTarget->GetTexture2D(), pLastUseRenderTarget->GetTexture2D());
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