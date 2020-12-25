#include "stdafx.h"
#include "IndexBufferVulkan.h"

#include "DeviceVulkan.h"

namespace est
{
	namespace graphics
	{
		namespace vulkan
		{
			IndexBuffer::IndexBuffer(const uint8_t* pData, uint32_t indexCount, size_t formatSize, bool isDynamic)
				: m_indexCount(indexCount)
			{
				VkDevice device = Device::GetInstance()->GetInterface();

				m_bufferSize = indexCount * formatSize;

				VkBuffer stagingBuffer{ nullptr };
				VkDeviceMemory stagingBufferMemory{ nullptr };
				Device::GetInstance()->CreateBuffer(m_bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingBufferMemory, nullptr);

				void* pBuffer = nullptr;
				vkMapMemory(device, stagingBufferMemory, 0, m_bufferSize, 0, &pBuffer);
				memcpy(pBuffer, pData, m_bufferSize);
				vkUnmapMemory(device, stagingBufferMemory);

				Device::GetInstance()->CreateBuffer(m_bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &m_buffer, &m_bufferMemory, nullptr);

				Device::GetInstance()->CopyBuffer(stagingBuffer, m_buffer, m_bufferSize);

				vkDestroyBuffer(device, stagingBuffer, nullptr);
				vkFreeMemory(device, stagingBufferMemory, nullptr);
			}

			IndexBuffer::~IndexBuffer()
			{
				VkDevice device = Device::GetInstance()->GetInterface();

				vkDestroyBuffer(device, m_buffer, nullptr);
				vkFreeMemory(device, m_bufferMemory, nullptr);
			}

			bool IndexBuffer::Map(MappedSubResourceData& mappedSubResourceData, bool isDiscard)
			{
				VkDevice device = Device::GetInstance()->GetInterface();

				VkResult result = vkMapMemory(device, m_bufferMemory, 0, m_bufferSize, 0, &mappedSubResourceData.pData);
				if (result != VK_SUCCESS)
				{
					mappedSubResourceData = {};
					return false;
				}

				return true;
			}

			void IndexBuffer::Unmap()
			{
				VkDevice device = Device::GetInstance()->GetInterface();

				vkUnmapMemory(device, m_bufferMemory);
			}
		}
	}
}