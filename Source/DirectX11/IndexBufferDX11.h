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
				IndexBuffer(const uint8_t* pData, uint32_t indexCount, size_t formatSize);
				virtual ~IndexBuffer();

			public:
				virtual uint32_t GetIndexCount() const override { return m_indexCount; }

				virtual bool Map(void** ppData) override;
				virtual void Unmap() override;

			public:
				ID3D11Buffer* GetBuffer() const { return m_pBuffer; }

			private:
				ID3D11Buffer* m_pBuffer{ nullptr };
				uint32_t m_indexCount{ 0 };
			};
		}
	}
}