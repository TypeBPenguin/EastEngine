#include "stdafx.h"
#include "ModelRendererVulkan.h"

#include "CommonLib/FileStream.h"
#include "CommonLib/FileUtil.h"
#include "CommonLib/Lock.h"

#include "GraphicsInterface/Instancing.h"
#include "GraphicsInterface/Camera.h"

#include "UtilVulkan.h"
#include "DeviceVulkan.h"
#include "GBufferVulkan.h"

#include "TextureVulkan.h"
#include "VertexBufferVulkan.h"
#include "IndexBufferVulkan.h"

namespace eastengine
{
	namespace graphics
	{
		namespace vulkan
		{
			namespace modelshader
			{
				enum
				{
					eDrawInstanceCount = 1024,
				};

				struct SkinningInstancingDataBuffer
				{
					std::array<SkinningInstancingData, eDrawInstanceCount> data;
				};

				struct StaticInstancingDataBuffer
				{
					std::array<math::Matrix, eDrawInstanceCount> data;
				};

				struct ObjectDataBuffer
				{
					math::Matrix matWorld;

					uint32_t nVTFID{ 0 };
					math::Vector3 f3Padding;
				};

				struct VSConstantsBuffer
				{
					math::Matrix matViewProj;
				};

				struct MaterialData
				{
					math::Color f4AlbedoColor;
					math::Color f4EmissiveColor;

					math::Vector4 f4PaddingRoughMetEmi;
					math::Vector4 f4SurSpecTintAniso;
					math::Vector4 f4SheenTintClearcoatGloss;

					float fStippleTransparencyFactor{ 0.f };
					math::Vector3 f3Padding;
				};

				enum TextureBindingSlot
				{
					eTexBind_Albedo = 0,
					eTexBind_Mask,
					eTexBind_Normal,
					eTexBind_Roughness,
					eTexBind_Metallic,
					eTexBind_Emissive,
					eTexBind_EmissiveColor,
					eTexBind_Subsurface,
					eTexBind_Specular,
					eTexBind_SpecularTint,
					eTexBind_Anisotropic,
					eTexBind_Sheen,
					eTexBind_SheenTint,
					eTexBind_Clearcoat,
					eTexBind_ClearcoatGloss,

					eTexBindCount,
				};

				enum Mask : uint64_t
				{
					eUseTexAlbedo = 1 << 0,
					eUseTexMask = 1 << 1,
					eUseTexNormal = 1 << 2,
					eUseTexRoughness = 1 << 3,
					eUseTexMetallic = 1 << 4,
					eUseTexEmissive = 1 << 5,
					eUseTexEmissiveColor = 1 << 6,
					eUseTexSubsurface = 1 << 7,
					eUseTexSpecular = 1 << 8,
					eUseTexSpecularTint = 1 << 9,
					eUseTexAnisotropic = 1 << 10,
					eUseTexSheen = 1 << 11,
					eUseTexSheenTint = 1 << 12,
					eUseTexClearcoat = 1 << 13,
					eUseTexClearcoatGloss = 1 << 14,

					eUseInstancing = 1 << 15,
					eUseSkinning = 1 << 16,

					MaskCount = 17,
				};

				const char* GetMaskName(uint64_t nMask)
				{
					static std::string s_strMaskName[] =
					{
						"USE_TEX_ALBEDO",
						"USE_TEX_MASK",
						"USE_TEX_NORMAL",
						"USE_TEX_ROUGHNESS",
						"USE_TEX_METALLIC",
						"USE_TEX_EMISSIVE",
						"USE_TEX_EMISSIVECOLOR",
						"USE_TEX_SUBSURFACE",
						"USE_TEX_SPECULAR",
						"USE_TEX_SPECULARTINT",
						"USE_TEX_ANISOTROPIC",
						"USE_TEX_SHEEN",
						"USE_TEX_SHEENTINT",
						"USE_TEX_CLEARCOAT",
						"USE_TEX_CLEARCOATGLOSS",
						"USE_INSTANCING",
						"USE_SKINNING",
						"USE_WRITEDEPTH",
						"USE_CUBEMAP",
						"USE_TESSELLATION",
						"USE_ALBEDO_ALPHA_IS_MASK_MAP",
						"USE_ALPHABLENDING",
					};

					return s_strMaskName[nMask].c_str();
				}

				std::vector<ShaderMacro> GetMacros(uint64_t nMask)
				{
					std::vector<ShaderMacro> vecMacros;
					vecMacros.push_back({ "Vulkan", "1" });
					for (uint64_t i = 0; i < modelshader::MaskCount; ++i)
					{
						if ((nMask & (1llu << i)) != 0)
						{
							vecMacros.push_back({ GetMaskName(i), "1" });
						}
					}

					return vecMacros;
				}

				const char* GetVertexShaderFile()
				{
					static std::string strVertShaderPath;
					if (strVertShaderPath.empty() == false)
						return strVertShaderPath.c_str();

					strVertShaderPath = file::GetPath(file::eFx);
					strVertShaderPath.append("Model\\Model.vert");

					return strVertShaderPath.c_str();
				}

				const char* GetFragmentShaderFile()
				{
					static std::string strFragShaderPath;
					if (strFragShaderPath.empty() == false)
						return strFragShaderPath.c_str();

					strFragShaderPath = file::GetPath(file::eFx);
					strFragShaderPath.append("Model\\Model.frag");

					return strFragShaderPath.c_str();
				}
			}

			class ModelRenderer::Impl
			{
			public:
				Impl();
				~Impl();

			public:
				void Render(Camera* pCamera);
				void Flush();

			public:
				void PushJob(const RenderJobStatic& job);
				void PushJob(const RenderJobSkinned& job);

			private:
				enum PipelineLayouts
				{
					ePL_SkinningInstance = 0,
					ePL_StaticInstance,
					ePL_ObjectData,

					ePL_VSContents,

					ePL_MaterialDataUB,
					ePL_MaterialSampler2D,

					ePL_Count,
				};

				struct DescriptorSetSampler2D
				{
					VkDescriptorSet set{ nullptr };
					uint32_t nFrameIndex{ 0 };
					bool isUsing{ false };
				};

				void GetPSOState(const PSOKey& key, uint64_t& nMask_out, EmRasterizerState::Type& emRasterizerState_out, EmBlendState::Type& emBlendState_out, EmDepthStencilState::Type& emDepthStencilState_out);
				PSOKey GetPSOKey(uint64_t nMask, EmRasterizerState::Type emRasterizerState, EmBlendState::Type emBlendState, EmDepthStencilState::Type emDepthStencilState);

				void CreateRenderPass(VkDevice device);
				void CreateFrameBuffer(VkDevice device);
				void CreateDescriptorSetLayout(VkDevice device);
				void CreatePipelineLayout(VkDevice device, uint64_t nMask, EmRasterizerState::Type emRasterizerState, EmBlendState::Type emBlendState, EmDepthStencilState::Type emDepthStencilState);

				void CreateDescriptorPool(VkDevice device, uint32_t nFrameCount);
				void CreateDescriptorSet(VkDevice device, uint32_t nFrameCount);
				void CreateUniformBuffer(VkDevice device, uint32_t nFrameCount);

			private:
				thread::SRWLock m_srwLock;

				struct JobStatic
				{
					RenderJobStatic element;
					DescriptorSetSampler2D* pSet{ nullptr };

					JobStatic(const RenderJobStatic& job, DescriptorSetSampler2D* pSet)
						: element(job)
						, pSet(pSet)
					{
					}

					JobStatic(JobStatic&& source) noexcept
						: element(std::move(source.element))
						, pSet(std::move(source.pSet))
					{
					}

					void operator = (const JobStatic& source)
					{
						element = source.element;
						pSet = source.pSet;
					}
				};
				std::vector<JobStatic> m_vecStaticJobs;

				struct JobSkinned
				{
					RenderJobSkinned job;
					DescriptorSetSampler2D* pSet{ nullptr };

					JobSkinned(const RenderJobSkinned& job, DescriptorSetSampler2D* pSet)
						: job(job)
						, pSet(pSet)
					{
					}

					JobSkinned(JobSkinned&& source) noexcept
						: job(std::move(source.job))
						, pSet(std::move(source.pSet))
					{
					}

					void operator = (const JobSkinned& source)
					{
						job = source.job;
						pSet = source.pSet;
					}
				};
				std::vector<JobSkinned> m_vecSkinnedJobs;

				std::string m_strVertShaderCode;
				std::string m_strFragShaderCode;

				VkRenderPass m_renderPass{ nullptr };
				VkFramebuffer m_frameBuffer{ nullptr };
				std::array<VkDescriptorSetLayout, ePL_Count> m_descriptorSetLyaouts{ nullptr };
				VkDescriptorPool m_descriptorPool{ nullptr };

				std::vector<DescriptorSetSampler2D> m_vecDescriptorSetSampler2Ds;
				std::unordered_map<const void*, DescriptorSetSampler2D*> m_umapDescriptorSetCaching;

				std::vector<UniformBuffer<modelshader::SkinningInstancingDataBuffer>> m_vecSkinningInstancingBuffers;
				std::vector<UniformBuffer<modelshader::StaticInstancingDataBuffer>> m_vecStaticInstancingBuffers;
				std::vector<UniformBuffer<modelshader::ObjectDataBuffer>> m_vecObjectBuffers;
				std::vector<UniformBuffer<modelshader::VSConstantsBuffer>> m_vecVSContantsBuffers;
				std::vector<UniformBuffer<modelshader::MaterialData>> m_vecMaterialBuffers;

				struct RenderPipeline
				{
					VkPipelineLayout pipelineLayout{ nullptr };
					VkPipeline pipeline{ nullptr };
				};
				std::unordered_map<PSOKey, RenderPipeline> m_umapPipelineLayouts;
			};

			ModelRenderer::Impl::Impl()
			{
				m_strVertShaderCode = util::LoadShaderCode(modelshader::GetVertexShaderFile());
				m_strFragShaderCode = util::LoadShaderCode(modelshader::GetFragmentShaderFile());

				VkDevice device = Device::GetInstance()->GetInterface();
				uint32_t nFrameCount = Device::GetInstance()->GetFrameCount();

				CreateRenderPass(device);
				CreateFrameBuffer(device);
				CreateDescriptorSetLayout(device);

				CreateDescriptorPool(device, nFrameCount);
				CreateDescriptorSet(device, nFrameCount);
				CreateUniformBuffer(device, nFrameCount);

				CreatePipelineLayout(device, 0, EmRasterizerState::eSolidCCW, EmBlendState::eOff, EmDepthStencilState::eRead_Off_Write_On);
				CreatePipelineLayout(device, modelshader::eUseSkinning, EmRasterizerState::eSolidCCW, EmBlendState::eOff, EmDepthStencilState::eRead_Off_Write_On);
			}

			ModelRenderer::Impl::~Impl()
			{
				VkDevice device = Device::GetInstance()->GetInterface();

				for (auto& buffer : m_vecSkinningInstancingBuffers)
				{
					vkDestroyBuffer(device, buffer.uniformBuffer, nullptr);
					vkFreeMemory(device, buffer.uniformBufferMemory, nullptr);
				}
				m_vecSkinningInstancingBuffers.clear();

				for (auto& buffer : m_vecStaticInstancingBuffers)
				{
					vkDestroyBuffer(device, buffer.uniformBuffer, nullptr);
					vkFreeMemory(device, buffer.uniformBufferMemory, nullptr);
				}
				m_vecStaticInstancingBuffers.clear();

				for (auto& buffer : m_vecObjectBuffers)
				{
					vkDestroyBuffer(device, buffer.uniformBuffer, nullptr);
					vkFreeMemory(device, buffer.uniformBufferMemory, nullptr);
				}
				m_vecObjectBuffers.clear();

				for (auto& buffer : m_vecVSContantsBuffers)
				{
					vkDestroyBuffer(device, buffer.uniformBuffer, nullptr);
					vkFreeMemory(device, buffer.uniformBufferMemory, nullptr);
				}
				m_vecVSContantsBuffers.clear();

				for (auto& buffer : m_vecMaterialBuffers)
				{
					vkDestroyBuffer(device, buffer.uniformBuffer, nullptr);
					vkFreeMemory(device, buffer.uniformBufferMemory, nullptr);
				}
				m_vecMaterialBuffers.clear();

				m_vecDescriptorSetSampler2Ds.clear();

				for (auto& layout : m_descriptorSetLyaouts)
				{
					vkDestroyDescriptorSetLayout(device, layout, nullptr);
				}
				m_descriptorSetLyaouts.fill(nullptr);

				vkDestroyDescriptorPool(device, m_descriptorPool, nullptr);

				for (auto& iter : m_umapPipelineLayouts)
				{
					RenderPipeline& pipeline = iter.second;
					vkDestroyPipeline(device, pipeline.pipeline, nullptr);
					vkDestroyPipelineLayout(device, pipeline.pipelineLayout, nullptr);
				}
				m_umapPipelineLayouts.clear();

				vkDestroyFramebuffer(device, m_frameBuffer, nullptr);
				vkDestroyRenderPass(device, m_renderPass, nullptr);
			}

			void ModelRenderer::Impl::Render(Camera* pCamera)
			{
				auto IsValidTexture = [](const IMaterial* pMaterial, EmMaterial::Type emType) -> bool
				{
					if (pMaterial == nullptr)
						return false;

					ITexture* pTexture = pMaterial->GetTexture(emType);
					if (pTexture == nullptr)
						return false;

					return pTexture->GetState() == IResource::eComplete;
				};

				VkDevice device = Device::GetInstance()->GetInterface();
				const uint32_t nFrameIndex = Device::GetInstance()->GetFrameIndex();
				const VkViewport* pViewport = Device::GetInstance()->GetViewport();
				const VkExtent2D extent = Device::GetInstance()->GetSwapChainExtent2D();

				VkCommandBuffer commandBuffer = Device::GetInstance()->GetCommandBuffer(nFrameIndex);
				
				const Texture* pTextureEmpty = Device::GetInstance()->GetEmptyTexture();

				VkRenderPassBeginInfo renderPassInfo{};
				renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
				renderPassInfo.renderPass = m_renderPass;
				renderPassInfo.framebuffer = m_frameBuffer;
				renderPassInfo.renderArea.offset = { 0, 0 };
				renderPassInfo.renderArea.extent = extent;
				
				std::array<VkClearValue, 4> clearValues{};
				clearValues[EmGBuffer::eNormals].color = { 0.f, 0.f, 0.f, 0.f };
				clearValues[EmGBuffer::eColors].color = { 0.f, 0.f, 0.f, 0.f };
				clearValues[EmGBuffer::eDisneyBRDF].color = { 0.f, 0.f, 0.f, 0.f };
				clearValues[3].depthStencil = { 1.0f, 0 };
				
				renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
				renderPassInfo.pClearValues = clearValues.data();
				
				vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
				
				vkCmdSetViewport(commandBuffer, 0, 1, pViewport);
				
				VkRect2D scissor{};
				scissor.extent = extent;
				
				vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
				
				modelshader::VSConstantsBuffer* pVSContents = m_vecVSContantsBuffers[nFrameIndex].Cast(0);
				pVSContents->matViewProj = pCamera->GetViewMatrix() * pCamera->GetProjMatrix();

				// Flush to make changes visible to the host 
				VkMappedMemoryRange memoryRange{};
				memoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
				memoryRange.memory = m_vecVSContantsBuffers[nFrameIndex].uniformBufferMemory;
				memoryRange.size = m_vecVSContantsBuffers[nFrameIndex].Size();
				vkFlushMappedMemoryRanges(device, 1, &memoryRange);

				m_vecVSContantsBuffers[nFrameIndex];

				size_t nDescriptorIndex = 0;
				const size_t nDescriptorSize = m_vecDescriptorSetSampler2Ds.size();

				const size_t nSize = m_vecStaticJobs.size();
				for (size_t i = 0; i < nSize; ++i)
				{
					JobStatic& jobStatic = m_vecStaticJobs[i];
					if (jobStatic.pSet != nullptr)
					{
						jobStatic.pSet->nFrameIndex = nFrameIndex;
					}
					else if (jobStatic.element.pMaterial != nullptr)
					{
						auto iter = m_umapDescriptorSetCaching.find(jobStatic.element.pMaterial);
						if (iter != m_umapDescriptorSetCaching.end())
						{
							jobStatic.pSet = iter->second;
						}
						else
						{
							for (; nDescriptorIndex < nDescriptorSize; ++nDescriptorIndex)
							{
								DescriptorSetSampler2D& descriptorSetSampler2D = m_vecDescriptorSetSampler2Ds[nDescriptorIndex];
								if (descriptorSetSampler2D.isUsing == false)
								{
									descriptorSetSampler2D.isUsing = true;
									descriptorSetSampler2D.nFrameIndex = nFrameIndex;

									jobStatic.pSet = &descriptorSetSampler2D;
									m_umapDescriptorSetCaching.emplace(jobStatic.element.pMaterial, &descriptorSetSampler2D);

									VkSampler sampler = Device::GetInstance()->GetSampler(jobStatic.element.pMaterial->GetSamplerState());

									std::vector<VkDescriptorImageInfo> vecImageInfos;
									vecImageInfos.reserve(modelshader::eTexBindCount);

									auto SetImageInfo = [&](EmMaterial::Type emType)
									{
										if (IsValidTexture(jobStatic.element.pMaterial, emType) == true)
										{
											Texture* pTexture = static_cast<Texture*>(jobStatic.element.pMaterial->GetTexture(emType));

											VkDescriptorImageInfo& imageInfo = vecImageInfos.emplace_back();
											imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
											imageInfo.imageView = pTexture->GetImageView();
											imageInfo.sampler = sampler;
										}
										else
										{
											VkDescriptorImageInfo& imageInfo = vecImageInfos.emplace_back();
											imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
											imageInfo.imageView = pTextureEmpty->GetImageView();
											imageInfo.sampler = sampler;
										}
									};

									SetImageInfo(EmMaterial::eAlbedo);
									SetImageInfo(EmMaterial::eMask);
									SetImageInfo(EmMaterial::eNormal);
									SetImageInfo(EmMaterial::eRoughness);
									SetImageInfo(EmMaterial::eMetallic);
									SetImageInfo(EmMaterial::eEmissive);
									SetImageInfo(EmMaterial::eEmissiveColor);
									SetImageInfo(EmMaterial::eSubsurface);
									SetImageInfo(EmMaterial::eSpecular);
									SetImageInfo(EmMaterial::eSpecularTint);
									SetImageInfo(EmMaterial::eAnisotropic);
									SetImageInfo(EmMaterial::eSheen);
									SetImageInfo(EmMaterial::eSheenTint);
									SetImageInfo(EmMaterial::eClearcoat);
									SetImageInfo(EmMaterial::eClearcoatGloss);

									VkWriteDescriptorSet descriptorWrite{};
									descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
									descriptorWrite.dstSet = jobStatic.pSet->set;
									descriptorWrite.dstBinding = 0;
									descriptorWrite.dstArrayElement = 0;
									descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
									descriptorWrite.descriptorCount = modelshader::eTexBindCount;
									descriptorWrite.pImageInfo = vecImageInfos.data();

									vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);

									break;
								}
							}
						}
					}
				}

				struct RenderJob
				{
					const JobStatic* pRenderJob{ nullptr };
					uint32_t nJobIndex{ 0 };
					uint32_t nMaterialDescriptorIndex{ 0 };
				};
				std::unordered_map<PSOKey, std::vector<RenderJob>> umapRenderJobStatic;
				umapRenderJobStatic.rehash(m_vecStaticJobs.size() / 2);

				{
					uint32_t nMaterialDescriptorIndex = 0;

					for (size_t i = 0; i < nSize; ++i)
					{
						const JobStatic& jobStatic = m_vecStaticJobs[i];

						uint32_t nTextureCount = 0;

						uint64_t nMask = 0;
						EmRasterizerState::Type emRasterizerState = EmRasterizerState::eSolidCCW;
						EmBlendState::Type emBlendState = EmBlendState::eOff;
						EmDepthStencilState::Type emDepthStencilState = EmDepthStencilState::eRead_Write_On;
						if (jobStatic.element.pMaterial != nullptr)
						{
							auto SetMaterialMask = [&](const IMaterial* pMaterial, EmMaterial::Type emType, modelshader::Mask emMask)
							{
								if (IsValidTexture(pMaterial, emType) == true)
								{
									++nTextureCount;
									nMask |= emMask;
								}
							};

							SetMaterialMask(jobStatic.element.pMaterial, EmMaterial::eAlbedo, modelshader::eUseTexAlbedo);
							SetMaterialMask(jobStatic.element.pMaterial, EmMaterial::eMask, modelshader::eUseTexMask);
							SetMaterialMask(jobStatic.element.pMaterial, EmMaterial::eNormal, modelshader::eUseTexNormal);
							SetMaterialMask(jobStatic.element.pMaterial, EmMaterial::eRoughness, modelshader::eUseTexRoughness);
							SetMaterialMask(jobStatic.element.pMaterial, EmMaterial::eMetallic, modelshader::eUseTexMetallic);
							SetMaterialMask(jobStatic.element.pMaterial, EmMaterial::eEmissive, modelshader::eUseTexEmissive);
							SetMaterialMask(jobStatic.element.pMaterial, EmMaterial::eEmissiveColor, modelshader::eUseTexEmissiveColor);
							SetMaterialMask(jobStatic.element.pMaterial, EmMaterial::eSubsurface, modelshader::eUseTexSubsurface);
							SetMaterialMask(jobStatic.element.pMaterial, EmMaterial::eSpecular, modelshader::eUseTexSpecular);
							SetMaterialMask(jobStatic.element.pMaterial, EmMaterial::eSpecularTint, modelshader::eUseTexSpecularTint);
							SetMaterialMask(jobStatic.element.pMaterial, EmMaterial::eAnisotropic, modelshader::eUseTexAnisotropic);
							SetMaterialMask(jobStatic.element.pMaterial, EmMaterial::eSheen, modelshader::eUseTexSheen);
							SetMaterialMask(jobStatic.element.pMaterial, EmMaterial::eSheenTint, modelshader::eUseTexSheenTint);
							SetMaterialMask(jobStatic.element.pMaterial, EmMaterial::eClearcoat, modelshader::eUseTexClearcoat);
							SetMaterialMask(jobStatic.element.pMaterial, EmMaterial::eClearcoatGloss, modelshader::eUseTexClearcoatGloss);

							emRasterizerState = jobStatic.element.pMaterial->GetRasterizerState();
							emBlendState = jobStatic.element.pMaterial->GetBlendState();
							emDepthStencilState = jobStatic.element.pMaterial->GetDepthStencilState();
						}

						PSOKey key = GetPSOKey(nMask, emRasterizerState, emBlendState, emDepthStencilState);

						RenderJob& job = umapRenderJobStatic[key].emplace_back();
						job.pRenderJob = &m_vecStaticJobs[i];
						job.nJobIndex = static_cast<uint32_t>(i);
						job.nMaterialDescriptorIndex = nMaterialDescriptorIndex;
					}
				}

				{
					std::for_each(umapRenderJobStatic.begin(), umapRenderJobStatic.end(), [&](const std::pair<const PSOKey, std::vector<RenderJob>>& iter)
					//Concurrency::parallel_for_each(umapRenderJobStatic.begin(), umapRenderJobStatic.end(), [&](const std::pair<const uint64_t, std::vector<RenderJob>>& iter)
					{
						const RenderPipeline* pRenderPipeline = nullptr;

						uint64_t nMask = 0;
						EmRasterizerState::Type emRasterizerState = EmRasterizerState::eSolidCCW;
						EmBlendState::Type emBlendState = EmBlendState::eOff;
						EmDepthStencilState::Type emDepthStencilState = EmDepthStencilState::eRead_Write_On;
						GetPSOState(iter.first, nMask, emRasterizerState, emBlendState, emDepthStencilState);

						auto iter_find = m_umapPipelineLayouts.find(iter.first);
						if (iter_find != m_umapPipelineLayouts.end())
						{
							pRenderPipeline = &iter_find->second;
						}
						else
						{
							CreatePipelineLayout(device, nMask, emRasterizerState, emBlendState, emDepthStencilState);

							iter_find = m_umapPipelineLayouts.find(iter.first);
							if (iter_find != m_umapPipelineLayouts.end())
							{
								pRenderPipeline = &iter_find->second;
							}
						}

						if (pRenderPipeline == nullptr || pRenderPipeline->pipeline == nullptr || pRenderPipeline->pipelineLayout == nullptr)
							return;

						const std::vector<RenderJob>& vecRenderJobs = iter.second;
						if (vecRenderJobs.empty() == true)
							return;

						std::vector<VkDescriptorSet> vecDescriptorSets;

						if ((nMask & modelshader::eUseInstancing) == modelshader::eUseInstancing)
						{
							if ((nMask & modelshader::eUseSkinning) == modelshader::eUseSkinning)
							{
								vecDescriptorSets.emplace_back(m_vecSkinningInstancingBuffers[nFrameIndex].descriptorSet);
							}
							else
							{
								vecDescriptorSets.emplace_back(m_vecStaticInstancingBuffers[nFrameIndex].descriptorSet);
							}
						}
						else
						{
							vecDescriptorSets.emplace_back(m_vecObjectBuffers[nFrameIndex].descriptorSet);
						}

						vecDescriptorSets.emplace_back(m_vecVSContantsBuffers[nFrameIndex].descriptorSet);
						vecDescriptorSets.emplace_back(m_vecMaterialBuffers[nFrameIndex].descriptorSet);

						if ((nMask & modelshader::eUseTexAlbedo) == modelshader::eUseTexAlbedo ||
							(nMask & modelshader::eUseTexMask) == modelshader::eUseTexMask ||
							(nMask & modelshader::eUseTexNormal) == modelshader::eUseTexNormal ||
							(nMask & modelshader::eUseTexRoughness) == modelshader::eUseTexRoughness ||
							(nMask & modelshader::eUseTexMetallic) == modelshader::eUseTexMetallic ||
							(nMask & modelshader::eUseTexEmissive) == modelshader::eUseTexEmissive ||
							(nMask & modelshader::eUseTexEmissiveColor) == modelshader::eUseTexEmissiveColor ||
							(nMask & modelshader::eUseTexSubsurface) == modelshader::eUseTexSubsurface ||
							(nMask & modelshader::eUseTexSpecular) == modelshader::eUseTexSpecular ||
							(nMask & modelshader::eUseTexSpecularTint) == modelshader::eUseTexSpecularTint ||
							(nMask & modelshader::eUseTexAnisotropic) == modelshader::eUseTexAnisotropic ||
							(nMask & modelshader::eUseTexSheen) == modelshader::eUseTexSheen ||
							(nMask & modelshader::eUseTexSheenTint) == modelshader::eUseTexSheenTint ||
							(nMask & modelshader::eUseTexClearcoat) == modelshader::eUseTexClearcoat ||
							(nMask & modelshader::eUseTexClearcoatGloss) == modelshader::eUseTexClearcoatGloss)
						{
							if (vecRenderJobs[0].pRenderJob->pSet != nullptr)
							{
								vecDescriptorSets.emplace_back(vecRenderJobs[0].pRenderJob->pSet->set);
							}
						}

						vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pRenderPipeline->pipeline);

						const size_t nSize = vecRenderJobs.size();
						for (size_t i = 0; i < nSize; ++i)
						{
							const RenderJob& job = vecRenderJobs[i];
							const JobStatic* pJobStatic = job.pRenderJob;

							std::vector<uint32_t> vecDynamicOffsets;
							if ((nMask & modelshader::eUseInstancing) != modelshader::eUseInstancing)
							{
								modelshader::ObjectDataBuffer* pDataBuffer = m_vecObjectBuffers[nFrameIndex].Cast(job.nJobIndex);
								pDataBuffer->matWorld = pJobStatic->element.matWorld;

								vecDynamicOffsets.emplace_back(static_cast<uint32_t>(job.nJobIndex * m_vecObjectBuffers[nFrameIndex].Size()));
							}

							vecDynamicOffsets.emplace_back(static_cast<uint32_t>(job.nJobIndex * m_vecVSContantsBuffers[nFrameIndex].Size()));
							vecDynamicOffsets.emplace_back(static_cast<uint32_t>(job.nJobIndex * m_vecMaterialBuffers[nFrameIndex].Size()));

							const VertexBuffer* pVertexBuffer = static_cast<const VertexBuffer*>(pJobStatic->element.pVertexBuffer);
							const VkBuffer vertexBuffers[] = { pVertexBuffer->GetBuffer() };
							const VkDeviceSize offsets[] = { 0 };
							vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

							const IndexBuffer* pIndexBuffer = static_cast<const IndexBuffer*>(pJobStatic->element.pIndexBuffer);
							vkCmdBindIndexBuffer(commandBuffer, pIndexBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);

							vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pRenderPipeline->pipelineLayout, 0,
								static_cast<uint32_t>(vecDescriptorSets.size()), vecDescriptorSets.data(),
								static_cast<uint32_t>(vecDynamicOffsets.size()), vecDynamicOffsets.data());

							vkCmdDrawIndexed(commandBuffer, pJobStatic->element.pIndexBuffer->GetIndexCount(), 1, 0, 0, 0);
						}
					});

					vkCmdEndRenderPass(commandBuffer);
				}
			}

			void ModelRenderer::Impl::Flush()
			{
				const uint32_t nFrameIndex = Device::GetInstance()->GetFrameIndex();

				for (auto iter = m_umapDescriptorSetCaching.begin(); iter != m_umapDescriptorSetCaching.end();)
				{
					if (iter->second->nFrameIndex == nFrameIndex)
					{
						iter->second->isUsing = false;
						iter->second->nFrameIndex = std::numeric_limits<uint32_t>::max();
						iter = m_umapDescriptorSetCaching.erase(iter);
					}
					else
					{
						++iter;
					}
				}

				const size_t nSize = m_vecDescriptorSetSampler2Ds.size();
				for (size_t i = 0; i < nSize; ++i)
				{
					DescriptorSetSampler2D& set = m_vecDescriptorSetSampler2Ds[i];
					if (set.nFrameIndex == nFrameIndex)
					{
						set.isUsing = false;
						set.nFrameIndex = std::numeric_limits<uint32_t>::max();
					}
				}

				m_vecStaticJobs.clear();
				m_vecSkinnedJobs.clear();
			}

			void ModelRenderer::Impl::PushJob(const RenderJobStatic& job)
			{
				thread::SRWWriteLock writeLock(&m_srwLock);

				auto iter = m_umapDescriptorSetCaching.find(job.pMaterial);
				if (iter != m_umapDescriptorSetCaching.end())
				{
					iter->second->isUsing = true;
					m_vecStaticJobs.emplace_back(job, iter->second);
				}
			}

			void ModelRenderer::Impl::PushJob(const RenderJobSkinned& job)
			{
				thread::SRWWriteLock writeLock(&m_srwLock);

				auto iter = m_umapDescriptorSetCaching.find(job.pMaterial);
				if (iter != m_umapDescriptorSetCaching.end())
				{
					iter->second->isUsing = true;
					m_vecSkinnedJobs.emplace_back(job, iter->second);
				}
			}

			void ModelRenderer::Impl::GetPSOState(const PSOKey& key, uint64_t& nMask_out, EmRasterizerState::Type& emRasterizerState_out, EmBlendState::Type& emBlendState_out, EmDepthStencilState::Type& emDepthStencilState_out)
			{
				uint32_t r = 0;
				uint32_t b = 0;
				uint32_t d = 0;

				sscanf_s(key.Value().c_str(), "%llu_%u_%u_%u", &nMask_out, &r, &b, &d);

				emRasterizerState_out = static_cast<EmRasterizerState::Type>(r);
				emBlendState_out = static_cast<EmBlendState::Type>(b);
				emDepthStencilState_out = static_cast<EmDepthStencilState::Type>(d);
			}

			PSOKey ModelRenderer::Impl::GetPSOKey(uint64_t nMask, EmRasterizerState::Type emRasterizerState, EmBlendState::Type emBlendState, EmDepthStencilState::Type emDepthStencilState)
			{
				String::StringID strKey;
				strKey.Format("%lld_%d_%d_%d", nMask, emRasterizerState, emBlendState, emDepthStencilState);

				return PSOKey{ strKey };
			}

			void ModelRenderer::Impl::CreateRenderPass(VkDevice device)
			{
				GBuffer* pGBuffer = Device::GetInstance()->GetGBuffer(0);

				std::array<VkAttachmentDescription, EmGBuffer::Count + 1> attachments{};
				attachments[EmGBuffer::eNormals].format = pGBuffer->GetRenderTarget(EmGBuffer::eNormals)->GetFormat();
				attachments[EmGBuffer::eNormals].samples = VK_SAMPLE_COUNT_1_BIT;

				// loadOP
				// VK_ATTACHMENT_LOAD_OP_LOAD : 첨부 파일의 기존 내용을 보존한다.
				// VK_ATTACHMENT_LOAD_OP_CLEAR : 시작시 값을 상수로 지운다.
				// VK_ATTACHMENT_LOAD_OP_DONT_CARE : 기존 내용은 정의되지 않았습니다. 우리는 그들에 관심이 없다.(???)
				attachments[EmGBuffer::eNormals].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;

				// storeOp
				// VK_ATTACHMENT_STORE_OP_STORE : 렌더링 된 내용은 메모리에 저장되며 나중에 읽을 수 있다.
				// VK_ATTACHMENT_STORE_OP_DONT_CARE : 프레임 버퍼의 내용은 렌더링 작업 후에 정의되지 않습니다.(???)
				attachments[EmGBuffer::eNormals].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
				attachments[EmGBuffer::eNormals].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				attachments[EmGBuffer::eNormals].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

				// imageLayout
				// VK_IMAGE_LAYOUT_PRESENT_SRC_KHR : 프레젠테이션에 최적
				// VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : 색상 첨부로 사용되는 이미지, fragment 쉐이더에서 색상을 쓰는데 가장 적합
				// VK_IMAGE_LAYOUT_PRESENT_SCR_KHR : 스왑 체인에 표시할 이미지
				// VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL : 메모리 복사 작업의 대상으로 사용될 이미지
				// VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL : 전송 작업에서 원본처럼 최적(?)
				// VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL : 전송 작업의 대상으로 최적
				// VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : 쉐이더에서 샘플링에 최적
				attachments[EmGBuffer::eNormals].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				attachments[EmGBuffer::eNormals].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

				attachments[EmGBuffer::eColors] = attachments[EmGBuffer::eNormals];
				attachments[EmGBuffer::eColors].format = pGBuffer->GetRenderTarget(EmGBuffer::eColors)->GetFormat();

				attachments[EmGBuffer::eDisneyBRDF] = attachments[EmGBuffer::eNormals];
				attachments[EmGBuffer::eDisneyBRDF].format = pGBuffer->GetRenderTarget(EmGBuffer::eDisneyBRDF)->GetFormat();

				attachments[3].format = pGBuffer->GetDepthStencil()->GetFormat();
				attachments[3].samples = VK_SAMPLE_COUNT_1_BIT;
				attachments[3].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
				attachments[3].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
				attachments[3].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				attachments[3].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
				attachments[3].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				attachments[3].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

				std::array<VkAttachmentReference, EmGBuffer::Count> colorAttachmentRef{};
				colorAttachmentRef[EmGBuffer::eNormals] = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
				colorAttachmentRef[EmGBuffer::eColors] = { 1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
				colorAttachmentRef[EmGBuffer::eDisneyBRDF] = { 2, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };

				VkAttachmentReference depthAttachmentRef{};
				depthAttachmentRef.attachment = 3;
				depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

				VkSubpassDescription subpass{};
				subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
				subpass.pColorAttachments = colorAttachmentRef.data();
				subpass.colorAttachmentCount = static_cast<uint32_t>(colorAttachmentRef.size());
				subpass.pDepthStencilAttachment = &depthAttachmentRef;

				std::array<VkSubpassDependency, 2> dependencies;
				dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
				dependencies[0].dstSubpass = 0;
				dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
				dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
				dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
				dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

				dependencies[1].srcSubpass = 0;
				dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
				dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
				dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
				dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
				dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

				VkRenderPassCreateInfo renderPassInfo{};
				renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
				renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
				renderPassInfo.pAttachments = attachments.data();
				renderPassInfo.subpassCount = 1;
				renderPassInfo.pSubpasses = &subpass;
				renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
				renderPassInfo.pDependencies = dependencies.data();

				if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &m_renderPass) != VK_SUCCESS)
				{
					throw_line("failed to create render pass");
				}
			}

			void ModelRenderer::Impl::CreateFrameBuffer(VkDevice device)
			{
				GBuffer* pGBuffer = Device::GetInstance()->GetGBuffer(0);
				const VkExtent2D& swapchainExtent = Device::GetInstance()->GetSwapChainExtent2D();

				const VkImageView attachments[] =
				{
					pGBuffer->GetRenderTarget(EmGBuffer::eNormals)->GetImageView(),
					pGBuffer->GetRenderTarget(EmGBuffer::eColors)->GetImageView(),
					pGBuffer->GetRenderTarget(EmGBuffer::eDisneyBRDF)->GetImageView(),
					pGBuffer->GetDepthStencil()->GetImageView(),
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

			void ModelRenderer::Impl::CreateDescriptorSetLayout(VkDevice device)
			{
				// ePL_SkinningInstance
				{
					const VkDescriptorSetLayoutBinding layoutBindings[] =
					{
						// ubSkinningInstancingData
						{
							0,	// binding
							VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,	// descriptorType
							1,	// descriptorCount
							VK_SHADER_STAGE_VERTEX_BIT,	// stageFlags
							nullptr	// pImmutableSamplers
						},
						// g_texVTF
						{
							1,	// binding
							VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,	// descriptorType
							1,	// descriptorCount
							VK_SHADER_STAGE_VERTEX_BIT,	// stageFlags
							nullptr	// pImmutableSamplers
						},
					};

					m_descriptorSetLyaouts[ePL_SkinningInstance] = util::CreateDescriptorSetLayout(device, layoutBindings, _countof(layoutBindings));
				}

				// ePL_StaticInstance
				{
					const VkDescriptorSetLayoutBinding layoutBindings[] =
					{
						// ubStaticInstancingData
						{
							0,	// binding
							VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,	// descriptorType
							1,	// descriptorCount
							VK_SHADER_STAGE_VERTEX_BIT,	// stageFlags
							nullptr	// pImmutableSamplers
						},
					};

					m_descriptorSetLyaouts[ePL_StaticInstance] = util::CreateDescriptorSetLayout(device, layoutBindings, _countof(layoutBindings));
				}

				//ePL_ObjectData
				{
					const VkDescriptorSetLayoutBinding layoutBindings[] =
					{
						// ubObjectData
						{
							0,	// binding
							VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,	// descriptorType
							1,	// descriptorCount
							VK_SHADER_STAGE_VERTEX_BIT,	// stageFlags
							nullptr	// pImmutableSamplers
						},
					};

					m_descriptorSetLyaouts[ePL_ObjectData] = util::CreateDescriptorSetLayout(device, layoutBindings, _countof(layoutBindings));
				}

				//ePL_VSContents
				{
					const VkDescriptorSetLayoutBinding layoutBindings[] =
					{
						// ubVSConstants
						{
							0,	// binding
							VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,	// descriptorType
							1,	// descriptorCount
							VK_SHADER_STAGE_VERTEX_BIT,	// stageFlags
							nullptr	// pImmutableSamplers
						},
					};

					m_descriptorSetLyaouts[ePL_VSContents] = util::CreateDescriptorSetLayout(device, layoutBindings, _countof(layoutBindings));
				}

				// ePL_MaterialDataUB
				{
					const VkDescriptorSetLayoutBinding layoutBindings[] =
					{
						// ubMaterialData
						{
							0,	// binding
							VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,	// descriptorType
							1,	// descriptorCount
							VK_SHADER_STAGE_FRAGMENT_BIT,	// stageFlags
							nullptr	// pImmutableSamplers
						}
					};

					m_descriptorSetLyaouts[ePL_MaterialDataUB] = util::CreateDescriptorSetLayout(device, layoutBindings, _countof(layoutBindings));
				}

				// ePL_Material
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
						// g_texMask
						{
							1,	// binding
							VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,	// descriptorType
							1,	// descriptorCount
							VK_SHADER_STAGE_FRAGMENT_BIT,	// stageFlags
							nullptr	// pImmutableSamplers
						},
						// g_texNormalMap
						{
							2,	// binding
							VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,	// descriptorType
							1,	// descriptorCount
							VK_SHADER_STAGE_FRAGMENT_BIT,	// stageFlags
							nullptr	// pImmutableSamplers
						},
						// g_texRoughness
						{
							3,	// binding
							VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,	// descriptorType
							1,	// descriptorCount
							VK_SHADER_STAGE_FRAGMENT_BIT,	// stageFlags
							nullptr	// pImmutableSamplers
						},
						// g_texMetallic
						{
							4,	// binding
							VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,	// descriptorType
							1,	// descriptorCount
							VK_SHADER_STAGE_FRAGMENT_BIT,	// stageFlags
							nullptr	// pImmutableSamplers
						},
						// g_texEmissive
						{
							5,	// binding
							VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,	// descriptorType
							1,	// descriptorCount
							VK_SHADER_STAGE_FRAGMENT_BIT,	// stageFlags
							nullptr	// pImmutableSamplers
						},
						// g_texEmissiveColor
						{
							6,	// binding
							VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,	// descriptorType
							1,	// descriptorCount
							VK_SHADER_STAGE_FRAGMENT_BIT,	// stageFlags
							nullptr	// pImmutableSamplers
						},
						// g_texSurface
						{
							7,	// binding
							VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,	// descriptorType
							1,	// descriptorCount
							VK_SHADER_STAGE_FRAGMENT_BIT,	// stageFlags
							nullptr	// pImmutableSamplers
						},
						// g_texSpecular
						{
							8,	// binding
							VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,	// descriptorType
							1,	// descriptorCount
							VK_SHADER_STAGE_FRAGMENT_BIT,	// stageFlags
							nullptr	// pImmutableSamplers
						},
						// g_texSpecularTint
						{
							9,	// binding
							VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,	// descriptorType
							1,	// descriptorCount
							VK_SHADER_STAGE_FRAGMENT_BIT,	// stageFlags
							nullptr	// pImmutableSamplers
						},
						// g_texAnisotropic
						{
							10,	// binding
							VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,	// descriptorType
							1,	// descriptorCount
							VK_SHADER_STAGE_FRAGMENT_BIT,	// stageFlags
							nullptr	// pImmutableSamplers
						},
						// g_texSheen
						{
							11,	// binding
							VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,	// descriptorType
							1,	// descriptorCount
							VK_SHADER_STAGE_FRAGMENT_BIT,	// stageFlags
							nullptr	// pImmutableSamplers
						},
						// g_texSheenTint
						{
							12,	// binding
							VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,	// descriptorType
							1,	// descriptorCount
							VK_SHADER_STAGE_FRAGMENT_BIT,	// stageFlags
							nullptr	// pImmutableSamplers
						},
						// g_texClearcoat
						{
							13,	// binding
							VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,	// descriptorType
							1,	// descriptorCount
							VK_SHADER_STAGE_FRAGMENT_BIT,	// stageFlags
							nullptr	// pImmutableSamplers
						},
						// g_texClearcoatGloss
						{
							14,	// binding
							VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,	// descriptorType
							1,	// descriptorCount
							VK_SHADER_STAGE_FRAGMENT_BIT,	// stageFlags
							nullptr	// pImmutableSamplers
						},
					};

					m_descriptorSetLyaouts[ePL_MaterialSampler2D] = util::CreateDescriptorSetLayout(device, layoutBindings, _countof(layoutBindings));
				}
			}

			void ModelRenderer::Impl::CreatePipelineLayout(VkDevice device, uint64_t nMask, EmRasterizerState::Type emRasterizerState, EmBlendState::Type emBlendState, EmDepthStencilState::Type emDepthStencilState)
			{
				VkExtent2D extend = Device::GetInstance()->GetSwapChainExtent2D();
				const VkViewport* pViewport = Device::GetInstance()->GetViewport();

				std::vector<ShaderMacro> vecVertMacros = modelshader::GetMacros(0);
				std::vector<uint32_t> vecCompiledVertShaderCode = util::CompileShader(CompileShaderType::eVertexShader, m_strVertShaderCode.c_str(), m_strVertShaderCode.size(), vecVertMacros.data(), vecVertMacros.size(), modelshader::GetVertexShaderFile());
				VkShaderModule vertShaderModule = util::CreateShaderModule(device, vecCompiledVertShaderCode.data(), vecCompiledVertShaderCode.size());

				std::vector<ShaderMacro> vecFragMacros = modelshader::GetMacros(0);
				std::vector<uint32_t> vecCompiledFragShaderCode = util::CompileShader(CompileShaderType::eFragmentShader, m_strFragShaderCode.c_str(), m_strFragShaderCode.size(), vecFragMacros.data(), vecFragMacros.size(), modelshader::GetFragmentShaderFile());
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

				VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
				vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

				VkVertexInputBindingDescription bindingDescription{};
				if ((nMask & modelshader::eUseSkinning) == modelshader::eUseSkinning)
				{
					util::GetVertexBindingDescription(VertexPosTexNorWeiIdx::Format(), &bindingDescription);
					util::GetVertexAttributeDescriptions(VertexPosTexNorWeiIdx::Format(), &vertexInputInfo.pVertexAttributeDescriptions, &vertexInputInfo.vertexAttributeDescriptionCount);
				}
				else
				{
					util::GetVertexBindingDescription(VertexPosTexNor::Format(), &bindingDescription);
					util::GetVertexAttributeDescriptions(VertexPosTexNor::Format(), &vertexInputInfo.pVertexAttributeDescriptions, &vertexInputInfo.vertexAttributeDescriptionCount);
				}
				vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
				vertexInputInfo.vertexBindingDescriptionCount = 1;

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

				VkPipelineRasterizationStateCreateInfo rasterizer = util::GetRasterizerCreateInfo(emRasterizerState);
				VkPipelineDepthStencilStateCreateInfo depthStencil = util::GetDepthStencilCreateInfo(emDepthStencilState);

				const VkPipelineColorBlendAttachmentState colorBlendAttachments[]
				{
					util::GetBlendAttachmentState(emBlendState),
					util::GetBlendAttachmentState(emBlendState),
					util::GetBlendAttachmentState(emBlendState),
				};
				VkPipelineColorBlendStateCreateInfo colorBlending = util::GetBlendCreateInfo(colorBlendAttachments, _countof(colorBlendAttachments));

				std::vector<VkDescriptorSetLayout> vecLayouts;

				if ((nMask & modelshader::eUseInstancing) == modelshader::eUseInstancing)
				{
					if ((nMask & modelshader::eUseSkinning) == modelshader::eUseSkinning)
					{
						vecLayouts.emplace_back(m_descriptorSetLyaouts[ePL_SkinningInstance]);
					}
					else
					{
						vecLayouts.emplace_back(m_descriptorSetLyaouts[ePL_StaticInstance]);
					}
				}
				else
				{
					vecLayouts.emplace_back(m_descriptorSetLyaouts[ePL_ObjectData]);
				}

				vecLayouts.emplace_back(m_descriptorSetLyaouts[ePL_VSContents]);

				vecLayouts.emplace_back(m_descriptorSetLyaouts[ePL_MaterialDataUB]);

				if ((nMask & modelshader::eUseTexAlbedo) == modelshader::eUseTexAlbedo ||
					(nMask & modelshader::eUseTexMask) == modelshader::eUseTexMask ||
					(nMask & modelshader::eUseTexNormal) == modelshader::eUseTexNormal ||
					(nMask & modelshader::eUseTexRoughness) == modelshader::eUseTexRoughness ||
					(nMask & modelshader::eUseTexMetallic) == modelshader::eUseTexMetallic ||
					(nMask & modelshader::eUseTexEmissive) == modelshader::eUseTexEmissive ||
					(nMask & modelshader::eUseTexEmissiveColor) == modelshader::eUseTexEmissiveColor ||
					(nMask & modelshader::eUseTexSubsurface) == modelshader::eUseTexSubsurface ||
					(nMask & modelshader::eUseTexSpecular) == modelshader::eUseTexSpecular ||
					(nMask & modelshader::eUseTexSpecularTint) == modelshader::eUseTexSpecularTint ||
					(nMask & modelshader::eUseTexAnisotropic) == modelshader::eUseTexAnisotropic ||
					(nMask & modelshader::eUseTexSheen) == modelshader::eUseTexSheen ||
					(nMask & modelshader::eUseTexSheenTint) == modelshader::eUseTexSheenTint ||
					(nMask & modelshader::eUseTexClearcoat) == modelshader::eUseTexClearcoat ||
					(nMask & modelshader::eUseTexClearcoatGloss) == modelshader::eUseTexClearcoatGloss)
				{
					vecLayouts.emplace_back(m_descriptorSetLyaouts[ePL_MaterialSampler2D]);
				}

				VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
				pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
				pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(vecLayouts.size());
				pipelineLayoutInfo.pSetLayouts = vecLayouts.data();

				VkPipelineLayout pipelineLayout = nullptr;
				if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
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
				pipelineInfo.pVertexInputState = &vertexInputInfo;
				pipelineInfo.pInputAssemblyState = &inputAssembly;
				pipelineInfo.pViewportState = &viewportState;
				pipelineInfo.pRasterizationState = &rasterizer;
				pipelineInfo.pMultisampleState = &multisampling;
				pipelineInfo.pDepthStencilState = &depthStencil;
				pipelineInfo.pColorBlendState = &colorBlending;
				pipelineInfo.pDynamicState = &dynamicStateInfo;
				pipelineInfo.layout = pipelineLayout;
				pipelineInfo.renderPass = m_renderPass;
				pipelineInfo.subpass = 0;
				pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

				VkPipeline pipeline{ nullptr };
				if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS)
				{
					throw_line("failed to create graphics pipeline!");
				}

				PSOKey key = GetPSOKey(nMask, emRasterizerState, emBlendState, emDepthStencilState);
				RenderPipeline& renderPipeline = m_umapPipelineLayouts[key];
				renderPipeline.pipelineLayout = pipelineLayout;
				renderPipeline.pipeline = pipeline;

				vkDestroyShaderModule(device, fragShaderModule, nullptr);
				vkDestroyShaderModule(device, vertShaderModule, nullptr);
			}

			void ModelRenderer::Impl::CreateDescriptorPool(VkDevice device, uint32_t nFrameCount)
			{
				enum
				{
					eVTF_Texture = 1,
					eUniformBuffer = 1,
					eDynamicUniformBuffer = 4,
				};

				const VkDescriptorPoolSize poolSizes[] =
				{
					{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, nFrameCount * eUniformBuffer },
					{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, nFrameCount * eDynamicUniformBuffer },
					{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, nFrameCount * (modelshader::eDrawInstanceCount * modelshader::eTexBindCount + eVTF_Texture) },
				};

				VkDescriptorPoolCreateInfo poolInfo{};
				poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
				poolInfo.poolSizeCount = _countof(poolSizes);
				poolInfo.pPoolSizes = poolSizes;
				poolInfo.maxSets = nFrameCount * ((modelshader::eDrawInstanceCount * modelshader::eTexBindCount) + eVTF_Texture + eUniformBuffer + eDynamicUniformBuffer);

				if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &m_descriptorPool) != VK_SUCCESS)
				{
					throw_line("failed to create descriptor pool");
				}
			}

			void ModelRenderer::Impl::CreateDescriptorSet(VkDevice device, uint32_t nFrameCount)
			{
				m_vecDescriptorSetSampler2Ds.resize(modelshader::eDrawInstanceCount * nFrameCount);

				VkDescriptorSetAllocateInfo allocInfo{};
				allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
				allocInfo.descriptorPool = m_descriptorPool;
				allocInfo.descriptorSetCount = 1;
				allocInfo.pSetLayouts = &m_descriptorSetLyaouts[ePL_MaterialSampler2D];

				const size_t nSize = m_vecDescriptorSetSampler2Ds.size();
				for (size_t i = 0; i < nSize; ++i)
				{
					if (vkAllocateDescriptorSets(device, &allocInfo, &m_vecDescriptorSetSampler2Ds[i].set) != VK_SUCCESS)
					{
						throw_line("failed to allocate descriptor set");
					}
				}
			}

			void ModelRenderer::Impl::CreateUniformBuffer(VkDevice device, uint32_t nFrameCount)
			{
				m_vecSkinningInstancingBuffers.resize(nFrameCount);
				m_vecStaticInstancingBuffers.resize(nFrameCount);
				m_vecObjectBuffers.resize(nFrameCount);
				m_vecVSContantsBuffers.resize(nFrameCount);
				m_vecMaterialBuffers.resize(nFrameCount);

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
					Device::GetInstance()->CreateBuffer(m_vecSkinningInstancingBuffers[i].Size(),
						VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
						VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
						&m_vecSkinningInstancingBuffers[i].uniformBuffer, &m_vecSkinningInstancingBuffers[i].uniformBufferMemory,
						reinterpret_cast<void**>(&m_vecSkinningInstancingBuffers[i].pUniformBufferData));

					Device::GetInstance()->CreateBuffer(m_vecStaticInstancingBuffers[i].Size(),
						VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
						VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
						&m_vecStaticInstancingBuffers[i].uniformBuffer, &m_vecStaticInstancingBuffers[i].uniformBufferMemory,
						reinterpret_cast<void**>(&m_vecStaticInstancingBuffers[i].pUniformBufferData));

					Device::GetInstance()->CreateBuffer(m_vecObjectBuffers[i].Size(),
						VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
						VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
						&m_vecObjectBuffers[i].uniformBuffer, &m_vecObjectBuffers[i].uniformBufferMemory,
						reinterpret_cast<void**>(&m_vecObjectBuffers[i].pUniformBufferData));

					Device::GetInstance()->CreateBuffer(m_vecVSContantsBuffers[i].Size(),
						VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
						VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
						&m_vecVSContantsBuffers[i].uniformBuffer, &m_vecVSContantsBuffers[i].uniformBufferMemory,
						reinterpret_cast<void**>(&m_vecVSContantsBuffers[i].pUniformBufferData));

					Device::GetInstance()->CreateBuffer(m_vecMaterialBuffers[i].Size(),
						VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
						VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
						&m_vecMaterialBuffers[i].uniformBuffer, &m_vecMaterialBuffers[i].uniformBufferMemory,
						reinterpret_cast<void**>(&m_vecMaterialBuffers[i].pUniformBufferData));

					AllocateDescriptorSet(0, &m_descriptorSetLyaouts[ePL_SkinningInstance], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, &m_vecSkinningInstancingBuffers[i].descriptorSet, m_vecSkinningInstancingBuffers[i].uniformBuffer);
					AllocateDescriptorSet(0, &m_descriptorSetLyaouts[ePL_StaticInstance], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, &m_vecStaticInstancingBuffers[i].descriptorSet, m_vecStaticInstancingBuffers[i].uniformBuffer);
					AllocateDescriptorSet(0, &m_descriptorSetLyaouts[ePL_ObjectData], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, &m_vecObjectBuffers[i].descriptorSet, m_vecObjectBuffers[i].uniformBuffer);
					AllocateDescriptorSet(0, &m_descriptorSetLyaouts[ePL_VSContents], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, &m_vecVSContantsBuffers[i].descriptorSet, m_vecVSContantsBuffers[i].uniformBuffer);
					AllocateDescriptorSet(0, &m_descriptorSetLyaouts[ePL_MaterialDataUB], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, &m_vecMaterialBuffers[i].descriptorSet, m_vecMaterialBuffers[i].uniformBuffer);
				}
			}

			ModelRenderer::ModelRenderer()
				: m_pImpl{ std::make_unique<Impl>() }
			{
			}

			ModelRenderer::~ModelRenderer()
			{
			}

			void ModelRenderer::Render(Camera* pCamera)
			{
				m_pImpl->Render(pCamera);
			}

			void ModelRenderer::Flush()
			{
				m_pImpl->Flush();
			}

			void ModelRenderer::PushJob(const RenderJobStatic& renderJob)
			{
				m_pImpl->PushJob(renderJob);
			}

			void ModelRenderer::PushJob(const RenderJobSkinned& renderJob)
			{
				m_pImpl->PushJob(renderJob);
			}
		}
	}
}