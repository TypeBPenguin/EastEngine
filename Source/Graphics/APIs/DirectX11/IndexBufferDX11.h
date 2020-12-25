#pragma once

#include "Graphics/Interface/GraphicsInterface.h"

struct ID3D11Buffer;

namespace est
{
	namespace graphics
	{
		namespace dx11
		{
			class IndexBuffer : public IIndexBuffer
			{
			public:
				IndexBuffer(const uint8_t* pData, uint32_t indexCount, size_t formatSize, bool isDynamic);
				virtual ~IndexBuffer();

			public:
				virtual uint32_t GetIndexCount() const override { return m_indexCount; }

				virtual bool Map(MappedSubResourceData& mappedSubResourceData, bool isDiscard = true) override;
				virtual void Unmap() override;

			public:
				bool Map(ID3D11DeviceContext* pDeviceContext, MappedSubResourceData& mappedSubResourceData, bool isDiscard);
				void Unmap(ID3D11DeviceContext* pDeviceContext);

			public:
				ID3D11Buffer* GetBuffer() const { return m_pBuffer; }

			private:
				ID3D11Buffer* m_pBuffer{ nullptr };
				uint32_t m_indexCount{ 0 };
				bool m_isDynamic{ false };
			};
		}
	}
}