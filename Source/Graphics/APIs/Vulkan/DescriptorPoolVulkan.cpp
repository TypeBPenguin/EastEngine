#include "stdafx.h"
#include "DescriptorPoolVulkan.h"

#include "DeviceVulkan.h"

namespace est
{
	namespace graphics
	{
		namespace vulkan
		{
			DescriptorPool::DescriptorPool(uint32_t nPersistentCount, VkDescriptorType descriptorType)
			{
				VkDevice device = Device::GetInstance()->GetInterface();

				assert(nPersistentCount > 0);

				m_nPersistentCount = nPersistentCount;
				m_descriptorType = descriptorType;

				m_vecDeadList.resize(nPersistentCount);
				for (uint32_t i = 0; i < nPersistentCount; ++i)
				{
					m_vecDeadList[i] = i;
				}

				VkDescriptorPoolSize poolSize{};
				poolSize.type = descriptorType;
				poolSize.descriptorCount = nPersistentCount;

				VkDescriptorPoolCreateInfo poolInfo{};
				poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
				poolInfo.poolSizeCount = 1;
				poolInfo.pPoolSizes = &poolSize;
				poolInfo.maxSets = nPersistentCount;

				if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &m_descriptorPool) != VK_SUCCESS)
				{
					throw_line("failed to create descriptor pool");
				}

				VkDescriptorSetLayoutBinding samplerLayoutBinding{};
				samplerLayoutBinding.binding = 0;
				samplerLayoutBinding.descriptorCount = nPersistentCount;
				samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				samplerLayoutBinding.pImmutableSamplers = nullptr;
				samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

				const VkDescriptorSetLayoutBinding bindings[] = { samplerLayoutBinding };
				VkDescriptorSetLayoutCreateInfo layoutInfo{};
				layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
				layoutInfo.bindingCount = _countof(bindings);
				layoutInfo.pBindings = bindings;

				if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &m_descriptorSetLayout) != VK_SUCCESS)
				{
					throw_line("failed to create descriptor set layout");
				}

				const VkDescriptorSetLayout layouts[] = { m_descriptorSetLayout };
				VkDescriptorSetAllocateInfo allocInfo{};
				allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
				allocInfo.descriptorPool = m_descriptorPool;
				allocInfo.descriptorSetCount = nPersistentCount;
				allocInfo.pSetLayouts = layouts;

				if (vkAllocateDescriptorSets(device, &allocInfo, &m_descriptorSet) != VK_SUCCESS)
				{
					throw_line("failed to allocate descriptor set");
				}
			}

			DescriptorPool::~DescriptorPool()
			{
				VkDevice device = Device::GetInstance()->GetInterface();

				vkDestroyDescriptorSetLayout(device, m_descriptorSetLayout, nullptr);
				vkDestroyDescriptorPool(device, m_descriptorPool, nullptr);
			}

			uint32_t DescriptorPool::AllocateDescriptorSet(VkImageView imageView)
			{
				AcquireSRWLockExclusive(&m_lock);

				assert(m_nAllocatedPersistentCount < m_nPersistentCount);
				uint32_t index = m_vecDeadList[m_nAllocatedPersistentCount];
				++m_nAllocatedPersistentCount;

				ReleaseSRWLockExclusive(&m_lock);

				/*alloc.index = index;
				for (int i = 0; i < m_nHeapCount; ++i)
				{
					alloc.cpuHandles[i] = m_startCPUHandle[i];
					alloc.cpuHandles[i].ptr += index * m_nDescriptorSize;
				}

				return alloc;

				VkDevice device = Device::GetInstance()->GetInterface();

				VkDescriptorImageInfo imageInfo{};
				imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				imageInfo.imageView = imageView;
				imageInfo.sampler = Device::GetInstance()->GetSampler(SamplerState::eMinMagMipLinearWrap);

				VkDescriptorBufferInfo bufferInfo{};
				bufferInfo.

				VkWriteDescriptorSet descriptorWrites{};
				descriptorWrites.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrites.dstSet = m_descriptorSet;
				descriptorWrites.dstBinding = 0;
				descriptorWrites.dstArrayElement = 0;
				descriptorWrites.descriptorType = m_descriptorType;
				descriptorWrites.descriptorCount = 1;
				descriptorWrites.pImageInfo = &imageInfo;

				vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);*/

				return index;
			}

			void DescriptorPool::FreeDescriptorSet(uint32_t descriptorSet)
			{
			}
		}
	}
}