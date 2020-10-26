#include "stdafx.h"
#include "IndexBufferDX11.h"

#include "DeviceDX11.h"

namespace est
{
	namespace graphics
	{
		namespace dx11
		{
			IndexBuffer::IndexBuffer(const uint8_t* pData, uint32_t indexCount, size_t formatSize, bool isDynamic)
				: m_indexCount(indexCount)
				, m_isDynamic(isDynamic)
			{
				TRACER_EVENT(L"IndexBufferDX11_Init");
				SetState(IResource::eReady);

				ID3D11Device* pDevice = Device::GetInstance()->GetInterface();

				HRESULT hr = S_OK;

				const uint32_t cpuAccessFlags = isDynamic == true ? D3D11_CPU_ACCESS_WRITE : 0;
				const D3D11_USAGE usage = isDynamic == true ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;

				D3D11_BUFFER_DESC bufferDesc{};
				bufferDesc.ByteWidth = static_cast<uint32_t>(indexCount * formatSize);
				bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
				bufferDesc.Usage = usage;
				bufferDesc.CPUAccessFlags = cpuAccessFlags;
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