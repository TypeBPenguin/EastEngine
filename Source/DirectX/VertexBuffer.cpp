#include "stdafx.h"
#include "VertexBuffer.h"

#include "Vertex.h"

namespace EastEngine
{
	namespace Graphics
	{
		VertexBuffer::VertexBuffer()
			: m_pVertexBuffer(nullptr)
			, m_nFormatSize(0)
			, m_emVertexFormat(EmVertexFormat::eUnknown)
			, m_nVertexNum(0)
		{
		}

		VertexBuffer::~VertexBuffer()
		{
			SafeRelease(m_pVertexBuffer);
		}

		bool VertexBuffer::Init(EmVertexFormat::Type emVertexFormat, uint32_t nElementCount, const void* pData, D3D11_USAGE emUsage, uint32_t nOptions)
		{
			uint32_t nAccessFlag = 0;
			if (emUsage == D3D11_USAGE_DYNAMIC)
			{
				nAccessFlag = D3D11_CPU_ACCESS_WRITE;
			}

			uint32_t nFormatSize = GetVertexFormatSize(emVertexFormat);
			uint32_t nDataSize = nFormatSize * nElementCount;

			D3D11_BUFFER_DESC bufferDesc;
			Memory::Clear(&bufferDesc, sizeof(D3D11_BUFFER_DESC));

			bufferDesc.ByteWidth = nDataSize;
			bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			bufferDesc.Usage = static_cast<D3D11_USAGE>(emUsage);
			bufferDesc.CPUAccessFlags = nAccessFlag;
			bufferDesc.MiscFlags = 0;

			if (pData != nullptr)
			{
				D3D11_SUBRESOURCE_DATA dataDesc;
				Memory::Clear(&dataDesc, sizeof(D3D11_SUBRESOURCE_DATA));
				dataDesc.pSysMem = pData;

				if (FAILED(GetDevice()->CreateBuffer(&bufferDesc, &dataDesc, &m_pVertexBuffer)))
					return nullptr;

				if ((nOptions & Option::eSaveRawValue) != 0)
				{
					m_vecRawValue.reserve(nElementCount);

					const byte* pValue = static_cast<const byte*>(pData);
					m_vecRawValue.insert(m_vecRawValue.end(), pValue, pValue + nElementCount);
				}

				if ((nOptions & Option::eSaveVertexPos) != 0)
				{
					m_vecVertexPos.resize(nElementCount);

					const byte* pValue = static_cast<const byte*>(pData);

					Concurrency::parallel_for(0u, nElementCount, [&](uint32_t i)
					{
						uint32_t nBufferIdx = nFormatSize * i;

						const VertexPos* pVertexPos = reinterpret_cast<const VertexPos*>(&pValue[nBufferIdx]);
						m_vecVertexPos[i] = *pVertexPos;
					});
				}

				/*if ((nOptions & Option::eSaveVertexClipSpace) != 0)
				{
					m_vecVertexClipSpace.resize(bufferData.dataCount);

					const byte* pValue = static_cast<const byte*>(pData);

					Concurrency::parallel_for(0u, nElementCount, [&](uint32_t i)
					{
						uint32_t nBufferIdx = nFormatSize * i;

						const VertexPos* pVertexPos = reinterpret_cast<const VertexPos*>(&pValue[nBufferIdx]);
						m_vecVertexClipSpace[i] = *pVertexPos;
					});
				}*/
			}
			else
			{
				if (FAILED(GetDevice()->CreateBuffer(&bufferDesc, nullptr, &m_pVertexBuffer)))
					return false;
			}

			m_nFormatSize = nFormatSize;
			m_nVertexNum = nElementCount;
			m_emVertexFormat = emVertexFormat;

			return true;
		}

		bool VertexBuffer::Map(uint32_t nSubresource, D3D11_MAP emMap, void** ppData) const
		{
			D3D11_MAPPED_SUBRESOURCE map;
			Memory::Clear(&map, sizeof(D3D11_MAPPED_SUBRESOURCE));

			HRESULT hr = GetDeviceContext()->Map(m_pVertexBuffer, nSubresource, static_cast<D3D11_MAP>(emMap), 0, &map);
			if (FAILED(hr))
			{
				*ppData = nullptr;
				return false;
			}

			*ppData = map.pData;
			return true;
		}

		void VertexBuffer::Unmap(uint32_t nSubresource) const
		{
			GetDeviceContext()->Unmap(m_pVertexBuffer, nSubresource);
		}
	}
}