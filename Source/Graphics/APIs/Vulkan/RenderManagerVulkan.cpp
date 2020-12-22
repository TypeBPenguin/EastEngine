#include "stdafx.h"
#include "RenderManagerVulkan.h"

#include "Graphics/Interface/Camera.h"

#include "DeviceVulkan.h"

#include "ModelRendererVulkan.h"
#include "DeferredRendererVulkan.h"

namespace est
{
	namespace graphics
	{
		namespace vulkan
		{
			//VkVertexInputBindingDescription GetBindingDescription(EmVertexFormat::Type emVertexFormat)
			//{
			//	switch (emVertexFormat)
			//	{
			//	case EmVertexFormat::ePosTex:
			//	{
			//		VkVertexInputBindingDescription bindingDescription{};
			//		bindingDescription.binding = 0;
			//		bindingDescription.stride = sizeof(VertexPosTex);
			//		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
			//
			//		return bindingDescription;
			//	}
			//	break;
			//	}
			//
			//	return {};
			//}
			//
			//const std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions(EmVertexFormat::Type emVertexFormat)
			//{
			//	std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
			//
			//	switch (emVertexFormat)
			//	{
			//	case EmVertexFormat::ePosTex:
			//	{
			//		attributeDescriptions.resize(2);
			//
			//		attributeDescriptions[0].binding = 0;
			//		attributeDescriptions[0].location = 0;
			//		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
			//		attributeDescriptions[0].offset = offsetof(VertexPosTex, pos);
			//
			//		attributeDescriptions[1].binding = 0;
			//		attributeDescriptions[1].location = 1;
			//		attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
			//		attributeDescriptions[1].offset = offsetof(VertexPosTex, uv);
			//	}
			//	break;
			//	}
			//
			//	return attributeDescriptions;
			//}

			class RenderManager::Impl
			{
			public:
				Impl();
				~Impl();

			private:
				//struct UniformBufferObject
				//{
				//	math::Matrix matWVP;
				//};
				//
				//struct TextureDescriptorSet
				//{
				//	VkDescriptorSet set{ nullptr };
				//	size_t frameIndex = 0;
				//	bool isUsing{ false };
				//};
				//
				//struct Subset
				//{
				//	TextureDescriptorSet* pTextureDescriptorSet{ nullptr };
				//
				//	const VertexBuffer* pVertexBuffer{ nullptr };
				//	const IndexBuffer* pIndexBuffer{ nullptr };
				//	const Texture* pTexture{ nullptr };
				//	math::Matrix matWVP;
				//};
				//
				//enum
				//{
				//	eUniformBufferAlignedSize = (sizeof(UniformBufferObject) + 255) & ~255,
				//	eInstanceCount = 256,
				//};

			public:
				void Render();
				//void Resize();

			public:
				//void PushJob(const VertexBuffer* pVertexBuffer, const IndexBuffer* pIndexBuffer, const Texture* pTexture, const math::Matrix& matWVP)
				//{
				//	if (m_vecSubsets.size() >= eInstanceCount)
				//		return;
				//
				//	Subset subset;
				//	subset.pVertexBuffer = pVertexBuffer;
				//	subset.pIndexBuffer = pIndexBuffer;
				//	subset.pTexture = pTexture;
				//	subset.matWVP = matWVP;
				//
				//	auto iter = m_umapDescriptorSet.find(subset.pTexture->GetKey());
				//	if (iter != m_umapDescriptorSet.end())
				//	{
				//		subset.pTextureDescriptorSet = iter->second;
				//	}
				//
				//	m_vecSubsets.push_back(subset);
				//}

				void PushJob(const RenderJobStatic& renderJob) { m_pModelRenderer->PushJob(renderJob); }
				void PushJob(const RenderJobSkinned& renderJob) { m_pModelRenderer->PushJob(renderJob); }
				void PushJob(const RenderJobTerrain& renderJob) {}
				void PushJob(const RenderJobVertex& renderJob) {};

			private:
				std::unique_ptr<ModelRenderer> m_pModelRenderer;
				std::unique_ptr<DeferredRenderer> m_pDeferredRenderer;

			public:
				//const ImageBuffer* GetImageBuffer() const { return m_pRenderTarget.get(); }

			private:
				//void CreateRenderTarget();
				//void CreateRenderPass();
				//void CreateFrameBuffer();
				//
				//void CreateGraphicsPipeline();
				//void CreateDescriptorSetLayout();
				//void CreateDescriptorPool();
				//void CreateDescriptorSet();
				//void CreateSampler();
				//void CreateUniformBuffer();
				//
				//VkShaderModule CreateShaderModule(const std::vector<char>& code);

			private:
				//std::unique_ptr<ImageBuffer> m_pRenderTarget;
				//VkFramebuffer m_frameBuffer{ nullptr };
				//
				//VkPipelineLayout m_pipelineLayout{ nullptr };
				//VkPipeline m_graphicsPipeline{ nullptr };
				//
				//VkRenderPass m_renderPass{ nullptr };
				//
				//VkDescriptorPool m_descriptorPool{ nullptr };
				//
				//std::array<TextureDescriptorSet, eInstanceCount> m_descriptorSets;
				//std::unordered_map<ITexture::Key, TextureDescriptorSet*> m_umapDescriptorSet;
				//VkDescriptorSetLayout m_descSetLayoutTexture{ nullptr };
				//
				//VkSampler m_sampler{ nullptr };
				//
				//VkBuffer m_uniformBuffer{ nullptr };
				//VkDeviceMemory m_uniformBufferMemory{ nullptr };
				//VkDescriptorSet m_uniformBufferDescriptorSet{ nullptr };
				//VkDescriptorSetLayout m_descSetLayoutUniform{ nullptr };
				//std::vector<uint8_t> m_vecUniformBufferData;
				//
				//std::vector<Subset> m_vecSubsets;
			};

			RenderManager::Impl::Impl()
			{
				m_pModelRenderer = std::make_unique<ModelRenderer>();
				m_pDeferredRenderer = std::make_unique<DeferredRenderer>();

				//CreateRenderTarget();
				//CreateRenderPass();
				//CreateFrameBuffer();
				//
				//CreateDescriptorSetLayout();
				//CreateGraphicsPipeline();
				//CreateDescriptorPool();
				//CreateUniformBuffer();
				//CreateDescriptorSet();
				//CreateSampler();
			}

			RenderManager::Impl::~Impl()
			{
				m_pModelRenderer.reset();
				m_pDeferredRenderer.reset();

				//VkDevice device = Device::GetInstance()->GetInterface();
				//
				//m_pRenderTarget.reset();
				//
				//vkDestroySampler(device, m_sampler, nullptr);
				//
				//vkDestroyBuffer(device, m_uniformBuffer, nullptr);
				//vkFreeMemory(device, m_uniformBufferMemory, nullptr);
				//vkDestroyDescriptorSetLayout(device, m_descSetLayoutUniform, nullptr);
				//
				//vkDestroyDescriptorSetLayout(device, m_descSetLayoutTexture, nullptr);
				//
				//vkDestroyDescriptorPool(device, m_descriptorPool, nullptr);
				//vkDestroyPipeline(device, m_graphicsPipeline, nullptr);
				//vkDestroyPipelineLayout(device, m_pipelineLayout, nullptr);
				//vkDestroyFramebuffer(device, m_frameBuffer, nullptr);
				//vkDestroyRenderPass(device, m_renderPass, nullptr);
			}

			void RenderManager::Impl::Render()
			{
				Camera& camera = GetCamera();

				m_pModelRenderer->Render(&camera);
				m_pDeferredRenderer->Render(&camera);

				//VkDevice device = Device::GetInstance()->GetInterface();
				//const uint32_t frameIndex = Device::GetInstance()->GetFrameIndex();
				//const math::Viewport& viewport = Device::GetInstance()->GetViewport();
				//VkExtent2D extent = Device::GetInstance()->GetSwapChainExtent2D();
				//
				//VkCommandBuffer commandBuffer = Device::GetInstance()->GetCommandBuffer(frameIndex);
				//
				//Camera& camera = GetCamera();
				//
				//VkRenderPassBeginInfo renderPassInfo{};
				//renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
				//renderPassInfo.renderPass = m_renderPass;
				//renderPassInfo.framebuffer = m_frameBuffer;
				//renderPassInfo.renderArea.offset = { 0, 0 };
				//renderPassInfo.renderArea.extent = extent;
				//
				//std::array<VkClearValue, 2> clearValues = {};
				//clearValues[0].color = { 0.f, 0.2f, 0.4f, 1.f };
				//clearValues[1].depthStencil = { 1.0f, 0 };
				//
				//renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
				//renderPassInfo.pClearValues = clearValues.data();
				//
				//vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
				//
				//vkCmdSetViewport(commandBuffer, 0, 1, util::Convert(viewport));
				//
				//VkRect2D scissor{};
				//scissor.extent = extent;
				//
				//vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
				//
				//vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline);
				//
				//const math::Matrix matViewProjection = pCamera->GetViewMatrix() * pCamera->GetProjectionMatrix();
				//
				//static UniformBufferObject* pData = nullptr;
				//if (pData == nullptr)
				//{
				//	VkResult result = vkMapMemory(device, m_uniformBufferMemory, 0, eUniformBufferAlignedSize * eInstanceCount, 0, reinterpret_cast<void**>(&pData));
				//	if (result != VK_SUCCESS)
				//	{
				//		throw_line("failed to uniform buffer map");
				//	}
				//}
				//
				//const size_t nSize = m_vecSubsets.size();
				//for (size_t i = 0; i < nSize; ++i)
				//{
				//	Subset& subset = m_vecSubsets[i];
				//	if (subset.pTextureDescriptorSet != nullptr)
				//	{
				//		subset.pTextureDescriptorSet->isUsing = true;
				//		subset.pTextureDescriptorSet->frameIndex = frameIndex;
				//	}
				//}
				//
				//for (auto iter = m_umapDescriptorSet.begin(); iter != m_umapDescriptorSet.end();)
				//{
				//	if (iter->second->frameIndex != frameIndex)
				//	{
				//		iter->second->isUsing = false;
				//		iter = m_umapDescriptorSet.erase(iter);
				//	}
				//	else
				//	{
				//		++iter;
				//	}
				//}
				//
				//size_t nDescriptorIndex = 0;
				//for (size_t i = 0; i < nSize; ++i)
				//{
				//	Subset& subset = m_vecSubsets[i];
				//	if (subset.pTextureDescriptorSet == nullptr)
				//	{
				//		for (; nDescriptorIndex < eInstanceCount; ++nDescriptorIndex)
				//		{
				//			TextureDescriptorSet& node = m_descriptorSets[nDescriptorIndex];
				//			if (node.isUsing == false)
				//			{
				//				node.frameIndex = frameIndex;
				//				node.isUsing = true;
				//
				//				subset.pTextureDescriptorSet = &node;
				//				m_umapDescriptorSet.emplace(subset.pTexture->GetKey(), &node);
				//
				//				VkDescriptorImageInfo imageInfo{};
				//				imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				//				imageInfo.imageView = subset.pTexture->GetImageView();
				//				imageInfo.sampler = m_sampler;
				//
				//				std::array<VkWriteDescriptorSet, 1> descriptorWrites{};
				//				descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				//				descriptorWrites[0].dstSet = subset.pTextureDescriptorSet->set;
				//				descriptorWrites[0].dstBinding = 0;
				//				descriptorWrites[0].dstArrayElement = 0;
				//				descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				//				descriptorWrites[0].descriptorCount = 1;
				//				descriptorWrites[0].pImageInfo = &imageInfo;
				//
				//				vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
				//
				//				break;
				//			}
				//		}
				//	}
				//
				//	const size_t nDynamicOffsets = i * eUniformBufferAlignedSize;
				//	if (nDynamicOffsets < eUniformBufferAlignedSize * eInstanceCount)
				//	{
				//		UniformBufferObject* pUniformBuffer = reinterpret_cast<UniformBufferObject*>(&m_vecUniformBufferData[nDynamicOffsets]);
				//		pUniformBuffer->matWVP = subset.matWVP * matViewProjection;
				//	}
				//}
				//
				//memcpy(pData, m_vecUniformBufferData.data(), eUniformBufferAlignedSize * nSize);
				//
				//// Flush to make changes visible to the host 
				//VkMappedMemoryRange memoryRange{};
				//memoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
				//memoryRange.memory = m_uniformBufferMemory;
				//memoryRange.size = eUniformBufferAlignedSize * nSize;
				//vkFlushMappedMemoryRanges(device, 1, &memoryRange);
				//
				////vkUnmapMemory(device, m_uniformBufferMemory);
				//
				//for (size_t i = 0; i < nSize; ++i)
				//{
				//	const Subset& subset = m_vecSubsets[i];
				//
				//	const VkBuffer vertexBuffers[] = { subset.pVertexBuffer->GetBuffer() };
				//	const VkDeviceSize offsets[] = { 0 };
				//	vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
				//	vkCmdBindIndexBuffer(commandBuffer, subset.pIndexBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
				//
				//	const uint32_t nDynamicOffsets[] = { static_cast<uint32_t>(i * eUniformBufferAlignedSize) };
				//	const VkDescriptorSet descriptorSets[] = { m_uniformBufferDescriptorSet, subset.pTextureDescriptorSet->set };
				//	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, _countof(descriptorSets), descriptorSets, _countof(nDynamicOffsets), nDynamicOffsets);
				//
				//	vkCmdDrawIndexed(commandBuffer, subset.pIndexBuffer->GetIndexCount(), 1, 0, 0, 0);
				//}
				//m_vecSubsets.clear();
				//
				//vkCmdEndRenderPass(commandBuffer);
			}

			//void RenderManager::Impl::CreateRenderTarget()
			//{
			//	const VkExtent2D& swapchainExtent = Device::GetInstance()->GetSwapChainExtent2D();

			//	m_pRenderTarget = std::make_unique<ImageBuffer>(*reinterpret_cast<const math::uint2*>(&swapchainExtent),
			//		VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
			//}

			//void RenderManager::Impl::CreateRenderPass()
			//{
			//	VkDevice device = Device::GetInstance()->GetInterface();
			//	const ImageBuffer* pDepthStencil = Device::GetInstance()->GetDepthStencil();

			//	VkAttachmentDescription colorAttachment{};
			//	colorAttachment.format = m_pRenderTarget->GetFormat();
			//	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;

			//	// loadOP
			//	// VK_ATTACHMENT_LOAD_OP_LOAD : 첨부 파일의 기존 내용을 보존한다.
			//	// VK_ATTACHMENT_LOAD_OP_CLEAR : 시작시 값을 상수로 지운다.
			//	// VK_ATTACHMENT_LOAD_OP_DONT_CARE : 기존 내용은 정의되지 않았습니다. 우리는 그들에 관심이 없다.(???)
			//	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;

			//	// storeOp
			//	// VK_ATTACHMENT_STORE_OP_STORE : 렌더링 된 내용은 메모리에 저장되며 나중에 읽을 수 있다.
			//	// VK_ATTACHMENT_STORE_OP_DONT_CARE : 프레임 버퍼의 내용은 렌더링 작업 후에 정의되지 않습니다.(???)
			//	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

			//	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			//	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

			//	// imageLayout
			//	// VK_IMAGE_LAYOUT_PRESENT_SRC_KHR : 프레젠테이션에 최적
			//	// VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : 색상 첨부로 사용되는 이미지, fragment 쉐이더에서 색상을 쓰는데 가장 적합
			//	// VK_IMAGE_LAYOUT_PRESENT_SCR_KHR : 스왑 체인에 표시할 이미지
			//	// VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL : 메모리 복사 작업의 대상으로 사용될 이미지
			//	// VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL : 전송 작업에서 원본처럼 최적(?)
			//	// VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL : 전송 작업의 대상으로 최적
			//	// VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : 쉐이더에서 샘플링에 최적
			//	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			//	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

			//	VkAttachmentReference colorAttachmentRef{};
			//	colorAttachmentRef.attachment = 0;
			//	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			//	VkAttachmentDescription depthAttachment{};
			//	depthAttachment.format = pDepthStencil->GetFormat();
			//	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
			//	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			//	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			//	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			//	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			//	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			//	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			//	VkAttachmentReference depthAttachmentRef{};
			//	depthAttachmentRef.attachment = 1;
			//	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			//	VkSubpassDescription subpass{};
			//	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			//	subpass.colorAttachmentCount = 1;
			//	subpass.pColorAttachments = &colorAttachmentRef;
			//	subpass.pDepthStencilAttachment = &depthAttachmentRef;

			//	VkSubpassDependency dependency{};
			//	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
			//	dependency.dstSubpass = 0;
			//	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			//	dependency.srcAccessMask = 0;
			//	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			//	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

			//	const VkAttachmentDescription attachments[] = { colorAttachment, depthAttachment };

			//	VkRenderPassCreateInfo renderPassInfo{};
			//	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			//	renderPassInfo.attachmentCount = _countof(attachments);
			//	renderPassInfo.pAttachments = attachments;
			//	renderPassInfo.subpassCount = 1;
			//	renderPassInfo.pSubpasses = &subpass;
			//	renderPassInfo.dependencyCount = 1;
			//	renderPassInfo.pDependencies = &dependency;

			//	if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &m_renderPass) != VK_SUCCESS)
			//	{
			//		throw_line("failed to create render pass");
			//	}
			//}

			//void RenderManager::Impl::CreateFrameBuffer()
			//{
			//	VkDevice device = Device::GetInstance()->GetInterface();
			//	const VkExtent2D& swapchainExtent = Device::GetInstance()->GetSwapChainExtent2D();
			//	const ImageBuffer* pDepthStencil = Device::GetInstance()->GetDepthStencil();

			//	const VkImageView attachments[] =
			//	{
			//		m_pRenderTarget->GetImageView(),
			//		pDepthStencil->GetImageView(),
			//	};

			//	VkFramebufferCreateInfo frameBufferInfo{};
			//	frameBufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			//	frameBufferInfo.renderPass = m_renderPass;
			//	frameBufferInfo.attachmentCount = _countof(attachments);
			//	frameBufferInfo.pAttachments = attachments;
			//	frameBufferInfo.width = swapchainExtent.width;
			//	frameBufferInfo.height = swapchainExtent.height;
			//	frameBufferInfo.layers = 1;

			//	if (vkCreateFramebuffer(device, &frameBufferInfo, nullptr, &m_frameBuffer) != VK_SUCCESS)
			//	{
			//		throw_line("failed to create framebuffer");
			//	}
			//}

			//void RenderManager::Impl::CreateGraphicsPipeline()
			//{
			//	VkDevice device = Device::GetInstance()->GetInterface();
			//	VkExtent2D extend = Device::GetInstance()->GetSwapChainExtent2D();
			//	const math::Viewport& viewport = Device::GetInstance()->GetViewport();

			//	auto vertShaderCode = Device::GetInstance()->ReadFile("../Graphics/Interface/vert.spv");
			//	auto fragShaderCode = Device::GetInstance()->ReadFile("../Graphics/Interface/frag.spv");

			//	VkShaderModule vertShaderModule = CreateShaderModule(vertShaderCode);
			//	VkShaderModule fragShaderModule = CreateShaderModule(fragShaderCode);

			//	VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
			//	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			//	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
			//	vertShaderStageInfo.module = vertShaderModule;
			//	vertShaderStageInfo.pName = "main";

			//	VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
			//	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			//	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
			//	fragShaderStageInfo.module = fragShaderModule;
			//	fragShaderStageInfo.pName = "main";

			//	VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

			//	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
			//	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

			//	VkVertexInputBindingDescription bindingDescription = GetBindingDescription(EmVertexFormat::ePosTex);
			//	const std::vector<VkVertexInputAttributeDescription> attributeDescriptions = GetAttributeDescriptions(EmVertexFormat::ePosTex);

			//	vertexInputInfo.vertexBindingDescriptionCount = 1;
			//	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
			//	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
			//	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

			//	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
			//	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
			//	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			//	inputAssembly.primitiveRestartEnable = VK_FALSE;

			//	VkRect2D scissor{};
			//	scissor.offset = { 0, 0 };
			//	scissor.extent = extend;

			//	VkPipelineViewportStateCreateInfo viewportState{};
			//	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
			//	viewportState.viewportCount = 1;
			//	viewportState.pViewports = pViewport;
			//	viewportState.scissorCount = 1;
			//	viewportState.pScissors = &scissor;

			//	VkPipelineRasterizationStateCreateInfo rasterizer{};
			//	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
			//	rasterizer.depthClampEnable = VK_FALSE;
			//	rasterizer.rasterizerDiscardEnable = VK_FALSE;
			//	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
			//	rasterizer.lineWidth = 1.0f;
			//	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
			//	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
			//	rasterizer.depthBiasEnable = VK_FALSE;

			//	VkPipelineMultisampleStateCreateInfo multisampling{};
			//	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
			//	multisampling.sampleShadingEnable = VK_FALSE;
			//	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

			//	VkPipelineDepthStencilStateCreateInfo depthStencil{};
			//	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
			//	depthStencil.depthTestEnable = VK_TRUE;
			//	depthStencil.depthWriteEnable = VK_TRUE;
			//	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
			//	depthStencil.depthBoundsTestEnable = VK_FALSE;
			//	depthStencil.stencilTestEnable = VK_FALSE;

			//	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
			//	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
			//	colorBlendAttachment.blendEnable = VK_FALSE;

			//	VkPipelineColorBlendStateCreateInfo colorBlending{};
			//	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
			//	colorBlending.logicOpEnable = VK_FALSE;
			//	colorBlending.logicOp = VK_LOGIC_OP_COPY;
			//	colorBlending.attachmentCount = 1;
			//	colorBlending.pAttachments = &colorBlendAttachment;
			//	colorBlending.blendConstants[0] = 0.0f;
			//	colorBlending.blendConstants[1] = 0.0f;
			//	colorBlending.blendConstants[2] = 0.0f;
			//	colorBlending.blendConstants[3] = 0.0f;

			//	const VkDescriptorSetLayout layouts[] = { m_descSetLayoutUniform, m_descSetLayoutTexture };

			//	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
			//	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			//	pipelineLayoutInfo.setLayoutCount = _countof(layouts);
			//	pipelineLayoutInfo.pSetLayouts = layouts;

			//	if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS)
			//	{
			//		throw_line("failed to create pipeline layout!");
			//	}

			//	const VkDynamicState dynamicStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

			//	VkPipelineDynamicStateCreateInfo dynamicStateInfo{};
			//	dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
			//	dynamicStateInfo.dynamicStateCount = _countof(dynamicStates);
			//	dynamicStateInfo.pDynamicStates = dynamicStates;

			//	VkGraphicsPipelineCreateInfo pipelineInfo{};
			//	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
			//	pipelineInfo.stageCount = 2;
			//	pipelineInfo.pStages = shaderStages;
			//	pipelineInfo.pVertexInputState = &vertexInputInfo;
			//	pipelineInfo.pInputAssemblyState = &inputAssembly;
			//	pipelineInfo.pViewportState = &viewportState;
			//	pipelineInfo.pRasterizationState = &rasterizer;
			//	pipelineInfo.pMultisampleState = &multisampling;
			//	pipelineInfo.pDepthStencilState = &depthStencil;
			//	pipelineInfo.pColorBlendState = &colorBlending;
			//	pipelineInfo.pDynamicState = &dynamicStateInfo;
			//	pipelineInfo.layout = m_pipelineLayout;
			//	pipelineInfo.renderPass = m_renderPass;
			//	pipelineInfo.subpass = 0;
			//	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

			//	if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_graphicsPipeline) != VK_SUCCESS)
			//	{
			//		throw_line("failed to create graphics pipeline!");
			//	}

			//	vkDestroyShaderModule(device, fragShaderModule, nullptr);
			//	vkDestroyShaderModule(device, vertShaderModule, nullptr);
			//}

			//void RenderManager::Impl::CreateDescriptorSetLayout()
			//{
			//	VkDevice device = Device::GetInstance()->GetInterface();

			//	{
			//		VkDescriptorSetLayoutBinding uboLayoutBinding{};
			//		uboLayoutBinding.binding = 0;
			//		uboLayoutBinding.descriptorCount = 1;
			//		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
			//		uboLayoutBinding.pImmutableSamplers = nullptr;
			//		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

			//		const VkDescriptorSetLayoutBinding bindings[] = { uboLayoutBinding };
			//		VkDescriptorSetLayoutCreateInfo layoutInfo{};
			//		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			//		layoutInfo.bindingCount = _countof(bindings);
			//		layoutInfo.pBindings = bindings;

			//		if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &m_descSetLayoutUniform) != VK_SUCCESS)
			//		{
			//			throw_line("failed to create descriptor set layout");
			//		}
			//	}

			//	VkDescriptorSetLayoutBinding samplerLayoutBinding{};
			//	samplerLayoutBinding.binding = 0;
			//	samplerLayoutBinding.descriptorCount = 1;
			//	samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			//	samplerLayoutBinding.pImmutableSamplers = nullptr;
			//	samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

			//	const VkDescriptorSetLayoutBinding bindings[] = { samplerLayoutBinding };
			//	VkDescriptorSetLayoutCreateInfo layoutInfo{};
			//	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			//	layoutInfo.bindingCount = _countof(bindings);
			//	layoutInfo.pBindings = bindings;

			//	if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &m_descSetLayoutTexture) != VK_SUCCESS)
			//	{
			//		throw_line("failed to create descriptor set layout");
			//	}
			//}

			//void RenderManager::Impl::CreateDescriptorPool()
			//{
			//	VkDevice device = Device::GetInstance()->GetInterface();

			//	std::array<VkDescriptorPoolSize, 2> poolSizes{};
			//	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
			//	poolSizes[0].descriptorCount = 1;
			//	poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			//	poolSizes[1].descriptorCount = eInstanceCount;

			//	VkDescriptorPoolCreateInfo poolInfo{};
			//	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			//	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
			//	poolInfo.pPoolSizes = poolSizes.data();
			//	poolInfo.maxSets = eInstanceCount + 1;

			//	if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &m_descriptorPool) != VK_SUCCESS)
			//	{
			//		throw_line("failed to create descriptor pool");
			//	}
			//}

			//void RenderManager::Impl::CreateDescriptorSet()
			//{
			//	VkDevice device = Device::GetInstance()->GetInterface();

			//	for (int i = 0; i < eInstanceCount; ++i)
			//	{
			//		VkDescriptorSetLayout layouts[] = { m_descSetLayoutTexture };
			//		VkDescriptorSetAllocateInfo allocInfo{};
			//		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			//		allocInfo.descriptorPool = m_descriptorPool;
			//		allocInfo.descriptorSetCount = 1;
			//		allocInfo.pSetLayouts = layouts;

			//		if (vkAllocateDescriptorSets(device, &allocInfo, &m_descriptorSets[i].set) != VK_SUCCESS)
			//		{
			//			throw_line("failed to allocate descriptor set");
			//		}
			//	}
			//}

			//void RenderManager::Impl::CreateSampler()
			//{
			//	VkDevice device = Device::GetInstance()->GetInterface();

			//	VkSamplerCreateInfo samplerInfo{};
			//	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
			//	samplerInfo.magFilter = VK_FILTER_LINEAR;
			//	samplerInfo.minFilter = VK_FILTER_LINEAR;
			//	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			//	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			//	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			//	samplerInfo.anisotropyEnable = VK_TRUE;
			//	samplerInfo.maxAnisotropy = 16;
			//	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
			//	samplerInfo.unnormalizedCoordinates = VK_FALSE;
			//	samplerInfo.compareEnable = VK_FALSE;
			//	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
			//	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

			//	if (vkCreateSampler(device, &samplerInfo, nullptr, &m_sampler) != VK_SUCCESS)
			//	{
			//		throw_line("failed to create texture sampler");
			//	}
			//}

			//void RenderManager::Impl::CreateUniformBuffer()
			//{
			//	VkDevice device = Device::GetInstance()->GetInterface();

			//	const VkDeviceSize bufferSize = eUniformBufferAlignedSize * eInstanceCount;
			//	Device::GetInstance()->CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &m_uniformBuffer, &m_uniformBufferMemory);

			//	VkDescriptorSetLayout layouts[] = { m_descSetLayoutUniform };
			//	VkDescriptorSetAllocateInfo allocInfo{};
			//	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			//	allocInfo.descriptorPool = m_descriptorPool;
			//	allocInfo.descriptorSetCount = 1;
			//	allocInfo.pSetLayouts = layouts;
			//	
			//	if (vkAllocateDescriptorSets(device, &allocInfo, &m_uniformBufferDescriptorSet) != VK_SUCCESS)
			//	{
			//		throw_line("failed to allocate descriptor set");
			//	}
			//	
			//	VkDescriptorBufferInfo bufferInfo{};
			//	bufferInfo.buffer = m_uniformBuffer;
			//	bufferInfo.offset = 0;
			//	bufferInfo.range = VK_WHOLE_SIZE;
			//	
			//	std::array<VkWriteDescriptorSet, 1> descriptorWrites{};
			//	descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			//	descriptorWrites[0].dstSet = m_uniformBufferDescriptorSet;
			//	descriptorWrites[0].dstBinding = 0;
			//	descriptorWrites[0].dstArrayElement = 0;
			//	descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
			//	descriptorWrites[0].descriptorCount = 1;
			//	descriptorWrites[0].pBufferInfo = &bufferInfo;
			//	
			//	vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);

			//	m_vecUniformBufferData.resize(bufferSize);
			//}

			//VkShaderModule RenderManager::Impl::CreateShaderModule(const std::vector<char>& code)
			//{
			//	VkDevice device = Device::GetInstance()->GetInterface();

			//	VkShaderModuleCreateInfo createInfo{};
			//	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			//	createInfo.codeSize = code.size();
			//	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

			//	VkShaderModule shaderModule{};
			//	if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
			//	{
			//		throw_line("failed to create shader module")
			//	}

			//	return shaderModule;
			//}

			//void RenderManager::Impl::Resize()
			//{
			//	const VkExtent2D& swapchainExtent = Device::GetInstance()->GetSwapChainExtent2D();
			//	const math::uint2& n2Size = m_pRenderTarget->GetSize();

			//	if (swapchainExtent.width == n2Size.x && swapchainExtent.height == n2Size.y)
			//		return;

			//	VkDevice device = Device::GetInstance()->GetInterface();
			//	m_pRenderTarget.reset();

			//	vkDestroyFramebuffer(device, m_frameBuffer, nullptr);
			//	vkDestroyRenderPass(device, m_renderPass, nullptr);

			//	CreateRenderTarget();
			//	CreateRenderPass();
			//	CreateFrameBuffer();
			//}

			RenderManager::RenderManager()
				: m_pImpl{ std::make_unique<Impl>() }
			{
			}

			RenderManager::~RenderManager()
			{
			}

			void RenderManager::Cleanup()
			{
			}

			void RenderManager::Render()
			{
				m_pImpl->Render();
			}

			void RenderManager::PushJob(const RenderJobStatic& renderJob)
			{
				m_pImpl->PushJob(renderJob);
			}

			void RenderManager::PushJob(const RenderJobSkinned& renderJob)
			{
				m_pImpl->PushJob(renderJob);
			}

			void RenderManager::PushJob(const RenderJobTerrain& renderJob)
			{
				m_pImpl->PushJob(renderJob);
			}

			void RenderManager::PushJob(const RenderJobVertex& renderJob)
			{
				m_pImpl->PushJob(renderJob);
			}
		}
	}
}