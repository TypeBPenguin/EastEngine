#pragma once

#include "GraphicsInterface/GraphicsInterface.h"

namespace eastengine
{
	namespace graphics
	{
		namespace vulkan
		{
			class IndexBuffer : public IIndexBuffer
			{
			public:
				IndexBuffer(const uint8_t* pData, size_t nBufferSize, uint32_t nVertexCount);
				virtual ~IndexBuffer();

			public:
				virtual uint32_t GetIndexCount() const override { return m_nIndexCount; }

				virtual bool Map(void** ppData) override;
				virtual void Unmap() override;

			public:
				VkBuffer GetBuffer() const { return m_buffer; }

			private:
				VkBuffer m_buffer{ nullptr };
				VkDeviceMemory m_bufferMemory{ nullptr };
				VkDeviceSize m_bufferSize{ 0 };

				uint32_t m_nIndexCount{ 0 };
			};
		}
	}
}