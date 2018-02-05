#pragma once

#include "D3DInterface.h"

namespace EastEngine
{
	namespace Graphics
	{
		class IndexBuffer : public IIndexBuffer
		{
		public:
			IndexBuffer();
			virtual ~IndexBuffer();

			bool Init(uint32_t nElementCount, const uint32_t* pData, D3D11_USAGE emUsage = D3D11_USAGE_DEFAULT, uint32_t nOptions = IIndexBuffer::Option::eNone);

		public:
			virtual ID3D11Buffer* GetBuffer() const override { return m_pIndexBuffer; }
			virtual uint32_t GetFormatSize() const override { return m_nFormatSize; }
			virtual uint32_t GetFormat() const override { return DXGI_FORMAT_R32_UINT; }
			virtual uint32_t GetIndexNum() const override { return m_nIndexNum; }

			virtual bool Map(ThreadType emThreadID, uint32_t Subresource, D3D11_MAP emMap, void** ppData) const override;
			virtual void Unmap(ThreadType emThreadID, uint32_t Subresource) const override;

			const uint32_t* GetRawValuePtr() const
			{
				if (m_vecRawValue.empty() == true)
					return nullptr;

				return &m_vecRawValue.front();
			}

		protected:
			ID3D11Buffer* m_pIndexBuffer;
			uint32_t m_nFormatSize;
			uint32_t m_nIndexNum;

			std::vector<uint32_t> m_vecRawValue;
		};
	}
}