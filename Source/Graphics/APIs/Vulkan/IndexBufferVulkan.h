#pragma once

#include "Graphics/Interface/GraphicsInterface.h"

namespace est
{
	namespace graphics
	{
		namespace vulkan
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
				VkBuffer GetBuffer() const { return m_buffer; }

			private:
				VkBuffer m_buffer{ nullptr };
				VkDeviceMemory m_bufferMemory{ nullptr };
				VkDeviceSize m_bufferSize{ 0 };

				uint32_t m_indexCount{ 0 };
			};
		}
	}
}