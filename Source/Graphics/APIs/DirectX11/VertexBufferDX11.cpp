#include "stdafx.h"
#include "VertexBufferDX11.h"

#include "DeviceDX11.h"

namespace est
{
	namespace graphics
	{
		namespace dx11
		{
			VertexBuffer::VertexBuffer(const uint8_t* pData, uint32_t vertexCount, size_t formatSize, bool isDynamic)
				: m_vertexCount(vertexCount)
				, m_formatSize(static_cast<uint32_t>(formatSize))
				, m_isDynamic(isDynamic)
			{
				TRACER_EVENT(L"VertexBuffer_Init");
				SetState(IResource::eReady);

				ID3D11Device* pDevice = Device::GetInstance()->GetInterface();

				HRESULT hr = S_OK;

				const uint32_t cpuAccessFlags = isDynamic == true ? D3D11_CPU_ACCESS_WRITE : 0;
				const D3D11_USAGE usage = isDynamic == true ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;

				D3D11_BUFFER_DESC bufferDesc{};
				bufferDesc.ByteWidth = static_cast<uint32_t>(vertexCount * formatSize);
				bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
				bufferDesc.Usage = usage;
				bufferDesc.CPUAccessFlags = cpuAccessFlags;
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

			bool VertexBuffer::Map(MappedSubResourceData& mappedSubResourceData, bool isDiscard)
			{
				ID3D11DeviceContext* pDeviceContext = Device::GetInstance()->GetImmediateContext();
				return Map(pDeviceContext, mappedSubResourceData, isDiscard);
			}

			void VertexBuffer::Unmap()
			{
				ID3D11DeviceContext* pDeviceContext = Device::GetInstance()->GetImmediateContext();
				Unmap(pDeviceContext);
			}

			bool VertexBuffer::Map(ID3D11DeviceContext* pDeviceContext, MappedSubResourceData& mappedSubResourceData, bool isDiscard)
			{
				const D3D11_MAP mapType = isDiscard == true ? D3D11_MAP_WRITE_DISCARD : D3D11_MAP_WRITE_NO_OVERWRITE;
				HRESULT hr = pDeviceContext->Map(m_pBuffer, 0, mapType, 0, reinterpret_cast<D3D11_MAPPED_SUBRESOURCE*>(&mappedSubResourceData));
				if (FAILED(hr))
				{
					mappedSubResourceData = {};
					return false;
				}
				return true;
			}

			void VertexBuffer::Unmap(ID3D11DeviceContext* pDeviceContext)
			{
				pDeviceContext->Unmap(m_pBuffer, 0);
			}
		}
	}
}