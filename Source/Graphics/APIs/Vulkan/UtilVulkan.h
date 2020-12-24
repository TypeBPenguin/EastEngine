#pragma once

#include "Graphics/Interface/Define.h"
#include "Graphics/Interface/Vertex.h"

namespace est
{
	namespace graphics
	{
		namespace vulkan
		{
			struct tKey { static constexpr const wchar_t* DefaultValue() { return L""; } };
			using PSOKey = PhantomType<tKey, string::StringID>;

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
				T* Cast(size_t index) { return reinterpret_cast<T*>(pUniformBufferData + (index * Size())); }
			};

			namespace util
			{
				std::string LoadShaderCode(const wchar_t* strFilePath);
				std::vector<uint32_t> CompileShader(CompileShaderType emCompileShaderType, const char* sourceText, size_t sourceTextSize, const ShaderMacro* pShaderMacros, size_t macroCount, const wchar_t* filePath);
				VkShaderModule CreateShaderModule(VkDevice device, const uint32_t* pShaderCode, size_t nCodeSize);

				VkDescriptorSetLayout CreateDescriptorSetLayout(VkDevice device, const VkDescriptorSetLayoutBinding* pDescriptorSetLayoutBinding, size_t nDescriptorSetLayoutBindingCount);

				void GetVertexBindingDescription(EmVertexFormat::Type emVertexFormat, VkVertexInputBindingDescription* pInputBindingDescription_out);
				void GetVertexAttributeDescriptions(EmVertexFormat::Type emVertexFormat, const VkVertexInputAttributeDescription** ppAttributeDescriptions_out, uint32_t* pAttributeDescriptionsCount_out);

				VkSamplerCreateInfo GetSamplerCreateInfo(SamplerState::Type samplerState);
				VkPipelineRasterizationStateCreateInfo GetRasterizerCreateInfo(RasterizerState::Type rasterizerState);
				VkPipelineColorBlendAttachmentState GetBlendAttachmentState(BlendState::Type blendState);
				VkPipelineColorBlendStateCreateInfo GetBlendCreateInfo(const VkPipelineColorBlendAttachmentState* pAttachmentState, uint32_t nAttachmentStateCount);
				VkPipelineDepthStencilStateCreateInfo GetDepthStencilCreateInfo(DepthStencilState::Type depthStencilState);

				const VkViewport* Convert(const math::Viewport& viewport);
			}
		}
	}
}

namespace std
{
	template <>
	struct hash<est::graphics::vulkan::PSOKey>
	{
		const size_t operator()(const est::graphics::vulkan::PSOKey& key) const
		{
			return reinterpret_cast<size_t>(key.Value().GetData());
		}
	};
}