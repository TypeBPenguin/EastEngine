#include "stdafx.h"
#include "StructuredBuffer.h"

namespace EastEngine
{
	namespace Graphics
	{
		StructuredBuffer::StructuredBuffer()
			: m_nByteWidth(0)
			, m_pBuffer(nullptr)
			, m_pUnorderedAccessView(nullptr)
			, m_pShaderResourceView(nullptr)
		{
		}

		StructuredBuffer::~StructuredBuffer()
		{
			SafeRelease(m_pShaderResourceView);
			SafeRelease(m_pUnorderedAccessView);
			SafeRelease(m_pBuffer);
		}

		bool StructuredBuffer::Init(void* pData, uint32_t nNumElements, uint32_t nByteStride, bool isEnableCpuWrite, bool isEnableGpuWrite)
		{
			m_nByteWidth = nNumElements * nByteStride;

			// Create buffer
			D3D11_BUFFER_DESC buf_desc;
			Memory::Clear(&buf_desc, sizeof(D3D11_BUFFER_DESC));
			buf_desc.ByteWidth = m_nByteWidth;
			buf_desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
			buf_desc.StructureByteStride = nByteStride;

			if (isEnableCpuWrite == false && isEnableGpuWrite == false)
			{
				buf_desc.Usage = D3D11_USAGE_IMMUTABLE;
				buf_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
				buf_desc.CPUAccessFlags = 0;
			}
			else if (isEnableCpuWrite == true && isEnableGpuWrite == false)
			{
				buf_desc.Usage = D3D11_USAGE_DYNAMIC;
				buf_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
				buf_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			}
			else if (isEnableCpuWrite == false && isEnableGpuWrite == true)
			{
				buf_desc.Usage = D3D11_USAGE_DEFAULT;
				buf_desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
				buf_desc.CPUAccessFlags = 0;
			}
			else
			{
				assert((isEnableCpuWrite && isEnableGpuWrite) == false);
			}

			D3D11_SUBRESOURCE_DATA init_data = { pData, 0, 0 };
			if (FAILED(GetDevice()->CreateBuffer(&buf_desc, pData != nullptr ? &init_data : nullptr, &m_pBuffer)))
				return false;

			// Create shader resource view
			D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc;
			Memory::Clear(&srv_desc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
			srv_desc.Format = DXGI_FORMAT_UNKNOWN;
			srv_desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
			srv_desc.Buffer.FirstElement = 0;
			srv_desc.Buffer.NumElements = nNumElements;

			if (FAILED(GetDevice()->CreateShaderResourceView(m_pBuffer, &srv_desc, &m_pShaderResourceView)))
				return false;

			if (isEnableGpuWrite == true)
			{
				// Create undordered access view
				D3D11_UNORDERED_ACCESS_VIEW_DESC uav_desc;
				Memory::Clear(&uav_desc, sizeof(D3D11_UNORDERED_ACCESS_VIEW_DESC));
				uav_desc.Format = DXGI_FORMAT_UNKNOWN;
				uav_desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
				uav_desc.Buffer.FirstElement = 0;
				uav_desc.Buffer.NumElements = nNumElements;
				uav_desc.Buffer.Flags = 0;

				if (FAILED(GetDevice()->CreateUnorderedAccessView(m_pBuffer, &uav_desc, &m_pUnorderedAccessView)))
					return false;
			}

			return true;
		}

		void StructuredBuffer::UpdateSubresource(uint32_t DstSubresource, const void* pSrcData, uint32_t SrcRowPitch)
		{
			D3D11_BOX box;
			box.front = 0;
			box.back = 1;
			box.left = 0;
			box.right = m_nByteWidth;
			box.top = 0;
			box.bottom = 1;

			GetDeviceContext()->UpdateSubresource(m_pBuffer, DstSubresource, &box, pSrcData, SrcRowPitch, 0);
		}
	}
}