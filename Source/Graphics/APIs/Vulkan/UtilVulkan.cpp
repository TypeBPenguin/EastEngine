#include "stdafx.h"
#include "UtilVulkan.h"

#include "CommonLib/FileUtil.h"
#include "CommonLib/FileStream.h"

//#include "shaderc/shaderc.hpp"

namespace est
{
	namespace graphics
	{
		namespace vulkan
		{
			//static_assert(CompileShaderType::eVertexShader == shaderc_glsl_vertex_shader, "mismatch compile shader type");
			//static_assert(CompileShaderType::eFragmentShader == shaderc_glsl_fragment_shader, "mismatch compile shader type");
			//static_assert(CompileShaderType::eComputeShader == shaderc_glsl_compute_shader, "mismatch compile shader type");
			//static_assert(CompileShaderType::eGeometryShader == shaderc_glsl_geometry_shader, "mismatch compile shader type");
			//static_assert(CompileShaderType::eTessControl == shaderc_glsl_tess_control_shader, "mismatch compile shader type");
			//static_assert(CompileShaderType::eTessEvaluation == shaderc_glsl_tess_evaluation_shader, "mismatch compile shader type");

			namespace util
			{
				//class FileIncluder : public shaderc::CompileOptions::IncluderInterface
				//{
				//public:
				//	FileIncluder(const char* filePath)
				//		: m_filePath(file::GetFilePath(filePath))
				//	{
				//	}
				//
				//	virtual ~FileIncluder() = default;
				//
				//private:
				//	struct IncludeFileInfo
				//	{
				//		std::string code;
				//	};
				//
				//public:
				//	virtual shaderc_include_result* GetInclude(const char* requested_source, shaderc_include_type type, const char* requesting_source, size_t include_depth) override
				//	{
				//		std::string filePath = m_filePath;
				//		filePath.append(requested_source);
				//
				//		file::Stream file;
				//		if (file.Open(filePath.c_str()) == false)
				//		{
				//			std::string error = string::Format("not exists file : %s", filePath.c_str());
				//			throw_line(error.c_str());
				//		}
				//
				//		IncludeFileInfo* pFileInfo = new IncludeFileInfo;
				//		pFileInfo->code.resize(file.GetFileSize());
				//		file.Read(pFileInfo->code.data(), static_cast<uint32_t>(file.GetFileSize()));
				//
				//		pFileInfo->code.resize(strlen(pFileInfo->code.c_str()));
				//
				//		return new shaderc_include_result
				//		{
				//			requested_source, strlen(requested_source),
				//			pFileInfo->code.c_str(), pFileInfo->code.size(),
				//			pFileInfo
				//		};
				//	}
				//
				//	virtual void ReleaseInclude(shaderc_include_result* pIncludeResult) override
				//	{
				//		IncludeFileInfo* pFileInfo = static_cast<IncludeFileInfo*>(pIncludeResult->user_data);
				//		SafeDelete(pFileInfo);
				//		SafeDelete(pIncludeResult);
				//	}
				//
				//private:
				//	const std::string m_filePath;
				//};

				std::string LoadShaderCode(const wchar_t* filePath)
				{
					file::Stream fileVertShader;
					if (fileVertShader.Open(filePath) == false)
					{
						std::string error = string::Format("failed to open file : %s", filePath);
						throw_line(error.c_str());
					}

					std::string strShaderCode;
					strShaderCode.resize(static_cast<size_t>(fileVertShader.GetFileSize()));
					fileVertShader.Read(strShaderCode.data(), static_cast<uint32_t>(strShaderCode.size()));

					if (strShaderCode.empty() == true)
					{
						throw_line("error : empty vert shader code");
					}
					strShaderCode.resize(strlen(strShaderCode.c_str()));

					return strShaderCode;
				}

				std::vector<uint32_t> CompileShader(CompileShaderType emCompileShaderType, const char* sourceText, size_t sourceTextSize, const ShaderMacro* pShaderMacros, size_t nMacroCount, const wchar_t* filePath)
				{
//					const std::array<shaderc_shader_kind, CompileShaderTypeCount> shaderKind =
//					{
//						shaderc_glsl_vertex_shader,
//						shaderc_glsl_fragment_shader,
//						shaderc_glsl_compute_shader,
//						shaderc_glsl_geometry_shader,
//						shaderc_glsl_tess_control_shader,
//						shaderc_glsl_tess_evaluation_shader,
//					};
//
//					shaderc::CompileOptions options;
//
//					for (size_t i = 0; i < nMacroCount; ++i)
//					{
//						options.AddMacroDefinition(pShaderMacros[i].Name, pShaderMacros[i].Definition);
//					}
//
//					options.SetIncluder(std::make_unique<FileIncluder>(filePath));
//					options.SetSourceLanguage(shaderc_source_language_glsl);
//
//#if defined( DEBUG ) || defined( _DEBUG )
//					options.SetOptimizationLevel(shaderc_optimization_level_zero);
//					options.SetGenerateDebugInfo();
//#else
//					options.SetOptimizationLevel(shaderc_optimization_level_size);
//#endif
//
//					// google shaderc 라이브러리에 메모리릭이 존재..
//					// https://github.com/google/shaderc/issues/356 참조
//					// 추후 수정 예정이라고하니 나중을 기다려보자
//					// 아니면 여기 나와있는 것 처럼 고쳐보자
//					shaderc::Compiler compiler;
//					shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(sourceText, sourceTextSize, shaderKind[emCompileShaderType], filePath, options);
//
//					if (module.GetCompilationStatus() != shaderc_compilation_status_success)
//					{
//						throw_line(module.GetErrorMessage().c_str());
//					}
//
//					return { module.cbegin(), module.cend() };
					return {};
				}

				VkShaderModule CreateShaderModule(VkDevice device, const uint32_t* pShaderCode, size_t nCodeSize)
				{
					VkShaderModuleCreateInfo createInfo{};
					createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
					createInfo.codeSize = nCodeSize * sizeof(uint32_t);
					createInfo.pCode = pShaderCode;

					VkShaderModule shaderModule{ nullptr };
					if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
					{
						throw_line("failed to create shader module")
					}

					return shaderModule;
				}

				VkDescriptorSetLayout CreateDescriptorSetLayout(VkDevice device, const VkDescriptorSetLayoutBinding* pDescriptorSetLayoutBinding, size_t nDescriptorSetLayoutBindingCount)
				{
					VkDescriptorSetLayoutCreateInfo layoutInfo{};
					layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
					layoutInfo.pBindings = pDescriptorSetLayoutBinding;
					layoutInfo.bindingCount = static_cast<uint32_t>(nDescriptorSetLayoutBindingCount);

					VkDescriptorSetLayout descriptorSetLayout = nullptr;
					if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
					{
						throw_line("failed to create descriptor set layout");
					}

					return descriptorSetLayout;
				}

				void GetVertexBindingDescription(EmVertexFormat::Type emVertexFormat, VkVertexInputBindingDescription* pInputBindingDescription_out)
				{
					if (pInputBindingDescription_out == nullptr)
						return;

					pInputBindingDescription_out->binding = 0;
					pInputBindingDescription_out->stride = static_cast<uint32_t>(GetVertexFormatSize(emVertexFormat));
					pInputBindingDescription_out->inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
				}

				void GetVertexAttributeDescriptions(EmVertexFormat::Type emVertexFormat, const VkVertexInputAttributeDescription** ppAttributeDescriptions_out, uint32_t* pAttributeDescriptionsCount_out)
				{
					switch (emVertexFormat)
					{
					case EmVertexFormat::ePos:
					{
						static const VkVertexInputAttributeDescription attributeDescriptions[] =
						{
							{ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexPos, pos) },
						};

						if (ppAttributeDescriptions_out != nullptr)
						{
							*ppAttributeDescriptions_out = attributeDescriptions;
						}

						if (pAttributeDescriptionsCount_out != nullptr)
						{
							*pAttributeDescriptionsCount_out = _countof(attributeDescriptions);
						}
					}
					break;
					case EmVertexFormat::ePos4:
					{
						static const VkVertexInputAttributeDescription attributeDescriptions[] =
						{
							{ 0, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(VertexPos4, pos) },
						};

						if (ppAttributeDescriptions_out != nullptr)
						{
							*ppAttributeDescriptions_out = attributeDescriptions;
						}

						if (pAttributeDescriptionsCount_out != nullptr)
						{
							*pAttributeDescriptionsCount_out = _countof(attributeDescriptions);
						}
					}
					break;
					case EmVertexFormat::ePosCol:
					{
						static const VkVertexInputAttributeDescription attributeDescriptions[] =
						{
							{ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexPosCol, pos) },
							{ 1, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(VertexPosCol, color) },
						};

						if (ppAttributeDescriptions_out != nullptr)
						{
							*ppAttributeDescriptions_out = attributeDescriptions;
						}

						if (pAttributeDescriptionsCount_out != nullptr)
						{
							*pAttributeDescriptionsCount_out = _countof(attributeDescriptions);
						}
					}
					break;
					case EmVertexFormat::ePosTex:
					{
						static const VkVertexInputAttributeDescription attributeDescriptions[] =
						{
							{ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexPosTex, pos) },
							{ 1, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(VertexPosTex, uv) },
						};

						if (ppAttributeDescriptions_out != nullptr)
						{
							*ppAttributeDescriptions_out = attributeDescriptions;
						}

						if (pAttributeDescriptionsCount_out != nullptr)
						{
							*pAttributeDescriptionsCount_out = _countof(attributeDescriptions);
						}
					}
					break;
					case EmVertexFormat::ePosTexCol:
					{
						static const VkVertexInputAttributeDescription attributeDescriptions[] =
						{
							{ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexPosTexCol, pos) },
							{ 1, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(VertexPosTexCol, uv) },
							{ 2, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(VertexPosTexCol, color) },
						};

						if (ppAttributeDescriptions_out != nullptr)
						{
							*ppAttributeDescriptions_out = attributeDescriptions;
						}

						if (pAttributeDescriptionsCount_out != nullptr)
						{
							*pAttributeDescriptionsCount_out = _countof(attributeDescriptions);
						}
					}
					break;
					case EmVertexFormat::ePosTexNor:
					{
						static const VkVertexInputAttributeDescription attributeDescriptions[] =
						{
							{ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexPosTexNor, pos) },
							{ 1, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(VertexPosTexNor, uv) },
							{ 2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexPosTexNor, normal) },
						};

						if (ppAttributeDescriptions_out != nullptr)
						{
							*ppAttributeDescriptions_out = attributeDescriptions;
						}

						if (pAttributeDescriptionsCount_out != nullptr)
						{
							*pAttributeDescriptionsCount_out = _countof(attributeDescriptions);
						}
					}
					break;
					case EmVertexFormat::ePosTexNorCol:
					{
						static const VkVertexInputAttributeDescription attributeDescriptions[] =
						{
							{ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexPosTexNorCol, pos) },
							{ 1, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(VertexPosTexNorCol, uv) },
							{ 2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexPosTexNorCol, normal) },
							{ 3, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(VertexPosTexNorCol, color) },
						};

						if (ppAttributeDescriptions_out != nullptr)
						{
							*ppAttributeDescriptions_out = attributeDescriptions;
						}

						if (pAttributeDescriptionsCount_out != nullptr)
						{
							*pAttributeDescriptionsCount_out = _countof(attributeDescriptions);
						}
					}
					break;
					case EmVertexFormat::ePosTexNorTanBin:
					{
						static const VkVertexInputAttributeDescription attributeDescriptions[] =
						{
							{ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexPosTexNorTanBin, pos) },
							{ 1, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(VertexPosTexNorTanBin, uv) },
							{ 2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexPosTexNorTanBin, normal) },
							{ 3, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexPosTexNorTanBin, tangent) },
							{ 4, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexPosTexNorTanBin, binormal) },
						};

						if (ppAttributeDescriptions_out != nullptr)
						{
							*ppAttributeDescriptions_out = attributeDescriptions;
						}

						if (pAttributeDescriptionsCount_out != nullptr)
						{
							*pAttributeDescriptionsCount_out = _countof(attributeDescriptions);
						}
					}
					break;
					case EmVertexFormat::ePosTexNorWeiIdx:
					{
						static const VkVertexInputAttributeDescription attributeDescriptions[] =
						{
							{ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexPosTexNorWeiIdx, pos) },
							{ 1, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(VertexPosTexNorWeiIdx, uv) },
							{ 2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexPosTexNorWeiIdx, normal) },
							{ 3, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexPosTexNorWeiIdx, boneWeight) },
							{ 4, 0, VK_FORMAT_R16G16B16A16_SFLOAT, offsetof(VertexPosTexNorWeiIdx, boneIndices) },
						};

						if (ppAttributeDescriptions_out != nullptr)
						{
							*ppAttributeDescriptions_out = attributeDescriptions;
						}

						if (pAttributeDescriptionsCount_out != nullptr)
						{
							*pAttributeDescriptionsCount_out = _countof(attributeDescriptions);
						}
					}
					break;
					default:
						if (ppAttributeDescriptions_out != nullptr)
						{
							*ppAttributeDescriptions_out = nullptr;
						}

						if (pAttributeDescriptionsCount_out != nullptr)
						{
							*pAttributeDescriptionsCount_out = 0;
						}
						break;
					}
				}

				VkSamplerCreateInfo GetSamplerCreateInfo(SamplerState::Type samplerState)
				{
					VkSamplerCreateInfo samplerCreateInfo{};
					samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;

					switch (samplerState)
					{
					case SamplerState::eMinMagMipLinearWrap:
					case SamplerState::eMinMagMipLinearClamp:
					case SamplerState::eMinMagMipLinearBorder:
					case SamplerState::eMinMagMipLinearMirror:
					case SamplerState::eMinMagMipLinearMirrorOnce:
						samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
						samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
						samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
						samplerCreateInfo.mipLodBias = 0.f;
						samplerCreateInfo.maxAnisotropy = 1;
						samplerCreateInfo.anisotropyEnable = VK_FALSE;
						samplerCreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;
						samplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
						samplerCreateInfo.minLod = 0.f;
						samplerCreateInfo.maxLod = std::numeric_limits<float>::max();
						switch (samplerState)
						{
						case SamplerState::eMinMagMipLinearWrap:
							samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
							samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
							samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
							break;
						case SamplerState::eMinMagMipLinearClamp:
							samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
							samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
							samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
							break;
						case SamplerState::eMinMagMipLinearBorder:
							samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
							samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
							samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
							break;
						case SamplerState::eMinMagMipLinearMirror:
							samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
							samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
							samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
							break;
						case SamplerState::eMinMagMipLinearMirrorOnce:
							// vkCreateSampler(): A VkSamplerAddressMode value is set to VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE 
							// but the VK_KHR_sampler_mirror_clamp_to_edge extension has not been enabled. 
							// The spec valid usage text states 'If the VK_KHR_sampler_mirror_clamp_to_edge extension is not enabled, 
							// addressModeU, addressModeV and addressModeW must not be VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE'
							// (https://www.khronos.org/registry/vulkan/specs/1.0/html/vkspec.html#VUID-VkSamplerCreateInfo-addressModeU-01079)

							samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
							samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
							samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;

							//samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
							//samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
							//samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
							break;
						}
						break;
					case SamplerState::eMinMagLinearMipPointWrap:
					case SamplerState::eMinMagLinearMipPointClamp:
					case SamplerState::eMinMagLinearMipPointBorder:
					case SamplerState::eMinMagLinearMipPointMirror:
					case SamplerState::eMinMagLinearMipPointMirrorOnce:
						samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
						samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
						samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
						samplerCreateInfo.mipLodBias = 0.f;
						samplerCreateInfo.maxAnisotropy = 1;
						samplerCreateInfo.anisotropyEnable = VK_FALSE;
						samplerCreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;
						samplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
						samplerCreateInfo.minLod = 0.f;
						samplerCreateInfo.maxLod = std::numeric_limits<float>::max();
						switch (samplerState)
						{
						case SamplerState::eMinMagLinearMipPointWrap:
							samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
							samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
							samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
							break;
						case SamplerState::eMinMagLinearMipPointClamp:
							samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
							samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
							samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
							break;
						case SamplerState::eMinMagLinearMipPointBorder:
							samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
							samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
							samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
							break;
						case SamplerState::eMinMagLinearMipPointMirror:
							samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
							samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
							samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
							break;
						case SamplerState::eMinMagLinearMipPointMirrorOnce:
							// vkCreateSampler(): A VkSamplerAddressMode value is set to VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE 
							// but the VK_KHR_sampler_mirror_clamp_to_edge extension has not been enabled. 
							// The spec valid usage text states 'If the VK_KHR_sampler_mirror_clamp_to_edge extension is not enabled, 
							// addressModeU, addressModeV and addressModeW must not be VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE'
							// (https://www.khronos.org/registry/vulkan/specs/1.0/html/vkspec.html#VUID-VkSamplerCreateInfo-addressModeU-01079)

							samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
							samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
							samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;

							//samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
							//samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
							//samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
							break;
						}
						break;
					case SamplerState::eAnisotropicWrap:
					case SamplerState::eAnisotropicClamp:
					case SamplerState::eAnisotropicBorder:
					case SamplerState::eAnisotropicMirror:
					case SamplerState::eAnisotropicMirrorOnce:
						samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
						samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
						samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
						samplerCreateInfo.mipLodBias = 0.f;
						samplerCreateInfo.maxAnisotropy = 16;
						samplerCreateInfo.anisotropyEnable = VK_TRUE;
						samplerCreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;
						samplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
						samplerCreateInfo.minLod = 0.f;
						samplerCreateInfo.maxLod = std::numeric_limits<float>::max();
						switch (samplerState)
						{
						case SamplerState::eAnisotropicWrap:
							samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
							samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
							samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
							break;
						case SamplerState::eAnisotropicClamp:
							samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
							samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
							samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
							break;
						case SamplerState::eAnisotropicBorder:
							samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
							samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
							samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
							break;
						case SamplerState::eAnisotropicMirror:
							samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
							samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
							samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
							break;
						case SamplerState::eAnisotropicMirrorOnce:
							// vkCreateSampler(): A VkSamplerAddressMode value is set to VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE 
							// but the VK_KHR_sampler_mirror_clamp_to_edge extension has not been enabled. 
							// The spec valid usage text states 'If the VK_KHR_sampler_mirror_clamp_to_edge extension is not enabled, 
							// addressModeU, addressModeV and addressModeW must not be VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE'
							// (https://www.khronos.org/registry/vulkan/specs/1.0/html/vkspec.html#VUID-VkSamplerCreateInfo-addressModeU-01079)

							samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
							samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
							samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;

							//samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
							//samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
							//samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
							break;
						}
						break;
					case SamplerState::eMinMagMipPointWrap:
					case SamplerState::eMinMagMipPointClamp:
					case SamplerState::eMinMagMipPointBorder:
					case SamplerState::eMinMagMipPointMirror:
					case SamplerState::eMinMagMipPointMirrorOnce:
						samplerCreateInfo.minFilter = VK_FILTER_NEAREST;
						samplerCreateInfo.magFilter = VK_FILTER_NEAREST;
						samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
						samplerCreateInfo.mipLodBias = 0.f;
						samplerCreateInfo.maxAnisotropy = 1;
						samplerCreateInfo.anisotropyEnable = VK_FALSE;
						samplerCreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;
						samplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
						samplerCreateInfo.minLod = 0.f;
						samplerCreateInfo.maxLod = std::numeric_limits<float>::max();
						switch (samplerState)
						{
						case SamplerState::eMinMagMipPointWrap:
							samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
							samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
							samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
							break;
						case SamplerState::eMinMagMipPointClamp:
							samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
							samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
							samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
							break;
						case SamplerState::eMinMagMipPointBorder:
							samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
							samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
							samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
							break;
						case SamplerState::eMinMagMipPointMirror:
							samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
							samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
							samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
							break;
						case SamplerState::eMinMagMipPointMirrorOnce:
							// vkCreateSampler(): A VkSamplerAddressMode value is set to VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE 
							// but the VK_KHR_sampler_mirror_clamp_to_edge extension has not been enabled. 
							// The spec valid usage text states 'If the VK_KHR_sampler_mirror_clamp_to_edge extension is not enabled, 
							// addressModeU, addressModeV and addressModeW must not be VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE'
							// (https://www.khronos.org/registry/vulkan/specs/1.0/html/vkspec.html#VUID-VkSamplerCreateInfo-addressModeU-01079)

							samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
							samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
							samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;

							//samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
							//samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
							//samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
							break;
						}
						break;
					default:
						assert(false);
						break;
					}

					return samplerCreateInfo;
				}

				VkPipelineRasterizationStateCreateInfo GetRasterizerCreateInfo(RasterizerState::Type rasterizerState)
				{
					// vulkan, dx 호환을 위해, ccw <-> cw 반대로 셋팅
					switch (rasterizerState)
					{
					case RasterizerState::eSolidCCW:
					{
						return
						{
							VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,	// sType
							nullptr,	// pNext
							0,	// flags
							VK_FALSE,	// depthClampEnable
							VK_FALSE,	// rasterizerDiscardEnable
							VK_POLYGON_MODE_FILL,	// polygonMode
							VK_CULL_MODE_BACK_BIT,	// cullMode
							VK_FRONT_FACE_CLOCKWISE,	// frontFace,
							VK_FALSE,	// depthBiasEnable
							0.f,	// depthBiasConstantFactor
							0.f,	// depthBiasClamp
							0.f,	// depthBiasSlopeFactor
							1.f	// lineWidth
						};
					}
					break;
					case RasterizerState::eSolidCW:
					{
						return
						{
							VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,	// sType
							nullptr,	// pNext
							0,	// flags
							VK_FALSE,	// depthClampEnable
							VK_FALSE,	// rasterizerDiscardEnable
							VK_POLYGON_MODE_FILL,	// polygonMode
							VK_CULL_MODE_FRONT_BIT,	// cullMode
							VK_FRONT_FACE_COUNTER_CLOCKWISE,	// frontFace,
							VK_FALSE,	// depthBiasEnable
							0.f,	// depthBiasConstantFactor
							0.f,	// depthBiasClamp
							0.f,	// depthBiasSlopeFactor
							1.f	// lineWidth
						};
					}
					break;
					case RasterizerState::eSolidCullNone:
					{
						return
						{
							VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,	// sType
							nullptr,	// pNext
							0,	// flags
							VK_FALSE,	// depthClampEnable
							VK_FALSE,	// rasterizerDiscardEnable
							VK_POLYGON_MODE_FILL,	// polygonMode
							VK_CULL_MODE_NONE,	// cullMode
							VK_FRONT_FACE_CLOCKWISE,	// frontFace,
							VK_FALSE,	// depthBiasEnable
							0.f,	// depthBiasConstantFactor
							0.f,	// depthBiasClamp
							0.f,	// depthBiasSlopeFactor
							1.f	// lineWidth
						};
					}
					break;
					case RasterizerState::eWireframeCCW:
					{
						return
						{
							VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,	// sType
							nullptr,	// pNext
							0,	// flags
							VK_FALSE,	// depthClampEnable
							VK_FALSE,	// rasterizerDiscardEnable
							VK_POLYGON_MODE_LINE,	// polygonMode
							VK_CULL_MODE_BACK_BIT,	// cullMode
							VK_FRONT_FACE_CLOCKWISE,	// frontFace,
							VK_FALSE,	// depthBiasEnable
							0.f,	// depthBiasConstantFactor
							0.f,	// depthBiasClamp
							0.f,	// depthBiasSlopeFactor
							1.f	// lineWidth
						};
					}
					break;
					case RasterizerState::eWireframeCW:
					{
						return
						{
							VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,	// sType
							nullptr,	// pNext
							0,	// flags
							VK_FALSE,	// depthClampEnable
							VK_FALSE,	// rasterizerDiscardEnable
							VK_POLYGON_MODE_LINE,	// polygonMode
							VK_CULL_MODE_FRONT_BIT,	// cullMode
							VK_FRONT_FACE_COUNTER_CLOCKWISE,	// frontFace,
							VK_FALSE,	// depthBiasEnable
							0.f,	// depthBiasConstantFactor
							0.f,	// depthBiasClamp
							0.f,	// depthBiasSlopeFactor
							1.f	// lineWidth
						};
					}
					break;
					case RasterizerState::eWireframeCullNone:
					{
						return
						{
							VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,	// sType
							nullptr,	// pNext
							0,	// flags
							VK_FALSE,	// depthClampEnable
							VK_FALSE,	// rasterizerDiscardEnable
							VK_POLYGON_MODE_LINE,	// polygonMode
							VK_CULL_MODE_NONE,	// cullMode
							VK_FRONT_FACE_CLOCKWISE,	// frontFace,
							VK_FALSE,	// depthBiasEnable
							0.f,	// depthBiasConstantFactor
							0.f,	// depthBiasClamp
							0.f,	// depthBiasSlopeFactor
							1.f	// lineWidth
						};
					}
					break;
					default:
					{
						assert(false);
						// return RasterizerState::eSolidCCW;
						return
						{
							VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,	// sType
							nullptr,	// pNext
							0,	// flags
							VK_FALSE,	// depthClampEnable
							VK_FALSE,	// rasterizerDiscardEnable
							VK_POLYGON_MODE_FILL,	// polygonMode
							VK_CULL_MODE_BACK_BIT,	// cullMode
							VK_FRONT_FACE_CLOCKWISE,	// frontFace,
							VK_FALSE,	// depthBiasEnable
							0.f,	// depthBiasConstantFactor
							0.f,	// depthBiasClamp
							0.f,	// depthBiasSlopeFactor
							1.f	// lineWidth
						};
					}
					break;
					}
				}

				VkPipelineColorBlendAttachmentState GetBlendAttachmentState(BlendState::Type blendState)
				{
					switch (blendState)
					{
					case BlendState::eOff:
					{
						return
						{
							false,	// blendEnable
							VK_BLEND_FACTOR_SRC_ALPHA,	// srcColorBlendFactor
							VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,	// dstColorBlendFactor
							VK_BLEND_OP_ADD,	// colorBlendOp
							VK_BLEND_FACTOR_ONE,	// srcAlphaBlendFactor
							VK_BLEND_FACTOR_ZERO,	// dstAlphaBlendFactor
							VK_BLEND_OP_ADD,	// alphaBlendOp
							VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,	// colorWriteMask
						};
					}
					break;
					case BlendState::eLinear:
					{
						return
						{
							true,	// blendEnable
							VK_BLEND_FACTOR_SRC_ALPHA,	// srcColorBlendFactor
							VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,	// dstColorBlendFactor
							VK_BLEND_OP_ADD,	// colorBlendOp
							VK_BLEND_FACTOR_ONE,	// srcAlphaBlendFactor
							VK_BLEND_FACTOR_ZERO,	// dstAlphaBlendFactor
							VK_BLEND_OP_ADD,	// alphaBlendOp
							VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,	// colorWriteMask
						};
					}
					break;
					case BlendState::eAdditive:
					{
						return
						{
							true,	// blendEnable
							VK_BLEND_FACTOR_SRC_ALPHA,	// srcColorBlendFactor
							VK_BLEND_FACTOR_ONE,	// dstColorBlendFactor
							VK_BLEND_OP_ADD,	// colorBlendOp
							VK_BLEND_FACTOR_SRC_ALPHA,	// srcAlphaBlendFactor
							VK_BLEND_FACTOR_ONE,	// dstAlphaBlendFactor
							VK_BLEND_OP_ADD,	// alphaBlendOp
							VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,	// colorWriteMask
						};
					}
					break;
					case BlendState::eSubTractive:
					{
						return
						{
							true,	// blendEnable
							VK_BLEND_FACTOR_SRC_ALPHA,	// srcColorBlendFactor
							VK_BLEND_FACTOR_ONE,	// dstColorBlendFactor
							VK_BLEND_OP_SUBTRACT,	// colorBlendOp
							VK_BLEND_FACTOR_ONE,	// srcAlphaBlendFactor
							VK_BLEND_FACTOR_ONE,	// dstAlphaBlendFactor
							VK_BLEND_OP_ADD,	// alphaBlendOp
							VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,	// colorWriteMask
						};
					}
					break;
					case BlendState::eMultiplicative:
					{
						return
						{
							true,	// blendEnable
							VK_BLEND_FACTOR_ZERO,	// srcColorBlendFactor
							VK_BLEND_FACTOR_SRC_COLOR,	// dstColorBlendFactor
							VK_BLEND_OP_ADD,	// colorBlendOp
							VK_BLEND_FACTOR_ONE,	// srcAlphaBlendFactor
							VK_BLEND_FACTOR_ONE,	// dstAlphaBlendFactor
							VK_BLEND_OP_ADD,	// alphaBlendOp
							VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,	// colorWriteMask
						};
					}
					break;
					case BlendState::eSquared:
					{
						return
						{
							true,	// blendEnable
							VK_BLEND_FACTOR_ZERO,	// srcColorBlendFactor
							VK_BLEND_FACTOR_DST_COLOR,	// dstColorBlendFactor
							VK_BLEND_OP_ADD,	// colorBlendOp
							VK_BLEND_FACTOR_ONE,	// srcAlphaBlendFactor
							VK_BLEND_FACTOR_ONE,	// dstAlphaBlendFactor
							VK_BLEND_OP_ADD,	// alphaBlendOp
							VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,	// colorWriteMask
						};
					}
					break;
					case BlendState::eNegative:
					{
						return
						{
							true,	// blendEnable
							VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR,	// srcColorBlendFactor
							VK_BLEND_FACTOR_ZERO,	// dstColorBlendFactor
							VK_BLEND_OP_ADD,	// colorBlendOp
							VK_BLEND_FACTOR_ONE,	// srcAlphaBlendFactor
							VK_BLEND_FACTOR_ONE,	// dstAlphaBlendFactor
							VK_BLEND_OP_ADD,	// alphaBlendOp
							VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,	// colorWriteMask
						};
					}
					break;
					case BlendState::eOpacity:
					{
						return
						{
							true,	// blendEnable
							VK_BLEND_FACTOR_ONE,	// srcColorBlendFactor
							VK_BLEND_FACTOR_ZERO,	// dstColorBlendFactor
							VK_BLEND_OP_ADD,	// colorBlendOp
							VK_BLEND_FACTOR_ONE,	// srcAlphaBlendFactor
							VK_BLEND_FACTOR_ZERO,	// dstAlphaBlendFactor
							VK_BLEND_OP_ADD,	// alphaBlendOp
							VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,	// colorWriteMask
						};
					}
					break;
					case BlendState::eAlphaBlend:
					{
						return
						{
							true,	// blendEnable
							VK_BLEND_FACTOR_ONE,	// srcColorBlendFactor
							VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,	// dstColorBlendFactor
							VK_BLEND_OP_ADD,	// colorBlendOp
							VK_BLEND_FACTOR_ONE,	// srcAlphaBlendFactor
							VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,	// dstAlphaBlendFactor
							VK_BLEND_OP_ADD,	// alphaBlendOp
							VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,	// colorWriteMask
						};
					}
					break;
					default:
					{
						assert(false);
						// return BlendState::eOff
						return
						{
							false,	// blendEnable
							VK_BLEND_FACTOR_SRC_ALPHA,	// srcColorBlendFactor
							VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,	// dstColorBlendFactor
							VK_BLEND_OP_ADD,	// colorBlendOp
							VK_BLEND_FACTOR_ONE,	// srcAlphaBlendFactor
							VK_BLEND_FACTOR_ZERO,	// dstAlphaBlendFactor
							VK_BLEND_OP_ADD,	// alphaBlendOp
							VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,	// colorWriteMask
						};
					}
					break;
					}
				}

				VkPipelineColorBlendStateCreateInfo GetBlendCreateInfo(const VkPipelineColorBlendAttachmentState* pAttachmentState, uint32_t nAttachmentStateCount)
				{
					return
					{
						VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,	// sType
						nullptr,	// pNext
						0,	// flags
						VK_FALSE,	// logicOpEnable
						VK_LOGIC_OP_COPY,	// logicOp
						nAttachmentStateCount,	// attachmentCount
						pAttachmentState,	// pAttachments
						{ 0.f, 0.f, 0.f, 0.f }
					};
				}

				VkPipelineDepthStencilStateCreateInfo GetDepthStencilCreateInfo(DepthStencilState::Type depthStencilState)
				{
					switch (depthStencilState)
					{
					case DepthStencilState::eRead_Write_On:
					{
						return
						{
							VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,	// sType
							nullptr,	// pNext
							0,	// flags
							VK_TRUE,	// depthTestEnable
							VK_TRUE,	// depthWriteEnable
							VK_COMPARE_OP_LESS,	// depthCompareOp
							VK_FALSE,	// depthBoundsTestEnable
							VK_FALSE,	// stencilTestEnable
							{
								VK_STENCIL_OP_KEEP,	// failOp;
								VK_STENCIL_OP_KEEP,	// passOp;
								VK_STENCIL_OP_INCREMENT_AND_CLAMP, // depthFailOp;
								VK_COMPARE_OP_ALWAYS,	// compareOp;
								0,	// compareMask;
								0,	// writeMask;
								0,	// reference;
							},	// front
							{
								VK_STENCIL_OP_KEEP,	// failOp;
								VK_STENCIL_OP_KEEP,	// passOp;
								VK_STENCIL_OP_DECREMENT_AND_CLAMP, // depthFailOp;
								VK_COMPARE_OP_ALWAYS,	// compareOp;
								0,	// compareMask;
								0,	// writeMask;
								0,	// reference;
							},	// back
							0.f,	// minDepthBounds
							0.f,	// maxDepthBounds
						};
					}
					break;
					case DepthStencilState::eRead_Write_Off:
					{
						return
						{
							VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,	// sType
							nullptr,	// pNext
							0,	// flags
							VK_FALSE,	// depthTestEnable
							VK_FALSE,	// depthWriteEnable
							VK_COMPARE_OP_LESS,	// depthCompareOp
							VK_FALSE,	// depthBoundsTestEnable
							VK_FALSE,	// stencilTestEnable
							{
								VK_STENCIL_OP_KEEP,	// failOp;
								VK_STENCIL_OP_KEEP,	// passOp;
								VK_STENCIL_OP_KEEP, // depthFailOp;
								VK_COMPARE_OP_ALWAYS,	// compareOp;
								0,	// compareMask;
								0,	// writeMask;
								0,	// reference;
							},	// front
							{
								VK_STENCIL_OP_KEEP,	// failOp;
								VK_STENCIL_OP_KEEP,	// passOp;
								VK_STENCIL_OP_KEEP, // depthFailOp;
								VK_COMPARE_OP_ALWAYS,	// compareOp;
								0,	// compareMask;
								0,	// writeMask;
								0,	// reference;
							},	// back
							0.f,	// minDepthBounds
							0.f,	// maxDepthBounds
						};
					}
					break;
					case DepthStencilState::eRead_On_Write_Off:
					{
						return
						{
							VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,	// sType
							nullptr,	// pNext
							0,	// flags
							VK_TRUE,	// depthTestEnable
							VK_FALSE,	// depthWriteEnable
							VK_COMPARE_OP_LESS,	// depthCompareOp
							VK_FALSE,	// depthBoundsTestEnable
							VK_FALSE,	// stencilTestEnable
							{
								VK_STENCIL_OP_KEEP,	// failOp;
								VK_STENCIL_OP_KEEP,	// passOp;
								VK_STENCIL_OP_INCREMENT_AND_CLAMP, // depthFailOp;
								VK_COMPARE_OP_ALWAYS,	// compareOp;
								0,	// compareMask;
								0,	// writeMask;
								0,	// reference;
							},	// front
							{
								VK_STENCIL_OP_KEEP,	// failOp;
								VK_STENCIL_OP_KEEP,	// passOp;
								VK_STENCIL_OP_DECREMENT_AND_CLAMP, // depthFailOp;
								VK_COMPARE_OP_ALWAYS,	// compareOp;
								0,	// compareMask;
								0,	// writeMask;
								0,	// reference;
							},	// back
							0.f,	// minDepthBounds
							0.f,	// maxDepthBounds
						};
					}
					break;
					case DepthStencilState::eRead_Off_Write_On:
					{
						return
						{
							VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,	// sType
							nullptr,	// pNext
							0,	// flags
							VK_FALSE,	// depthTestEnable
							VK_TRUE,	// depthWriteEnable
							VK_COMPARE_OP_LESS,	// depthCompareOp
							VK_FALSE,	// depthBoundsTestEnable
							VK_FALSE,	// stencilTestEnable
							{
								VK_STENCIL_OP_KEEP,	// failOp;
								VK_STENCIL_OP_KEEP,	// passOp;
								VK_STENCIL_OP_KEEP, // depthFailOp;
								VK_COMPARE_OP_ALWAYS,	// compareOp;
								0,	// compareMask;
								0,	// writeMask;
								0,	// reference;
							},	// front
							{
								VK_STENCIL_OP_KEEP,	// failOp;
								VK_STENCIL_OP_KEEP,	// passOp;
								VK_STENCIL_OP_KEEP, // depthFailOp;
								VK_COMPARE_OP_ALWAYS,	// compareOp;
								0,	// compareMask;
								0,	// writeMask;
								0,	// reference;
							},	// back
							0.f,	// minDepthBounds
							0.f,	// maxDepthBounds
						};
					}
					break;
					break;
					default:
					{
						assert(false);

						// return DepthStencilState::eRead_Write_On
						return
						{
							VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,	// sType
							nullptr,	// pNext
							0,	// flags
							VK_TRUE,	// depthTestEnable
							VK_TRUE,	// depthWriteEnable
							VK_COMPARE_OP_LESS,	// depthCompareOp
							VK_FALSE,	// depthBoundsTestEnable
							VK_FALSE,	// stencilTestEnable
						{
							VK_STENCIL_OP_KEEP,	// failOp;
							VK_STENCIL_OP_KEEP,	// passOp;
							VK_STENCIL_OP_INCREMENT_AND_CLAMP, // depthFailOp;
							VK_COMPARE_OP_ALWAYS,	// compareOp;
							0,	// compareMask;
							0,	// writeMask;
							0,	// reference;
						},	// front
							{
								VK_STENCIL_OP_KEEP,	// failOp;
								VK_STENCIL_OP_KEEP,	// passOp;
								VK_STENCIL_OP_DECREMENT_AND_CLAMP, // depthFailOp;
								VK_COMPARE_OP_ALWAYS,	// compareOp;
								0,	// compareMask;
								0,	// writeMask;
								0,	// reference;
							},	// back
							0.f,	// minDepthBounds
							0.f,	// maxDepthBounds
						};
					}
					break;
					}
				}

				const VkViewport* Convert(const math::Viewport& viewport)
				{
					return reinterpret_cast<const VkViewport*>(&viewport);
				}
			}
		}
	}
}