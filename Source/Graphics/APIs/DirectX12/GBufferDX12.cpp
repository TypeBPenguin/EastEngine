#include "stdafx.h"
#include "GBufferDX12.h"

#include "DeviceDX12.h"

namespace est
{
	namespace graphics
	{
		namespace dx12
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

				CD3DX12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R16G16B16A16_FLOAT, width, height, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);
				
				{
					desc.Format = static_cast<DXGI_FORMAT>(GBufferFormat(GBufferType::eNormals));
					m_pRenderTargets[GBufferType::eNormals] = RenderTarget::Create(&desc, math::Color::Transparent, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
					if (m_pRenderTargets[GBufferType::eNormals] == nullptr)
					{
						throw_line("failed to create normal GBuffer");
					}
				}

				{
					desc.Format = static_cast<DXGI_FORMAT>(GBufferFormat(GBufferType::eColors));
					m_pRenderTargets[GBufferType::eColors] = RenderTarget::Create(&desc, math::Color::Transparent, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
					if (m_pRenderTargets[GBufferType::eColors] == nullptr)
					{
						throw_line("failed to create color GBuffer");
					}
				}

				{
					desc.Format = static_cast<DXGI_FORMAT>(GBufferFormat(GBufferType::eDisneyBRDF));
					m_pRenderTargets[GBufferType::eDisneyBRDF] = RenderTarget::Create(&desc, math::Color::Transparent, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
					if (m_pRenderTargets[GBufferType::eDisneyBRDF] == nullptr)
					{
						throw_line("failed to create disneyBRDF GBuffer");
					}
				}

				const Options& options = GetOptions();
				if (options.OnMotionBlur == true && options.motionBlurConfig.IsVelocityMotionBlur() == true)
				{
					desc.Format = static_cast<DXGI_FORMAT>(GBufferFormat(GBufferType::eVelocity));
					m_pRenderTargets[GBufferType::eVelocity] = RenderTarget::Create(&desc, math::Color::Transparent, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
					if (m_pRenderTargets[GBufferType::eVelocity] == nullptr)
					{
						throw_line("failed to create velocity GBuffer");
					}

					m_pPrevVelocityBuffer = RenderTarget::Create(&desc, math::Color::Transparent, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
					if (m_pPrevVelocityBuffer == nullptr)
					{
						throw_line("failed to create velocity GBuffer");
					}
				}

				desc.Format = DXGI_FORMAT_D32_FLOAT;
				desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

				m_pDepthStencil = DepthStencil::Create(&desc, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
				if (m_pDepthStencil == nullptr)
				{
					throw_line("failed to create depth stencil GBuffer");
				}
			}

			void GBuffer::Resize(GBufferType emType, uint32_t width, uint32_t height)
			{
				Release(emType);

				CD3DX12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R16G16B16A16_FLOAT, width, height, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);

				switch (emType)
				{
				case GBufferType::eNormals:
				{
					desc.Format = static_cast<DXGI_FORMAT>(GBufferFormat(GBufferType::eNormals));
					m_pRenderTargets[GBufferType::eNormals] = RenderTarget::Create(&desc, math::Color::Transparent, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
					if (m_pRenderTargets[GBufferType::eNormals] == nullptr)
					{
						throw_line("failed to create normal GBuffer");
					}
				}
				break;
				case GBufferType::eColors:
				{
					desc.Format = static_cast<DXGI_FORMAT>(GBufferFormat(GBufferType::eColors));
					m_pRenderTargets[GBufferType::eColors] = RenderTarget::Create(&desc, math::Color::Transparent, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
					if (m_pRenderTargets[GBufferType::eColors] == nullptr)
					{
						throw_line("failed to create color GBuffer");
					}
				}
				break;
				case GBufferType::eDisneyBRDF:
				{
					desc.Format = static_cast<DXGI_FORMAT>(GBufferFormat(GBufferType::eDisneyBRDF));
					m_pRenderTargets[GBufferType::eDisneyBRDF] = RenderTarget::Create(&desc, math::Color::Transparent, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
					if (m_pRenderTargets[GBufferType::eDisneyBRDF] == nullptr)
					{
						throw_line("failed to create disneyBRDF GBuffer");
					}
				}
				break;
				case GBufferType::eVelocity:
				{
					desc.Format = static_cast<DXGI_FORMAT>(GBufferFormat(GBufferType::eVelocity));
					m_pRenderTargets[GBufferType::eVelocity] = RenderTarget::Create(&desc, math::Color::Transparent, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
					if (m_pRenderTargets[GBufferType::eVelocity] == nullptr)
					{
						throw_line("failed to create velocity GBuffer");
					}

					m_pPrevVelocityBuffer = RenderTarget::Create(&desc, math::Color::Transparent, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
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

			void GBuffer::Clear(ID3D12GraphicsCommandList* pCommandList)
			{
				for (auto& pRenderTarget : m_pRenderTargets)
				{
					if (pRenderTarget != nullptr)
					{
						pRenderTarget->Clear(pCommandList);
					}
				}
				m_pDepthStencil->Clear(pCommandList);
			}
		}
	}
}