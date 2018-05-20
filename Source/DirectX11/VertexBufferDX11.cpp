#include "stdafx.h"
#include "VertexBufferDX11.h"

#include "DeviceDX11.h"

namespace eastengine
{
	namespace graphics
	{
		namespace dx11
		{
			VertexBuffer::VertexBuffer(const uint8_t* pData, size_t nBufferSize, uint32_t nVertexCount)
				: m_nVertexCount(nVertexCount)
				, m_nFormatSize(static_cast<uint32_t>(nBufferSize / nVertexCount))
			{
				TRACER_EVENT("VertexBuffer_Init");
				SetState(IResource::eReady);

				ID3D11Device* pDevice = Device::GetInstance()->GetInterface();

				HRESULT hr = S_OK;

				D3D11_BUFFER_DESC bufferDesc{};
				bufferDesc.ByteWidth = static_cast<uint32_t>(nBufferSize);
				bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
				bufferDesc.Usage = D3D11_USAGE_DEFAULT;
				bufferDesc.CPUAccessFlags = 0;
				bufferDesc.MiscFlags = 0;

				D3D11_SUBRESOURCE_DATA vertexData{};
				vertexData.pSysMem = pData;

				hr = pDevice->CreateBuffer(&bufferDesc, &vertexData, &m_pBuffer);
				if (FAILED(hr))
				{
					SetState(IResource::eInvalid);
					throw_line("failed to create vertex buffer");
				}

				SetState(IResource::eComplete);
			}

			VertexBuffer::~VertexBuffer()
			{
				SafeRelease(m_pBuffer);
			}

			bool VertexBuffer::Map(void** ppData)
			{
				if (ppData == nullptr)
					return false;

				D3D11_MAPPED_SUBRESOURCE map{};

				ID3D11DeviceContext* pDeviceContext = Device::GetInstance()->GetImmediateContext();
				HRESULT hr = pDeviceContext->Map(m_pBuffer, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &map);
				if (FAILED(hr))
				{
					(*ppData) = nullptr;
					return false;
				}

				*ppData = map.pData;
				return true;
			}

			void VertexBuffer::Unmap()
			{
				ID3D11DeviceContext* pDeviceContext = Device::GetInstance()->GetImmediateContext();
				pDeviceContext->Unmap(m_pBuffer, 0);
			}
		}
	}
}