#pragma once

namespace est
{
	namespace graphics
	{
		namespace vulkan
		{
			class DescriptorPool
			{
			public:
				DescriptorPool(uint32_t nDescriptorSetCount, VkDescriptorType descriptorType);
				~DescriptorPool();

			public:
				uint32_t AllocateDescriptorSet(VkImageView imageView);
				void FreeDescriptorSet(uint32_t descriptorSet);

			public:
				VkDescriptorPool GetHeap() const;
				VkDescriptorSetLayout GetSetLayout() const;
				uint32_t TotalDescriptorsCount() const;

			private:
				VkDescriptorPool m_descriptorPool{ nullptr };
				uint32_t m_nPersistentCount{ 0 };
				uint32_t m_nAllocatedPersistentCount{ 0 };
				std::vector<uint32_t> m_vecDeadList;

				VkDescriptorSetLayout m_descriptorSetLayout{ nullptr };
				VkDescriptorSet m_descriptorSet;

				VkDescriptorType m_descriptorType{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER };
				SRWLOCK m_lock{ SRWLOCK_INIT };
			};
		}
	}
}