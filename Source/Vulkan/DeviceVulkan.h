#pragma once

#include "CommonLib/Singleton.h"

#include "GraphicsInterface/Define.h"

struct VkDevice_T;
struct VkCommandPool_T;
struct VkRenderPass_T;
struct VkFramebuffer_T;
struct VkCommandBuffer_T;
struct VkBuffer_T;
struct VkImage_T;
struct VkDeviceMemory_T;
struct VkSemaphore_T;
struct VkImageView_T;
struct VkSampler_T;

typedef VkDevice_T* VkDevice;
typedef VkCommandPool_T* VkCommandPool;
typedef VkCommandBuffer_T* VkCommandBuffer;
typedef VkBuffer_T* VkBuffer;
typedef VkImage_T* VkImage;
typedef VkImageView_T* VkImageView;
typedef VkDeviceMemory_T* VkDeviceMemory;
typedef VkSemaphore_T* VkSemaphore;
typedef VkSampler_T* VkSampler;

struct VkExtent2D;
struct VkViewport;
enum VkFormat;
enum VkImageTiling;
enum VkImageLayout;

typedef uint32_t VkFlags;
typedef uint64_t VkDeviceSize;
typedef VkFlags VkBufferUsageFlags;
typedef VkFlags VkMemoryPropertyFlags;
typedef VkFlags VkImageUsageFlags;
typedef VkFlags VkImageAspectFlags;

namespace eastengine
{
	namespace graphics
	{
		class ImageBasedLight;

		namespace vulkan
		{
			class GBuffer;
			class RenderManager;
			class Texture;

			class Device : public Singleton<Device>
			{
				friend Singleton<Device>;
			private:
				Device();
				virtual ~Device();

			public:
				void Initialize(uint32_t nWidth, uint32_t nHeight, bool isFullScreen, const String::StringID& strApplicationTitle, const String::StringID& strApplicationName);

				void Run(std::function<void()> funcUpdate);

				void Flush(float fElapsedTime);

			public:
				const math::UInt2& GetScreenSize() const;

				VkDevice GetInterface() const;
				VkCommandPool GetCommandPool() const;

				uint32_t GetFrameIndex() const;
				uint32_t GetFrameCount() const;
				VkCommandBuffer GetCommandBuffer(size_t nIndex) const;

				VkFormat GetSwapChainImageFormat() const;
				VkExtent2D GetSwapChainExtent2D() const;

				const VkViewport* GetViewport() const;

				VkSemaphore GetImageAvailableSemaphore() const;
				VkSemaphore GetRenderFinishedSemaphore() const;

				VkSampler GetSampler(EmSamplerState::Type emType) const;

			public:
				GBuffer* GetGBuffer(int nFrameIndex) const;
				ImageBasedLight* GetImageBasedLight() const;
				RenderManager* GetRenderManager() const;

				const Texture* GetEmptyTexture() const;

			public:
				std::vector<char> ReadFile(const std::string& filename);

			public:
				void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
				void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t nWidth, uint32_t nHeight);

				void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer* buffer, VkDeviceMemory* bufferMemory, void** ppData_out);

				void CreateImage(uint32_t nWidth, uint32_t nHeight, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage* image_out, VkDeviceMemory* imageMemory_out);
				VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
				void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

				VkFormat FindDepthFormat() const;

			private:
				class Impl;
				std::unique_ptr<Impl> m_pImpl;
			};
		}
	}
}