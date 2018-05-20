#include "stdafx.h"
#include "RenderManagerDX12.h"

#include "GraphicsInterface/Camera.h"

#include "DeviceDX12.h"
#include "GBufferDX12.h"

#include "VertexBufferDX12.h"
#include "IndexBufferDX12.h"
#include "TextureDX12.h"

#include "ModelRendererDX12.h"
#include "DeferredRendererDX12.h"

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
				Camera* pCamera = Camera::GetInstance();

				m_pModelRenderer->Render(pCamera, ModelRenderer::eDeferred);
				m_pDeferredRenderer->Render(pCamera);

				int nFrameIndex = Device::GetInstance()->GetFrameIndex();
				RenderTarget* pSwapChainRenderTarget = Device::GetInstance()->GetSwapChainRenderTarget(nFrameIndex);
				RenderTarget* pLastUseRenderTarget = Device::GetInstance()->GetLastUseRenderTarget();

				ID3D12GraphicsCommandList2* pCommandList = Device::GetInstance()->PopCommandList(nullptr);
				{
					CD3DX12_RESOURCE_BARRIER transition[] =
					{
						CD3DX12_RESOURCE_BARRIER::Transition(pSwapChainRenderTarget->GetResource(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_COPY_DEST),
						CD3DX12_RESOURCE_BARRIER::Transition(pLastUseRenderTarget->GetResource(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_SOURCE),
					};
					pCommandList->ResourceBarrier(_countof(transition), transition);
				}

				pCommandList->CopyResource(pSwapChainRenderTarget->GetResource(), pLastUseRenderTarget->GetResource());

				{
					CD3DX12_RESOURCE_BARRIER transition[] =
					{
						CD3DX12_RESOURCE_BARRIER::Transition(pSwapChainRenderTarget->GetResource(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PRESENT),
						CD3DX12_RESOURCE_BARRIER::Transition(pLastUseRenderTarget->GetResource(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET),
					};
					pCommandList->ResourceBarrier(_countof(transition), transition);
				}
				Device::GetInstance()->PushCommandList(pCommandList);
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