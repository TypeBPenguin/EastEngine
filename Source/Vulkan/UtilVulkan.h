#pragma once

#include "GraphicsInterface/Define.h"
#include "GraphicsInterface/Vertex.h"

namespace eastengine
{
	namespace graphics
	{
		namespace vulkan
		{
			struct tKey {};
			using PSOKey = PhantomType<tKey, const String::StringKey>;

			enum CompileShaderType
			{
				eVertexShader = 0,
				eFragmentShader,
				eComputeShader,
				eGeometryShader,
				eTessControl,
				eTessEvaluation,

				CompileShaderTypeCount,
			};

			struct ShaderMacro
			{
				const char* Name{ nullptr };
				const char* Definition{ nullptr };
			};

			template <typename T>
			struct UniformBuffer
			{
				VkBuffer uniformBuffer{ nullptr };
				VkDeviceMemory uniformBufferMemory{ nullptr };
				VkDescriptorSet descriptorSet{ nullptr };

				uint8_t* pUniformBufferData{ nullptr };

				static constexpr size_t Size() noexcept { return (sizeof(T) + 255) & ~255; }
				T* Cast(size_t nIndex) { return reinterpret_cast<T*>(pUniformBufferData + (nIndex * Size())); }
			};

			namespace util
			{
				std::string LoadShaderCode(const char* strFilePath);
				std::vector<uint32_t> CompileShader(CompileShaderType emCompileShaderType, const char* sourceText, size_t sourceTextSize, const ShaderMacro* pShaderMacros, size_t nMacroCount, const char* strFilePath);
				VkShaderModule CreateShaderModule(VkDevice device, const uint32_t* pShaderCode, size_t nCodeSize);

				VkDescriptorSetLayout CreateDescriptorSetLayout(VkDevice device, const VkDescriptorSetLayoutBinding* pDescriptorSetLayoutBinding, size_t nDescriptorSetLayoutBindingCount);

				void GetVertexBindingDescription(EmVertexFormat::Type emVertexFormat, VkVertexInputBindingDescription* pInputBindingDescription_out);
				void GetVertexAttributeDescriptions(EmVertexFormat::Type emVertexFormat, const VkVertexInputAttributeDescription** ppAttributeDescriptions_out, uint32_t* pAttributeDescriptionsCount_out);

				VkSamplerCreateInfo GetSamplerCreateInfo(EmSamplerState::Type emSamplerState);
				VkPipelineRasterizationStateCreateInfo GetRasterizerCreateInfo(EmRasterizerState::Type emRasterizerState);
				VkPipelineColorBlendAttachmentState GetBlendAttachmentState(EmBlendState::Type emBlendState);
				VkPipelineColorBlendStateCreateInfo GetBlendCreateInfo(const VkPipelineColorBlendAttachmentState* pAttachmentState, uint32_t nAttachmentStateCount);
				VkPipelineDepthStencilStateCreateInfo GetDepthStencilCreateInfo(EmDepthStencilState::Type emDepthStencilState);
			}
		}
	}
}

namespace std
{
	template <>
	struct hash<eastengine::graphics::vulkan::PSOKey>
	{
		std::uint64_t operator()(const eastengine::graphics::vulkan::PSOKey& key) const
		{
			return key.value.value;
		}
	};
}