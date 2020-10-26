#pragma once

#include "Graphics/Interface/GraphicsInterface.h"

struct ID3D11Buffer;

namespace est
{
	namespace graphics
	{
		namespace dx11
		{
			class VertexBuffer : public IVertexBuffer
			{
			public:
				VertexBuffer(const uint8_t* pData, uint32_t vertexCount, size_t formatSize, bool isDynamic);
				virtual ~VertexBuffer();

			public:
				virtual uint32_t GetVertexCount() const override { return m_vertexCount; }
				virtual uint32_t GetFormatSize() const override { return m_formatSize; }

				virtual bool Map(void** ppData) override;
				virtual void Unmap() override;

			public:
				ID3D11Buffer* GetBuffer() const { return m_pBuffer; }

			private:
				ID3D11Buffer* m_pBuffer{ nullptr };

				uint32_t m_vertexCount{ 0 };
				uint32_t m_formatSize{ 0 };

				bool m_isDynamic{ false };
			};
		}
	}
}