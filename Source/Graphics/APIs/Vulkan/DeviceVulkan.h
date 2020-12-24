#pragma once

#include "CommonLib/Singleton.h"
#include "Graphics/Interface/GraphicsInterface.h"

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

namespace est
{
	namespace graphics
	{
		class IImageBasedLight;

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
				void Initialize(uint32_t width, uint32_t height, bool isFullScreen, const string::StringID& applicationTitle, const string::StringID& applicationName, std::function<HRESULT(HWND, uint32_t, WPARAM, LPARAM)> messageHandler);
				void Run(std::function<bool()> funcUpdate);
				void Cleanup(float elapsedTime);

			public:
				void ScreenShot(ScreenShotFormat format, const std::wstring& path, std::function<void(bool, const std::wstring&)> screenShotCallback);

			public:
				const math::uint2& GetScreenSize() const;
				const math::Viewport& GetViewport() const;

				bool IsFullScreen() const;
				void SetFullScreen(bool isFullScreen, std::function<void(bool)> callback);

				const std::vector<DisplayModeDesc>& GetSupportedDisplayModeDesc() const;
				size_t GetSelectedDisplayModeIndex() const;
				void ChangeDisplayMode(size_t displayModeIndex, std::function<void(bool)> callback);

				VkDevice GetInterface() const;
				VkCommandPool GetCommandPool() const;

				uint32_t GetFrameIndex() const;
				uint32_t GetFrameCount() const;
				VkCommandBuffer GetCommandBuffer(size_t index) const;

				VkFormat GetSwapChainImageFormat() const;
				VkExtent2D GetSwapChainExtent2D() const;

				VkSemaphore GetImageAvailableSemaphore() const;
				VkSemaphore GetRenderFinishedSemaphore() const;

				VkSampler GetSampler(SamplerState::Type emType) const;

			public:
				GBuffer* GetGBuffer(int frameIndex) const;
				IImageBasedLight* GetImageBasedLight() const;
				void SetImageBasedLight(IImageBasedLight* pImageBasedLight);
				RenderManager* GetRenderManager() const;

				const Texture* GetEmptyTexture() const;

			public:
				std::vector<char> ReadFile(const std::string& filename);

			public:
				void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
				void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

				void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer* buffer, VkDeviceMemory* bufferMemory, void** ppData_out);

				void CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage* image_out, VkDeviceMemory* imageMemory_out);
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