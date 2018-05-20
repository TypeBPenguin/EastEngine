#pragma once

#include "GraphicsInterface/GraphicsInterface.h"

struct ID3D11Buffer;

namespace eastengine
{
	namespace graphics
	{
		namespace dx11
		{
			class IndexBuffer : public IIndexBuffer
			{
			public:
				IndexBuffer(const uint8_t* pData, size_t nBufferSize, uint32_t nIndexCount);
				virtual ~IndexBuffer();

			public:
				virtual uint32_t GetIndexCount() const override { return m_nIndexCount; }

				virtual bool Map(void** ppData) override;
				virtual void Unmap() override;

			public:
				ID3D11Buffer* GetBuffer() const { return m_pBuffer; }

			private:
				ID3D11Buffer* m_pBuffer{ nullptr };
				uint32_t m_nIndexCount{ 0 };
			};
		}
	}
}