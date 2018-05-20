#include "stdafx.h"
#include "IndexBufferDX11.h"

#include "DeviceDX11.h"

namespace eastengine
{
	namespace graphics
	{
		namespace dx11
		{
			IndexBuffer::IndexBuffer(const uint8_t* pData, size_t nBufferSize, uint32_t nIndexCount)
				: m_nIndexCount(nIndexCount)
			{
				TRACER_EVENT("IndexBufferDX11_Init");
				SetState(IResource::eReady);

				ID3D11Device* pDevice = Device::GetInstance()->GetInterface();

				HRESULT hr = S_OK;

				D3D11_BUFFER_DESC bufferDesc{};
				bufferDesc.ByteWidth = static_cast<uint32_t>(nBufferSize);
				bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
				bufferDesc.Usage = D3D11_USAGE_DEFAULT;
				bufferDesc.CPUAccessFlags = 0;
				bufferDesc.MiscFlags = 0;

				D3D11_SUBRESOURCE_DATA indexData{};
				indexData.pSysMem = pData;

				hr = pDevice->CreateBuffer(&bufferDesc, &indexData, &m_pBuffer);
				if (FAILED(hr))
				{
					SetState(IResource::eInvalid);
					throw_line("failed to create index buffer");
				}

				SetState(IResource::eComplete);
				SetAlive(true);
			}

			IndexBuffer::~IndexBuffer()
			{
				SafeRelease(m_pBuffer);
			}

			bool IndexBuffer::Map(void** ppData)
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

			void IndexBuffer::Unmap()
			{
				ID3D11DeviceContext* pDeviceContext = Device::GetInstance()->GetImmediateContext();
				pDeviceContext->Unmap(m_pBuffer, 0);
			}
		}
	}
}