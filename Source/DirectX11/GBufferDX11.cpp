#include "stdafx.h"
#include "GBufferDX11.h"

#include "DeviceDX11.h"

namespace eastengine
{
	namespace graphics
	{
		namespace dx11
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

				D3D11_TEXTURE2D_DESC desc{};
				desc.Width = nWidth;
				desc.Height = nHeight;
				desc.ArraySize = 1;
				desc.MipLevels = 0;
				desc.SampleDesc.Count = 1;
				desc.Usage = D3D11_USAGE_DEFAULT;
				desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
				desc.CPUAccessFlags = 0;
				desc.MiscFlags = 0;

				{
					desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
					m_pRenderTargets[EmGBuffer::eNormals] = RenderTarget::Create(&desc);
					if (m_pRenderTargets[EmGBuffer::eNormals] == nullptr)
					{
						throw_line("failed to create normal GBuffer");
					}
				}

				{
					desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
					m_pRenderTargets[EmGBuffer::eColors] = RenderTarget::Create(&desc);
					if (m_pRenderTargets[EmGBuffer::eColors] == nullptr)
					{
						throw_line("failed to create color GBuffer");
					}
				}

				{
					desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
					m_pRenderTargets[EmGBuffer::eDisneyBRDF] = RenderTarget::Create(&desc);
					if (m_pRenderTargets[EmGBuffer::eDisneyBRDF] == nullptr)
					{
						throw_line("failed to create disneyBRDF GBuffer");
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

			void GBuffer::Release()
			{
				for (auto& pRenderTarget : m_pRenderTargets)
				{
					pRenderTarget.reset();
				}

				m_pDepthStencil.reset();
			}

			void GBuffer::Clear(ID3D11DeviceContext* pImmediateContext)
			{
				for (uint32_t i = 0; i < EmGBuffer::Count; ++i)
				{
					pImmediateContext->ClearRenderTargetView(m_pRenderTargets[i]->GetRenderTargetView(), math::Color::Transparent);
				}

				pImmediateContext->ClearDepthStencilView(m_pDepthStencil->GetDepthStencilView(), D3D11_CLEAR_DEPTH, 1.f, 0);
			}
		}
	}
}