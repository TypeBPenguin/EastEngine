#include "stdafx.h"
#include "DeferredRendererVulkan.h"

#include "CommonLib/FileUtil.h"

#include "GraphicsInterface/Camera.h"
#include "GraphicsInterface/LightManager.h"

#include "UtilVulkan.h"
#include "DeviceVulkan.h"
#include "GBufferVulkan.h"

#include "TextureVulkan.h"

namespace eastengine
{
	namespace graphics
	{
		class Camera;

		namespace vulkan
		{
			namespace deferredshader
			{
				enum Binding
				{
					eBind_TexDepth,
					eBind_TexNormal,
					eBind_TexAlbedoSpecular,
					eBind_TexDisneyBRDF,

					eBind_TexDiffuseHDR,
					eBind_TexSpecularHDR,
					eBind_TexSpecularBRDF,

					eBind_UniformLightContents,
					eBind_UniformCommonContents,
				};

				struct LightContents
				{
					uint32_t nDirectionalLightCount{ 0 };
					uint32_t nPointLightCount{ 0 };
					uint32_t nSpotLightCount{ 0 };
					uint32_t padding{ 0 };

					//std::array<DirectionalLightData, ILight::eMaxDirectionalLightCount> lightDirectional{};
					//std::array<PointLightData, ILight::eMaxPointLightCount> lightPoint{};
					//std::array<SpotLightData, ILight::eMaxSpotLightCount> lightSpot{};
				};

				struct CommonContents
				{
					math::Matrix matInvView;
					math::Matrix matInvProj;

					math::float3 f3CameraPos;
					int nEnableShadowCount{ 0 };
				};

				std::vector<ShaderMacro> GetMacros(uint64_t nMask)
				{
					std::vector<ShaderMacro> vecMacros;
					vecMacros.push_back({ "Vulkan", "1" });

					return vecMacros;
				}

				const char* GetVertexShaderFile()
				{
					static std::string strVertShaderPath;
					if (strVertShaderPath.empty() == false)
						return strVertShaderPath.c_str();

					strVertShaderPath = file::GetPath(file::eFx);
					strVertShaderPath.append("FullScnreeQuad.vert");

					return strVertShaderPath.c_str();
				}

				const char* GetFragmentShaderFile()
				{
					static std::string strFragShaderPath;
					if (strFragShaderPath.empty() == false)
						return strFragShaderPath.c_str();

					strFragShaderPath = file::GetPath(file::eFx);
					strFragShaderPath.append("Model\\Deferred.frag");

					return strFragShaderPath.c_str();
				}
			}

			class DeferredRenderer::Impl
			{
			public:
				Impl();
				~Impl();

			public:
				void Render(Camera* pCamera);
				void Cleanup();

			private:
				enum PipelineLayouts
				{
					ePL_Sampler2D = 0,
					ePL_LightContents,
					ePL_CommonContents,

					ePL_Count,
				};

				void CreateRenderPass(VkDevice device);
				void CreateFrameBuffer(VkDevice device);
				void CreateDescriptorSetLayout(VkDevice device);
				void CreatePipelineLayout(VkDevice device);

				void CreateDescriptorPool(VkDevice device, uint32_t nFrameCount);
				void CreateDescriptorSet(VkDevice device, uint32_t nFrameCount);
				void CreateUniformBuffer(VkDevice device, uint32_t nFrameCount);

			private:
				VkRenderPass m_renderPass{ nullptr };
				VkFramebuffer m_frameBuffer{ nullptr };
				std::array<VkDescriptorSetLayout, ePL_Count> m_descriptorSetLayouts{ nullptr };

				VkPipelineLayout m_pipelineLayout{ nullptr };
				VkPipeline m_pipeline{ nullptr };

				VkDescriptorPool m_descriptorPool{ nullptr };

				std::vector<VkDescriptorSet> m_vecDescriptorSets;

				std::unique_ptr<ImageBuffer> m_pRenderTarget;

				std::vector<UniformBuffer<deferredshader::LightContents>> m_vecLightContentsBuffers;
				std::vector<UniformBuffer<deferredshader::CommonContents>> m_vecCommonContentsBuffers;
			};

			DeferredRenderer::Impl::Impl()
			{
				VkDevice device = Device::GetInstance()->GetInterface();
				const uint32_t nFrameCount = Device::GetInstance()->GetFrameCount();

				CreateRenderPass(device);
				CreateFrameBuffer(device);
				CreateDescriptorSetLayout(device);

				CreateDescriptorPool(device, nFrameCount);
				CreateDescriptorSet(device, nFrameCount);
				CreateUniformBuffer(device, nFrameCount);

				CreatePipelineLayout(device);
			}

			DeferredRenderer::Impl::~Impl()
			{
				VkDevice device = Device::GetInstance()->GetInterface();

				for (auto& buffer : m_vecLightContentsBuffers)
				{
					vkDestroyBuffer(device, buffer.uniformBuffer, nullptr);
					vkFreeMemory(device, buffer.uniformBufferMemory, nullptr);
				}
				m_vecLightContentsBuffers.clear();

				for (auto& buffer : m_vecCommonContentsBuffers)
				{
					vkDestroyBuffer(device, buffer.uniformBuffer, nullptr);
					vkFreeMemory(device, buffer.uniformBufferMemory, nullptr);
				}
				m_vecCommonContentsBuffers.clear();

				m_vecDescriptorSets.clear();

				for (auto& layout : m_descriptorSetLayouts)
				{
					vkDestroyDescriptorSetLayout(device, layout, nullptr);
				}
				vkDestroyDescriptorPool(device, m_descriptorPool, nullptr);

				vkDestroyPipeline(device, m_pipeline, nullptr);
				vkDestroyPipelineLayout(device, m_pipelineLayout, nullptr);

				vkDestroyFramebuffer(device, m_frameBuffer, nullptr);
				vkDestroyRenderPass(device, m_renderPass, nullptr);
			}

			void DeferredRenderer::Impl::Render(Camera* pCamera)
			{
				LightManager* pLightManager = LightManager::GetInstance();

				VkDevice device = Device::GetInstance()->GetInterface();
				const uint32_t nFrameIndex = Device::GetInstance()->GetFrameIndex();
				const VkViewport* pViewport = Device::GetInstance()->GetViewport();
				const VkExtent2D extent = Device::GetInstance()->GetSwapChainExtent2D();

				const GBuffer* pGBuffer = Device::GetInstance()->GetGBuffer(0);
				const IImageBasedLight* pImageBasedLight = Device::GetInstance()->GetImageBasedLight();
				const VkSampler samplerPointClamp = Device::GetInstance()->GetSampler(EmSamplerState::eMinMagMipPointClamp);
				const VkSampler samplerLinearClamp = Device::GetInstance()->GetSampler(EmSamplerState::eMinMagMipLinearClamp);

				VkCommandBuffer commandBuffer = Device::GetInstance()->GetCommandBuffer(nFrameIndex);

				VkRenderPassBeginInfo renderPassInfo{};
				renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
				renderPassInfo.renderPass = m_renderPass;
				renderPassInfo.framebuffer = m_frameBuffer;
				renderPassInfo.renderArea.offset = { 0, 0 };
				renderPassInfo.renderArea.extent = extent;

				VkClearValue clearValue{};
				clearValue.color = { 0.f, 0.f, 0.f, 0.f };

				renderPassInfo.clearValueCount = 1;
				renderPassInfo.pClearValues = &clearValue;

				vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

				vkCmdSetViewport(commandBuffer, 0, 1, pViewport);

				VkRect2D scissor{};
				scissor.extent = extent;

				vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

				{
					deferredshader::CommonContents* pCommonContents = m_vecCommonContentsBuffers[nFrameIndex].Cast(0);
					pCommonContents->matInvView = pCamera->GetViewMatrix().Invert();
					pCommonContents->matInvProj = pCamera->GetProjMatrix().Invert();

					pCommonContents->f3CameraPos = pCamera->GetPosition();
					pCommonContents->nEnableShadowCount = 0;

					// Flush to make changes visible to the host 
					VkMappedMemoryRange memoryRange{};
					memoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
					memoryRange.memory = m_vecCommonContentsBuffers[nFrameIndex].uniformBufferMemory;
					memoryRange.size = m_vecCommonContentsBuffers[nFrameIndex].Size();
					vkFlushMappedMemoryRanges(device, 1, &memoryRange);
				}

				{
					deferredshader::LightContents* pLightContents = m_vecLightContentsBuffers[nFrameIndex].Cast(0);

					const DirectionalLightData* pDirectionalLightData = nullptr;
					pLightManager->GetDirectionalLightData(&pDirectionalLightData, &pLightContents->nDirectionalLightCount);
					//Memory::Copy(pLightContents->lightDirectional.data(), sizeof(pLightContents->lightDirectional), pDirectionalLightData, sizeof(DirectionalLightData) * pLightContents->nDirectionalLightCount);

					const PointLightData* pPointLightData = nullptr;
					pLightManager->GetPointLightData(&pPointLightData, &pLightContents->nPointLightCount);
					//Memory::Copy(pLightContents->lightPoint.data(), sizeof(pLightContents->lightPoint), pPointLightData, sizeof(PointLightData) * pLightContents->nPointLightCount);

					const SpotLightData* pSpotLightData = nullptr;
					pLightManager->GetSpotLightData(&pSpotLightData, &pLightContents->nSpotLightCount);
					//Memory::Copy(pLightContents->lightSpot.data(), sizeof(pLightContents->lightSpot), pSpotLightData, sizeof(SpotLightData) * pLightContents->nSpotLightCount);

					// Flush to make changes visible to the host 
					VkMappedMemoryRange memoryRange{};
					memoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
					memoryRange.memory = m_vecLightContentsBuffers[nFrameIndex].uniformBufferMemory;
					memoryRange.size = m_vecLightContentsBuffers[nFrameIndex].Size();
					vkFlushMappedMemoryRanges(device, 1, &memoryRange);
				}

				auto SetImageInfo = [&](VkImageView imageView, VkSampler sampler, deferredshader::Binding emBinding)
				{
					VkDescriptorImageInfo imageInfo{};
					imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
					imageInfo.imageView = imageView;
					imageInfo.sampler = sampler;

					VkWriteDescriptorSet descriptorWrite{};
					descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
					descriptorWrite.dstSet = m_vecDescriptorSets[nFrameIndex];
					descriptorWrite.dstBinding = emBinding;
					descriptorWrite.dstArrayElement = 0;
					descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
					descriptorWrite.descriptorCount = 1;
					descriptorWrite.pImageInfo = &imageInfo;

					vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
				};

				SetImageInfo(pGBuffer->GetDepthStencil()->GetImageView(), samplerPointClamp, deferredshader::eBind_TexDepth);
				SetImageInfo(pGBuffer->GetRenderTarget(EmGBuffer::eNormals)->GetImageView(), samplerPointClamp, deferredshader::eBind_TexNormal);
				SetImageInfo(pGBuffer->GetRenderTarget(EmGBuffer::eColors)->GetImageView(), samplerPointClamp, deferredshader::eBind_TexAlbedoSpecular);
				SetImageInfo(pGBuffer->GetRenderTarget(EmGBuffer::eDisneyBRDF)->GetImageView(), samplerPointClamp, deferredshader::eBind_TexDisneyBRDF);

				Texture* pDiffuseHDR = static_cast<Texture*>(pImageBasedLight->GetDiffuseHDR());
				SetImageInfo(pDiffuseHDR->GetImageView(), samplerLinearClamp, deferredshader::eBind_TexDiffuseHDR);

				Texture* pSpecularHDR = static_cast<Texture*>(pImageBasedLight->GetSpecularHDR());
				SetImageInfo(pSpecularHDR->GetImageView(), samplerLinearClamp, deferredshader::eBind_TexSpecularHDR);

				Texture* pSpecularBRDF = static_cast<Texture*>(pImageBasedLight->GetSpecularBRDF());
				SetImageInfo(pSpecularBRDF->GetImageView(), samplerLinearClamp, deferredshader::eBind_TexSpecularBRDF);

				vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);

				vkCmdBindVertexBuffers(commandBuffer, 0, 0, nullptr, nullptr);
				vkCmdBindIndexBuffer(commandBuffer, nullptr, 0, VK_INDEX_TYPE_UINT32);

				const VkDescriptorSet descriptorSets[] =
				{
					m_vecDescriptorSets[nFrameIndex],
					m_vecLightContentsBuffers[nFrameIndex].descriptorSet,
					m_vecCommonContentsBuffers[nFrameIndex].descriptorSet
				};
				vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, _countof(descriptorSets), descriptorSets, 0, nullptr);

				vkCmdDraw(commandBuffer, 3, 1, 0, 0);

				vkCmdEndRenderPass(commandBuffer);
			}

			void DeferredRenderer::Impl::Cleanup()
			{
			}

			void DeferredRenderer::Impl::CreateRenderPass(VkDevice device)
			{
				VkAttachmentDescription attachment{};
				attachment.format = VK_FORMAT_R8G8B8A8_UNORM;
				attachment.samples = VK_SAMPLE_COUNT_1_BIT;
				attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
				attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
				attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
				attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				attachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

				VkAttachmentReference colorAttachmentRef{};
				colorAttachmentRef.attachment = 0;
				colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

				VkSubpassDescription subpass{};
				subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
				subpass.pColorAttachments = &colorAttachmentRef;
				subpass.colorAttachmentCount = 1;
				subpass.pDepthStencilAttachment = nullptr;

				VkSubpassDependency dependency{};
				dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
				dependency.dstSubpass = 0;
				dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				dependency.srcAccessMask = 0;
				dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

				VkRenderPassCreateInfo renderPassInfo{};
				renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
				renderPassInfo.attachmentCount = 1;
				renderPassInfo.pAttachments = &attachment;
				renderPassInfo.subpassCount = 1;
				renderPassInfo.pSubpasses = &subpass;
				renderPassInfo.dependencyCount = 1;
				renderPassInfo.pDependencies = &dependency;

				if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &m_renderPass) != VK_SUCCESS)
				{
					throw_line("failed to create render pass");
				}
			}

			void DeferredRenderer::Impl::CreateFrameBuffer(VkDevice device)
			{
				const VkExtent2D& swapchainExtent = Device::GetInstance()->GetSwapChainExtent2D();

				math::uint2 n2Size(swapchainExtent.width, swapchainExtent.height);
				m_pRenderTarget = std::make_unique<ImageBuffer>(n2Size, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);

				const VkImageView attachments[] =
				{
					m_pRenderTarget->GetImageView(),
				};

				VkFramebufferCreateInfo frameBufferInfo{};
				frameBufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
				frameBufferInfo.renderPass = m_renderPass;
				frameBufferInfo.attachmentCount = _countof(attachments);
				frameBufferInfo.pAttachments = attachments;
				frameBufferInfo.width = swapchainExtent.width;
				frameBufferInfo.height = swapchainExtent.height;
				frameBufferInfo.layers = 1;

				if (vkCreateFramebuffer(device, &frameBufferInfo, nullptr, &m_frameBuffer) != VK_SUCCESS)
				{
					throw_line("failed to create framebuffer");
				}
			}

			void DeferredRenderer::Impl::CreateDescriptorSetLayout(VkDevice device)
			{
				{
					const VkDescriptorSetLayoutBinding layoutBindings[] =
					{
						// g_texAlbedo
						{
							0,	// binding
							VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,	// descriptorType
							1,	// descriptorCount
							VK_SHADER_STAGE_FRAGMENT_BIT,	// stageFlags
							nullptr	// pImmutableSamplers
						},
						// g_texNormal
						{
							1,	// binding
							VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,	// descriptorType
							1,	// descriptorCount
							VK_SHADER_STAGE_FRAGMENT_BIT,	// stageFlags
							nullptr	// pImmutableSamplers
						},
						// g_texAlbedoSpecular
						{
							2,	// binding
							VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,	// descriptorType
							1,	// descriptorCount
							VK_SHADER_STAGE_FRAGMENT_BIT,	// stageFlags
							nullptr	// pImmutableSamplers
						},
						// g_texDisneyBRDF
						{
							3,	// binding
							VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,	// descriptorType
							1,	// descriptorCount
							VK_SHADER_STAGE_FRAGMENT_BIT,	// stageFlags
							nullptr	// pImmutableSamplers
						},
						// g_texDiffuseHDR
						{
							4,	// binding
							VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,	// descriptorType
							1,	// descriptorCount
							VK_SHADER_STAGE_FRAGMENT_BIT,	// stageFlags
							nullptr	// pImmutableSamplers
						},
						// g_texSpecularHDR
						{
							5,	// binding
							VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,	// descriptorType
							1,	// descriptorCount
							VK_SHADER_STAGE_FRAGMENT_BIT,	// stageFlags
							nullptr	// pImmutableSamplers
						},
						// g_texSpecularBRDF
						{
							6,	// binding
							VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,	// descriptorType
							1,	// descriptorCount
							VK_SHADER_STAGE_FRAGMENT_BIT,	// stageFlags
							nullptr	// pImmutableSamplers
						},
					};

					m_descriptorSetLayouts[ePL_Sampler2D] = util::CreateDescriptorSetLayout(device, layoutBindings, _countof(layoutBindings));
				}

				{
					const VkDescriptorSetLayoutBinding layoutBindings[] =
					{
						// ubLightContents
						{
							0,	// binding
							VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,	// descriptorType
							1,	// descriptorCount
							VK_SHADER_STAGE_FRAGMENT_BIT,	// stageFlags
							nullptr	// pImmutableSamplers
						},
					};

					m_descriptorSetLayouts[ePL_LightContents] = util::CreateDescriptorSetLayout(device, layoutBindings, _countof(layoutBindings));
				}

				{
					const VkDescriptorSetLayoutBinding layoutBindings[] =
					{
						// ubCommonContents
						{
							0,	// binding
							VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,	// descriptorType
							1,	// descriptorCount
							VK_SHADER_STAGE_FRAGMENT_BIT,	// stageFlags
							nullptr	// pImmutableSamplers
						},
					};

					m_descriptorSetLayouts[ePL_CommonContents] = util::CreateDescriptorSetLayout(device, layoutBindings, _countof(layoutBindings));
				}
			}

			void DeferredRenderer::Impl::CreatePipelineLayout(VkDevice device)
			{
				VkExtent2D extend = Device::GetInstance()->GetSwapChainExtent2D();
				const VkViewport* pViewport = Device::GetInstance()->GetViewport();

				std::string strVertShaderCode = util::LoadShaderCode(deferredshader::GetVertexShaderFile());
				std::string strFragShaderCode = util::LoadShaderCode(deferredshader::GetFragmentShaderFile());

				std::vector<ShaderMacro> vecVertMacros = deferredshader::GetMacros(0);
				std::vector<uint32_t> vecCompiledVertShaderCode = util::CompileShader(CompileShaderType::eVertexShader, strVertShaderCode.c_str(), strVertShaderCode.size(), vecVertMacros.data(), vecVertMacros.size(), deferredshader::GetVertexShaderFile());
				VkShaderModule vertShaderModule = util::CreateShaderModule(device, vecCompiledVertShaderCode.data(), vecCompiledVertShaderCode.size());

				std::vector<ShaderMacro> vecFragMacros = deferredshader::GetMacros(0);
				std::vector<uint32_t> vecCompiledFragShaderCode = util::CompileShader(CompileShaderType::eFragmentShader, strFragShaderCode.c_str(), strFragShaderCode.size(), vecFragMacros.data(), vecFragMacros.size(), deferredshader::GetFragmentShaderFile());
				VkShaderModule fragShaderModule = util::CreateShaderModule(device, vecCompiledFragShaderCode.data(), vecCompiledFragShaderCode.size());

				VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
				vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
				vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
				vertShaderStageInfo.module = vertShaderModule;
				vertShaderStageInfo.pName = "main";

				VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
				fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
				fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
				fragShaderStageInfo.module = fragShaderModule;
				fragShaderStageInfo.pName = "main";

				const VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

				VkPipelineVertexInputStateCreateInfo emptyInputState{};
				emptyInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
				emptyInputState.vertexAttributeDescriptionCount = 0;
				emptyInputState.pVertexAttributeDescriptions = nullptr;
				emptyInputState.vertexBindingDescriptionCount = 0;
				emptyInputState.pVertexBindingDescriptions = nullptr;

				VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
				inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
				inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
				inputAssembly.primitiveRestartEnable = VK_FALSE;

				VkRect2D scissor{};
				scissor.offset = { 0, 0 };
				scissor.extent = extend;

				VkPipelineViewportStateCreateInfo viewportState{};
				viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
				viewportState.viewportCount = 1;
				viewportState.pViewports = pViewport;
				viewportState.scissorCount = 1;
				viewportState.pScissors = &scissor;

				VkPipelineMultisampleStateCreateInfo multisampling{};
				multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
				multisampling.sampleShadingEnable = VK_FALSE;
				multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

				VkPipelineRasterizationStateCreateInfo rasterizer
				{
					VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,	// sType
					nullptr,	// pNext
					0,	// flags
					VK_FALSE,	// depthClampEnable
					VK_FALSE,	// rasterizerDiscardEnable
					VK_POLYGON_MODE_FILL,	// polygonMode
					VK_CULL_MODE_FRONT_BIT,	// cullMode
					VK_FRONT_FACE_CLOCKWISE,	// frontFace,
					VK_FALSE,	// depthBiasEnable
					0.f,	// depthBiasConstantFactor
					0.f,	// depthBiasClamp
					0.f,	// depthBiasSlopeFactor
					1.f	// lineWidth
				};
				VkPipelineDepthStencilStateCreateInfo depthStencil = util::GetDepthStencilCreateInfo(EmDepthStencilState::eRead_Write_Off);

				VkPipelineColorBlendAttachmentState colorBlendAttachment = util::GetBlendAttachmentState(EmBlendState::eOff);
				VkPipelineColorBlendStateCreateInfo colorBlending = util::GetBlendCreateInfo(&colorBlendAttachment, 1);

				VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
				pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
				pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(m_descriptorSetLayouts.size());
				pipelineLayoutInfo.pSetLayouts = m_descriptorSetLayouts.data();

				if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS)
				{
					throw_line("failed to create pipeline layout!");
				}

				const VkDynamicState dynamicStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

				VkPipelineDynamicStateCreateInfo dynamicStateInfo{};
				dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
				dynamicStateInfo.dynamicStateCount = _countof(dynamicStates);
				dynamicStateInfo.pDynamicStates = dynamicStates;

				VkGraphicsPipelineCreateInfo pipelineInfo{};
				pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
				pipelineInfo.stageCount = _countof(shaderStages);
				pipelineInfo.pStages = shaderStages;
				pipelineInfo.pVertexInputState = &emptyInputState;
				pipelineInfo.pInputAssemblyState = &inputAssembly;
				pipelineInfo.pViewportState = &viewportState;
				pipelineInfo.pRasterizationState = &rasterizer;
				pipelineInfo.pMultisampleState = &multisampling;
				pipelineInfo.pDepthStencilState = &depthStencil;
				pipelineInfo.pColorBlendState = &colorBlending;
				pipelineInfo.pDynamicState = &dynamicStateInfo;
				pipelineInfo.layout = m_pipelineLayout;
				pipelineInfo.renderPass = m_renderPass;
				pipelineInfo.subpass = 0;
				pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

				if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_pipeline) != VK_SUCCESS)
				{
					throw_line("failed to create graphics pipeline!");
				}

				vkDestroyShaderModule(device, fragShaderModule, nullptr);
				vkDestroyShaderModule(device, vertShaderModule, nullptr);
			}

			void DeferredRenderer::Impl::CreateDescriptorPool(VkDevice device, uint32_t nFrameCount)
			{
				const VkDescriptorPoolSize poolSizes[] =
				{
					{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, nFrameCount * 2 },
					{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, nFrameCount * (4 + 3) },
				};

				VkDescriptorPoolCreateInfo poolInfo{};
				poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
				poolInfo.poolSizeCount = _countof(poolSizes);
				poolInfo.pPoolSizes = poolSizes;
				poolInfo.maxSets = nFrameCount * (2 + 4 + 3);

				if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &m_descriptorPool) != VK_SUCCESS)
				{
					throw_line("failed to create descriptor pool");
				}
			}

			void DeferredRenderer::Impl::CreateDescriptorSet(VkDevice device, uint32_t nFrameCount)
			{
				m_vecDescriptorSets.resize(nFrameCount);

				VkDescriptorSetAllocateInfo allocInfo{};
				allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
				allocInfo.descriptorPool = m_descriptorPool;
				allocInfo.descriptorSetCount = 1;
				allocInfo.pSetLayouts = &m_descriptorSetLayouts[ePL_Sampler2D];

				const size_t nSize = m_vecDescriptorSets.size();
				for (size_t i = 0; i < nSize; ++i)
				{
					if (vkAllocateDescriptorSets(device, &allocInfo, &m_vecDescriptorSets[i]) != VK_SUCCESS)
					{
						throw_line("failed to allocate descriptor set");
					}
				}
			}

			void DeferredRenderer::Impl::CreateUniformBuffer(VkDevice device, uint32_t nFrameCount)
			{
				m_vecLightContentsBuffers.resize(nFrameCount);
				m_vecCommonContentsBuffers.resize(nFrameCount);

				auto AllocateDescriptorSet = [&](uint32_t nBinding, const VkDescriptorSetLayout* pSetLayouts, VkDescriptorType descriptorType, VkDescriptorSet* ppDescriptorSets, VkBuffer uniformBuffer)
				{
					VkDescriptorSetAllocateInfo allocInfo{};
					allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
					allocInfo.descriptorPool = m_descriptorPool;
					allocInfo.descriptorSetCount = 1;
					allocInfo.pSetLayouts = pSetLayouts;

					if (vkAllocateDescriptorSets(device, &allocInfo, ppDescriptorSets) != VK_SUCCESS)
					{
						throw_line("failed to allocate descriptor set");
					}

					VkDescriptorBufferInfo bufferInfo{};
					bufferInfo.buffer = uniformBuffer;
					bufferInfo.offset = 0;
					bufferInfo.range = VK_WHOLE_SIZE;

					std::array<VkWriteDescriptorSet, 1> descriptorWrites{};
					descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
					descriptorWrites[0].dstSet = *ppDescriptorSets;
					descriptorWrites[0].dstBinding = nBinding;
					descriptorWrites[0].dstArrayElement = 0;
					descriptorWrites[0].descriptorType = descriptorType;
					descriptorWrites[0].descriptorCount = 1;
					descriptorWrites[0].pBufferInfo = &bufferInfo;

					vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
				};

				for (uint32_t i = 0; i < nFrameCount; ++i)
				{
					Device::GetInstance()->CreateBuffer(m_vecLightContentsBuffers[i].Size(),
						VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
						VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
						&m_vecLightContentsBuffers[i].uniformBuffer, &m_vecLightContentsBuffers[i].uniformBufferMemory,
						reinterpret_cast<void**>(&m_vecLightContentsBuffers[i].pUniformBufferData));

					Device::GetInstance()->CreateBuffer(m_vecCommonContentsBuffers[i].Size(),
						VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
						VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
						&m_vecCommonContentsBuffers[i].uniformBuffer, &m_vecCommonContentsBuffers[i].uniformBufferMemory,
						reinterpret_cast<void**>(&m_vecCommonContentsBuffers[i].pUniformBufferData));

					AllocateDescriptorSet(0, &m_descriptorSetLayouts[ePL_LightContents], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, &m_vecLightContentsBuffers[i].descriptorSet, m_vecLightContentsBuffers[i].uniformBuffer);
					AllocateDescriptorSet(0, &m_descriptorSetLayouts[ePL_CommonContents], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, &m_vecCommonContentsBuffers[i].descriptorSet, m_vecCommonContentsBuffers[i].uniformBuffer);
				}
			}

			DeferredRenderer::DeferredRenderer()
				: m_pImpl{ std::make_unique<Impl>() }
			{
			}

			DeferredRenderer::~DeferredRenderer()
			{
			}

			void DeferredRenderer::Render(Camera* pCamera)
			{
				m_pImpl->Render(pCamera);
			}

			void DeferredRenderer::Cleanup()
			{
				m_pImpl->Cleanup();
			}
		}
	}
}