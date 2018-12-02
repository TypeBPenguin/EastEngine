#include "stdafx.h"
#include "VertexBufferVulkan.h"

#include "DeviceVulkan.h"

namespace eastengine
{
	namespace graphics
	{
		namespace vulkan
		{
			VertexBuffer::VertexBuffer(const uint8_t* pData, uint32_t vertexCount, size_t formatSize, bool isDynamic)
				: m_vertexCount(vertexCount)
				, m_formatSize(static_cast<uint32_t>(formatSize))
			{
				VkDevice device = Device::GetInstance()->GetInterface();

				m_bufferSize = vertexCount * formatSize;

				VkBuffer stagingBuffer{ nullptr };
				VkDeviceMemory stagingBufferMemory{ nullptr };
				Device::GetInstance()->CreateBuffer(m_bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingBufferMemory, nullptr);

				void* pBuffer = nullptr;
				vkMapMemory(device, stagingBufferMemory, 0, m_bufferSize, 0, &pBuffer);
				memcpy(pBuffer, pData, m_bufferSize);
				vkUnmapMemory(device, stagingBufferMemory);

				Device::GetInstance()->CreateBuffer(m_bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &m_buffer, &m_bufferMemory, nullptr);

				Device::GetInstance()->CopyBuffer(stagingBuffer, m_buffer, m_bufferSize);

				vkDestroyBuffer(device, stagingBuffer, nullptr);
				vkFreeMemory(device, stagingBufferMemory, nullptr);
			}

			VertexBuffer::~VertexBuffer()
			{
				VkDevice device = Device::GetInstance()->GetInterface();

				vkDestroyBuffer(device, m_buffer, nullptr);
				vkFreeMemory(device, m_bufferMemory, nullptr);
			}

			bool VertexBuffer::Map(void** ppData)
			{
				VkDevice device = Device::GetInstance()->GetInterface();

				VkResult result = vkMapMemory(device, m_bufferMemory, 0, m_bufferSize, 0, ppData);
				if (result != VK_SUCCESS)
				{
					(*ppData) = nullptr;
					return false;
				}

				return true;
			}

			void VertexBuffer::Unmap()
			{
				VkDevice device = Device::GetInstance()->GetInterface();

				vkUnmapMemory(device, m_bufferMemory);
			}
		}
	}
}
