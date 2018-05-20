#pragma once

#include "GraphicsInterface/GraphicsInterface.h"

struct ID3D11Buffer;

namespace eastengine
{
	namespace graphics
	{
		namespace dx11
		{
			class VertexBuffer : public IVertexBuffer
			{
			public:
				VertexBuffer(const uint8_t* pData, size_t nBufferSize, uint32_t nVertexCount);
				virtual ~VertexBuffer();

			public:
				virtual uint32_t GetVertexCount() const override { return m_nVertexCount; }
				virtual uint32_t GetFormatSize() const override { return m_nFormatSize; }

				virtual bool Map(void** ppData) override;
				virtual void Unmap() override;

			public:
				ID3D11Buffer* GetBuffer() const { return m_pBuffer; }

			private:
				ID3D11Buffer* m_pBuffer{ nullptr };

				uint32_t m_nVertexCount{ 0 };
				uint32_t m_nFormatSize{ 0 };
			};
		}
	}
}