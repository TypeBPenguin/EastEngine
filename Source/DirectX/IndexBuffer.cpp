#include "stdafx.h"
#include "IndexBuffer.h"

namespace EastEngine
{
	namespace Graphics
	{
		IndexBuffer::IndexBuffer()
			: m_pIndexBuffer(nullptr)
			, m_nFormatSize(0)
			, m_nIndexNum(0)
		{
		}

		IndexBuffer::~IndexBuffer()
		{
			SafeRelease(m_pIndexBuffer);
		}

		bool IndexBuffer::Init(uint32_t nElementCount, const uint32_t* pData, D3D11_USAGE emUsage, uint32_t nOptions)
		{
			uint32_t nAccessFlag = 0;
			if (emUsage == D3D11_USAGE_DYNAMIC)
			{
				nAccessFlag = D3D11_CPU_ACCESS_WRITE;
			}

			uint32_t nFormatSize = sizeof(uint32_t);
			uint32_t nDataSize = nFormatSize * nElementCount;

			D3D11_BUFFER_DESC bufferDesc;
			Memory::Clear(&bufferDesc, sizeof(D3D11_BUFFER_DESC));

			bufferDesc.ByteWidth = nDataSize;
			bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
			bufferDesc.Usage = static_cast<D3D11_USAGE>(emUsage);
			bufferDesc.CPUAccessFlags = nAccessFlag;
			bufferDesc.MiscFlags = 0;

			if (pData != nullptr)
			{
				D3D11_SUBRESOURCE_DATA dataDesc;
				Memory::Clear(&dataDesc, sizeof(D3D11_SUBRESOURCE_DATA));
				dataDesc.pSysMem = pData;

				if (FAILED(GetDevice()->CreateBuffer(&bufferDesc, &dataDesc, &m_pIndexBuffer)))
					return nullptr;

				if ((nOptions & Option::eSaveRawValue) != 0)
				{
					m_vecRawValue.reserve(nElementCount);

					m_vecRawValue.insert(m_vecRawValue.end(), pData, pData + nElementCount);
				}
			}
			else
			{
				if (FAILED(GetDevice()->CreateBuffer(&bufferDesc, nullptr, &m_pIndexBuffer)))
					return nullptr;
			}

			m_nFormatSize = nFormatSize;
			m_nIndexNum = nElementCount;

			return true;
		}

		bool IndexBuffer::Map(ThreadType emThreadID, uint32_t nSubresource, D3D11_MAP emMap, void** ppData) const
		{
			D3D11_MAPPED_SUBRESOURCE map;
			Memory::Clear(&map, sizeof(D3D11_MAPPED_SUBRESOURCE));

			HRESULT hr = GetDeferredContext(emThreadID)->Map(m_pIndexBuffer, nSubresource, static_cast<D3D11_MAP>(emMap), 0, &map);
			if (FAILED(hr))
			{
				*ppData = nullptr;
				return false;
			}

			*ppData = map.pData;
			return true;
		}

		void IndexBuffer::Unmap(ThreadType emThreadID, uint32_t Subresource) const
		{
			GetDeferredContext(emThreadID)->Unmap(m_pIndexBuffer, Subresource);
		}
	}
}