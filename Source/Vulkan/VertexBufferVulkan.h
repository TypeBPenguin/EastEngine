#pragma once

#include "GraphicsInterface/GraphicsInterface.h"

namespace eastengine
{
	namespace graphics
	{
		namespace vulkan
		{
			class VertexBuffer : public IVertexBuffer
			{
			public:
				VertexBuffer(const uint8_t* pData, uint32_t vertexCount, size_t formatSize);
				virtual ~VertexBuffer();

			public:
				virtual uint32_t GetVertexCount() const override { return m_vertexCount; }
				virtual uint32_t GetFormatSize() const override { return m_formatSize; }

				virtual bool Map(void** ppData) override;
				virtual void Unmap() override;

			public:
				VkBuffer GetBuffer() const { return m_buffer; }

			private:
				VkBuffer m_buffer{ nullptr };
				VkDeviceMemory m_bufferMemory{ nullptr };
				VkDeviceSize m_bufferSize{ 0 };

				uint32_t m_vertexCount{ 0 };
				uint32_t m_formatSize{ 0 };
			};
		}
	}
}