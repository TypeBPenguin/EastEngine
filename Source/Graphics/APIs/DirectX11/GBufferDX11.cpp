#include "stdafx.h"
#include "GBufferDX11.h"

#include "DeviceDX11.h"
#include "Graphics/Interface/ParallelUpdateRender.h"

namespace est
{
	namespace graphics
	{
		namespace dx11
		{
			GBuffer::GBuffer(uint32_t width, uint32_t height)
			{
				Resize(width, height);
			}

			GBuffer::~GBuffer()
			{
				Release();
			}

			void GBuffer::Resize(uint32_t width, uint32_t height)
			{
				Release();

				D3D11_TEXTURE2D_DESC desc{};
				desc.Width = width;
				desc.Height = height;
				desc.ArraySize = 1;
				desc.MipLevels = 0;
				desc.SampleDesc.Count = 1;
				desc.Usage = D3D11_USAGE_DEFAULT;
				desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
				desc.CPUAccessFlags = 0;
				desc.MiscFlags = 0;

				{
					desc.Format = static_cast<DXGI_FORMAT>(GBufferFormat(GBufferType::eNormals));
					m_pRenderTargets[GBufferType::eNormals] = RenderTarget::Create(&desc);
					if (m_pRenderTargets[GBufferType::eNormals] == nullptr)
					{
						throw_line("failed to create normal GBuffer");
					}
				}

				{
					desc.Format = static_cast<DXGI_FORMAT>(GBufferFormat(GBufferType::eColors));
					m_pRenderTargets[GBufferType::eColors] = RenderTarget::Create(&desc);
					if (m_pRenderTargets[GBufferType::eColors] == nullptr)
					{
						throw_line("failed to create color GBuffer");
					}
				}

				{
					desc.Format = static_cast<DXGI_FORMAT>(GBufferFormat(GBufferType::eDisneyBRDF));
					m_pRenderTargets[GBufferType::eDisneyBRDF] = RenderTarget::Create(&desc);
					if (m_pRenderTargets[GBufferType::eDisneyBRDF] == nullptr)
					{
						throw_line("failed to create disneyBRDF GBuffer");
					}
				}

				const Options& options = RenderOptions();
				if (options.OnMotionBlur == true && options.motionBlurConfig.IsVelocityMotionBlur() == true)
				{
					desc.Format = static_cast<DXGI_FORMAT>(GBufferFormat(GBufferType::eVelocity));
					m_pRenderTargets[GBufferType::eVelocity] = RenderTarget::Create(&desc);
					if (m_pRenderTargets[GBufferType::eVelocity] == nullptr)
					{
						throw_line("failed to create velocity GBuffer");
					}

					m_pPrevVelocityBuffer = RenderTarget::Create(&desc);
					if (m_pPrevVelocityBuffer == nullptr)
					{
						throw_line("failed to create velocity GBuffer");
					}
				}

				desc.Format = DXGI_FORMAT_R32_TYPELESS;
				desc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;

				m_pDepthStencil = DepthStencil::Create(&desc);
				if (m_pDepthStencil == nullptr)
				{
					throw_line("failed to create depth stencil GBuffer");
				}
			}

			void GBuffer::Resize(GBufferType emType, uint32_t width, uint32_t height)
			{
				Release(emType);

				D3D11_TEXTURE2D_DESC desc{};
				desc.Width = width;
				desc.Height = height;
				desc.ArraySize = 1;
				desc.MipLevels = 0;
				desc.SampleDesc.Count = 1;
				desc.Usage = D3D11_USAGE_DEFAULT;
				desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
				desc.CPUAccessFlags = 0;
				desc.MiscFlags = 0;

				switch (emType)
				{
				case GBufferType::eNormals:
				{
					desc.Format = static_cast<DXGI_FORMAT>(GBufferFormat(GBufferType::eNormals));
					m_pRenderTargets[GBufferType::eNormals] = RenderTarget::Create(&desc);
					if (m_pRenderTargets[GBufferType::eNormals] == nullptr)
					{
						throw_line("failed to create normal GBuffer");
					}
				}
				break;
				case GBufferType::eColors:
				{
					desc.Format = static_cast<DXGI_FORMAT>(GBufferFormat(GBufferType::eColors));
					m_pRenderTargets[GBufferType::eColors] = RenderTarget::Create(&desc);
					if (m_pRenderTargets[GBufferType::eColors] == nullptr)
					{
						throw_line("failed to create color GBuffer");
					}
				}
				break;
				case GBufferType::eDisneyBRDF:
				{
					desc.Format = static_cast<DXGI_FORMAT>(GBufferFormat(GBufferType::eDisneyBRDF));
					m_pRenderTargets[GBufferType::eDisneyBRDF] = RenderTarget::Create(&desc);
					if (m_pRenderTargets[GBufferType::eDisneyBRDF] == nullptr)
					{
						throw_line("failed to create disneyBRDF GBuffer");
					}
				}
				break;
				case GBufferType::eVelocity:
				{
					desc.Format = static_cast<DXGI_FORMAT>(GBufferFormat(GBufferType::eVelocity));
					m_pRenderTargets[GBufferType::eVelocity] = RenderTarget::Create(&desc);
					if (m_pRenderTargets[GBufferType::eVelocity] == nullptr)
					{
						throw_line("failed to create velocity GBuffer");
					}

					m_pPrevVelocityBuffer = RenderTarget::Create(&desc);
					if (m_pPrevVelocityBuffer == nullptr)
					{
						throw_line("failed to create velocity GBuffer");
					}
				}
				break;
				}
			}

			void GBuffer::Release()
			{
				for (auto& pRenderTarget : m_pRenderTargets)
				{
					pRenderTarget.reset();
				}
				m_pPrevVelocityBuffer.reset();

				m_pDepthStencil.reset();
			}

			void GBuffer::Release(GBufferType emType)
			{
				m_pRenderTargets[emType].reset();

				if (emType == GBufferType::eVelocity)
				{
					m_pPrevVelocityBuffer.reset();
				}
			}

			void GBuffer::Cleanup()
			{
				std::swap(m_pRenderTargets[GBufferType::eVelocity], m_pPrevVelocityBuffer);
			}

			void GBuffer::Clear(ID3D11DeviceContext* pImmediateContext)
			{
				for (auto& pRenderTarget : m_pRenderTargets)
				{
					if (pRenderTarget != nullptr)
					{
						pImmediateContext->ClearRenderTargetView(pRenderTarget->GetRenderTargetView(), math::Color::Transparent);
					}
				}
				pImmediateContext->ClearDepthStencilView(m_pDepthStencil->GetDepthStencilView(), D3D11_CLEAR_DEPTH, 1.f, 0);
			}
		}
	}
}