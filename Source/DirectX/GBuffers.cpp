#include "stdafx.h"
#include "GBuffers.h"

#include "D3DInterface.h"

namespace eastengine
{
	namespace graphics
	{
		GBuffers::GBuffers()
		{
			m_pGBuffers.fill(nullptr);
		}

		GBuffers::~GBuffers()
		{
			for (auto& pBuffer : m_pGBuffers)
			{
				SafeDelete(pBuffer);
			}
		}

		bool GBuffers::Init(const math::Viewport& viewport)
		{
			RenderTargetDesc2D renderTargetInfo;
			renderTargetInfo.Width = static_cast<uint32_t>(viewport.width);
			renderTargetInfo.Height = static_cast<uint32_t>(viewport.height);
			renderTargetInfo.MipLevels = 1;
			renderTargetInfo.ArraySize = 1;
			renderTargetInfo.SampleDesc.Count = 1;
			renderTargetInfo.Usage = D3D11_USAGE_DEFAULT;
			renderTargetInfo.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
			renderTargetInfo.CPUAccessFlags = 0;
			renderTargetInfo.MiscFlags = 0;

			{
				renderTargetInfo.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
				renderTargetInfo.Build();

				m_pGBuffers[EmGBuffer::eNormals] = IRenderTarget::Create(renderTargetInfo);
				if (m_pGBuffers[EmGBuffer::eNormals] == nullptr)
					return false;
			}

			{
				renderTargetInfo.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
				renderTargetInfo.Build();

				m_pGBuffers[EmGBuffer::eColors] = IRenderTarget::Create(renderTargetInfo);
				if (m_pGBuffers[EmGBuffer::eColors] == nullptr)
					return false;
			}

			{
				renderTargetInfo.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
				renderTargetInfo.Build();

				m_pGBuffers[EmGBuffer::eDisneyBRDF] = IRenderTarget::Create(renderTargetInfo);
				if (m_pGBuffers[EmGBuffer::eDisneyBRDF] == nullptr)
					return false;
			}

			return true;
		}
	}
}