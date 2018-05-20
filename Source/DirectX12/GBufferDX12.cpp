#include "stdafx.h"
#include "GBufferDX12.h"

#include "DeviceDX12.h"

namespace eastengine
{
	namespace graphics
	{
		namespace dx12
		{
			GBuffer::GBuffer(uint32_t nWidth, uint32_t nHeight)
			{
				Resize(nWidth, nHeight);
			}

			GBuffer::~GBuffer()
			{
				Release();
			}

			void GBuffer::Resize(uint32_t nWidth, uint32_t nHeight)
			{
				Release();

				CD3DX12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R16G16B16A16_FLOAT, nWidth, nHeight, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);
				
				{
					desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
					m_pRenderTargets[EmGBuffer::eNormals] = RenderTarget::Create(L"GBuffer_Normals", &desc, math::Color::Transparent, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
					if (m_pRenderTargets[EmGBuffer::eNormals] == nullptr)
					{
						throw_line("failed to create normal GBuffer");
					}
				}

				{
					desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
					m_pRenderTargets[EmGBuffer::eColors] = RenderTarget::Create(L"GBuffer_Colors", &desc, math::Color::Transparent, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
					if (m_pRenderTargets[EmGBuffer::eColors] == nullptr)
					{
						throw_line("failed to create color GBuffer");
					}
				}

				{
					desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
					m_pRenderTargets[EmGBuffer::eDisneyBRDF] = RenderTarget::Create(L"GBuffer_DisneyBRDF", &desc, math::Color::Transparent, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
					if (m_pRenderTargets[EmGBuffer::eDisneyBRDF] == nullptr)
					{
						throw_line("failed to create disneyBRDF GBuffer");
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

			void GBuffer::Release()
			{
				for (auto& pRenderTarget : m_pRenderTargets)
				{
					pRenderTarget.reset();
				}

				m_pDepthStencil.reset();
			}

			void GBuffer::Clear(ID3D12GraphicsCommandList* pCommandList)
			{
				for (uint32_t i = 0; i < EmGBuffer::Count; ++i)
				{
					pCommandList->ClearRenderTargetView(m_pRenderTargets[i]->GetCPUHandle(), math::Color::Transparent, 0, nullptr);
				}

				pCommandList->ClearDepthStencilView(m_pDepthStencil->GetCPUHandle(), D3D12_CLEAR_FLAG_DEPTH, 1.f, 0, 0, nullptr);
			}
		}
	}
}