#include "stdafx.h"
#include "DeviceVulkan.h"

#include "UtilVulkan.h"

#include "SwapChainBufferVulkan.h"
#include "GBufferVulkan.h"
#include "RenderManagerVulkan.h"
#include "TextureVulkan.h"

namespace sid
{
	RegisterStringID(Vulkan_Empty_Texture);
}

namespace est
{
	namespace graphics
	{
		namespace vulkan
		{
			const std::vector<const char*> ValidationLayers =
			{
				"VK_LAYER_LUNARG_standard_validation",
			};

			const std::vector<const char*> DeviceExtensions =
			{
				VK_KHR_SWAPCHAIN_EXTENSION_NAME,
			};

#ifdef NDEBUG
			const bool EnableValidationLayers = false;
#else
			const bool EnableValidationLayers = true;
#endif

			static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
				VkDebugReportFlagsEXT flags,
				VkDebugReportObjectTypeEXT objType,
				uint64_t obj,
				size_t location,
				int32_t code,
				const char* strLayerPrefix,
				const char* strMsg,
				void* pUserData)
			{
				LOG_ERROR(L"validation layer : %s\n", strMsg);

				return VK_FALSE;
			}

			struct QueueFamilyIndices
			{
				int nGraphicsFamily{ -1 };
				int nPresentFamily{ -1 };

				bool IsComplete() const
				{
					return nGraphicsFamily >= 0 && nPresentFamily >= 0;
				}
			};

			struct SwapChainSupportDetails
			{
				VkSurfaceCapabilitiesKHR capabilities{};
				std::vector<VkSurfaceFormatKHR> vecFormats;
				std::vector<VkPresentModeKHR> vecPresentModes;
			};

			class Device::Impl
			{
			public:
				Impl();
				~Impl();

			public:
				void Initialize(uint32_t width, uint32_t height, bool isFullScreen, const string::StringID& applicationTitle, const string::StringID& applicationName);
				void Release();

				void Run(std::function<bool()> funcUpdate);
				void Cleanup(float elapsedTime);

				void Render();

			public:
				const math::uint2& GetScreenSize() const { return m_screenSize; }
				const math::Viewport& GetViewport() const { return m_viewport; }

				VkDevice GetInterface() const { return m_device; }
				VkCommandPool GetCommandPool() const { return m_commandPool; }

				uint32_t GetFrameIndex() const { return m_frameIndex; }
				uint32_t GetFrameCount() const { return m_nFrameCount; }
				VkCommandBuffer GetCommandBuffer(size_t index) const { return m_vecCommandBuffers[index]; }

				VkFormat GetSwapChainImageFormat() const { return m_swapChainImageFormat; }
				VkExtent2D GetSwapChainExtent2D() const { return m_swapChainExtent; }

				VkSemaphore GetImageAvailableSemaphore() const { return m_imageAvailableSemaphore; }
				VkSemaphore GetRenderFinishedSemaphore() const { return m_renderFinishedSemaphore; }

				VkSampler GetSampler(SamplerState::Type emType) const { return m_samplers[emType]; }

			public:
				GBuffer* GetGBuffer(int frameIndex) const { return m_pGBuffers[frameIndex].get(); }
				IImageBasedLight* GetImageBasedLight() const { return m_pImageBasedLight; }
				void SetImageBasedLight(IImageBasedLight* pImageBasedLight) { m_pImageBasedLight = pImageBasedLight; }

				RenderManager* GetRenderManager() const { return m_pRenderManager.get(); }

				const Texture* GetEmptyTexture() const { return m_pTextureEmpty.get(); }

			public:
				const std::vector<DisplayModeDesc>& GetSupportedDisplayModeDesc() const { return m_supportedDisplayModes; }
				size_t GetSelectedDisplayModeIndex() const { return m_selectedDisplayModeIndex; }

			private:
				void InitializeWindow(uint32_t width, uint32_t height, bool isFullScreen, const string::StringID& applicationTitle, const string::StringID& applicationName);
				void InitializeVK();

				void CleanupSwapChain();
				void RecreateSwapChain();

			private:
				void CreateInstance();
				void CreateSurface();
				void PickPhysicsDevice();
				void CreateLogicalDevice();

				void CreateSwapChain();
				void CreateCommandPool();
				void CreateCommandBuffers();
				void CreateSemaphores();

				void CreateSampler();

				void SetDebugCallback();
				VkResult CreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback);
				void DestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator);

				VkCommandBuffer BeginSingleTimeCommands();
				void EndSingleTimeCommands(VkCommandBuffer commandBuffer);

			public:
				void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
				void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

				void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer* buffer, VkDeviceMemory* bufferMemory, void** ppData_out);
				void CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage* image_out, VkDeviceMemory* imageMemory_out);
				VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);

				void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
				void ImageBarrier(VkCommandBuffer commandBuffer, VkImage image, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, VkImageLayout oldLayout, VkImageLayout newLayout);

			private:
				bool CheckValidationLayerSupport() const;
				bool CheckDeviceExtensionSupport(VkPhysicalDevice device) const;
				bool IsDeviceSuitable(VkPhysicalDevice device) const;
				QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device) const;
				SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device) const;
				VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& vecAvailableFormats) const;
				VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& vecAvailablePresentModes) const;
				VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) const;
				VkFormat FindSupportedFormat(const std::vector<VkFormat>& vecCandidates, VkImageTiling tiling, VkFormatFeatureFlags features) const;
				uint32_t FindMemoryType(uint32_t nTypeFilter, VkMemoryPropertyFlags properties) const;
				bool IsHasStencilComponent(VkFormat format) const;

			public:
				VkFormat FindDepthFormat() const;

			private:
				static void OnWindowResized(GLFWwindow* pWindow, int width, int height);

			private:
				math::uint2 m_screenSize{ 800, 600 };

				string::StringID m_applicationTitle;
				string::StringID m_applicationName;

				size_t m_selectedDisplayModeIndex{ std::numeric_limits<size_t>::max() };
				std::vector<DisplayModeDesc> m_supportedDisplayModes;

				GLFWwindow* m_pWindow{ nullptr };

			private:
				math::Viewport m_viewport{};

				uint32_t m_frameIndex{ 0 };
				uint32_t m_nFrameCount{ 0 };

				VkInstance m_instance{ nullptr };
				VkDebugReportCallbackEXT m_callback{ nullptr };
				VkSurfaceKHR m_surface{ nullptr };
				VkPhysicalDevice m_physicalDevice{ nullptr };
				VkDevice m_device{ nullptr };
				VkQueue m_graphicsQueue{ nullptr };
				VkQueue m_presentQueue{ nullptr };

				VkSwapchainKHR m_swapChain{ nullptr };
				std::vector<std::unique_ptr<SwapChainBuffer>> m_vecSwapChainImages;
				VkFormat m_swapChainImageFormat{ VK_FORMAT_UNDEFINED };
				VkExtent2D m_swapChainExtent{};

				VkCommandPool m_commandPool{ nullptr };
				std::vector<VkCommandBuffer> m_vecCommandBuffers;

				std::array<VkSampler, SamplerState::TypeCount> m_samplers{ nullptr };

				VkSemaphore m_imageAvailableSemaphore{ nullptr };
				VkSemaphore m_renderFinishedSemaphore{ nullptr };

				std::vector<std::unique_ptr<GBuffer>> m_pGBuffers;
				IImageBasedLight* m_pImageBasedLight{ nullptr };

				std::unique_ptr<RenderManager> m_pRenderManager;

				std::unique_ptr<Texture> m_pTextureEmpty;
			};

			Device::Impl::Impl()
			{
			}

			Device::Impl::~Impl()
			{
				Release();
			}

			void Device::Impl::Initialize(uint32_t width, uint32_t height, bool isFullScreen, const string::StringID& applicationTitle, const string::StringID& applicationName)
			{
				InitializeWindow(width, height, isFullScreen, applicationTitle, applicationName);
				InitializeVK();

				m_pGBuffers.resize(m_nFrameCount);
				for (uint32_t i = 0; i < m_nFrameCount; ++i)
				{
					m_pGBuffers[i] = std::make_unique<GBuffer>(m_swapChainExtent.width, m_swapChainExtent.height);
				}
				m_pRenderManager = std::make_unique<RenderManager>();

				m_pTextureEmpty = std::make_unique<Texture>(Texture::Key(sid::Vulkan_Empty_Texture));
				const math::RGBA rgba(0.f, 0.f, 0.f, 1.f);
				m_pTextureEmpty->Bind(1, 1, &rgba.r);
			}

			void Device::Impl::Release()
			{
				m_pRenderManager.reset();

				m_pGBuffers.clear();
				m_pTextureEmpty.reset();

				for (auto& samperState : m_samplers)
				{
					vkDestroySampler(m_device, samperState, nullptr);
				}
				m_samplers.fill(nullptr);

				CleanupSwapChain();

				vkDestroySemaphore(m_device, m_renderFinishedSemaphore, nullptr);
				vkDestroySemaphore(m_device, m_imageAvailableSemaphore, nullptr);

				vkDestroyCommandPool(m_device, m_commandPool, nullptr);

				vkDestroyDevice(m_device, nullptr);
				DestroyDebugReportCallbackEXT(m_instance, m_callback, nullptr);
				vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
				vkDestroyInstance(m_instance, nullptr);

				glfwDestroyWindow(m_pWindow);
				glfwTerminate();
			}

			void Device::Impl::Run(std::function<bool()> funcUpdate)
			{
				while (glfwWindowShouldClose(m_pWindow) == 0)
				{
					glfwPollEvents();

					Render();
				}

				vkDeviceWaitIdle(m_device);
			}

			void Device::Impl::Cleanup(float elapsedTime)
			{
			}

			void Device::Impl::Render()
			{
				VkResult result = vkAcquireNextImageKHR(m_device, m_swapChain, std::numeric_limits<uint64_t>::max(), m_imageAvailableSemaphore, VK_NULL_HANDLE, &m_frameIndex);

				if (result == VK_ERROR_OUT_OF_DATE_KHR)
				{
					RecreateSwapChain();
					return;
				}
				else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
				{
					throw_line("failed to acquire swap chain image");
				}

				VkCommandBuffer commandBuffer = m_vecCommandBuffers[m_frameIndex];

				vkResetCommandBuffer(commandBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);

				VkCommandBufferBeginInfo beginInfo{};
				beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
				beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

				vkBeginCommandBuffer(commandBuffer, &beginInfo);

				m_pRenderManager->Render();

				//////////////////////////////////////////
				//
				//const ImageBuffer* pRenderTarget = m_pRenderManager->GetImageBuffer();
				//ImageBarrier(commandBuffer, pRenderTarget->GetImage(),
				//	VK_ACCESS_MEMORY_WRITE_BIT | VK_ACCESS_HOST_WRITE_BIT,
				//	VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_TRANSFER_READ_BIT,
				//	VK_IMAGE_LAYOUT_UNDEFINED,
				//	VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
				//
				//ImageBarrier(commandBuffer, m_vecSwapChainImages[m_frameIndex]->GetImage(),
				//	0,
				//	VK_ACCESS_MEMORY_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT,
				//	VK_IMAGE_LAYOUT_UNDEFINED,
				//	VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
				//
				//VkImageCopy copyInfo{};
				//copyInfo.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				//copyInfo.srcSubresource.layerCount = 1;
				//copyInfo.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				//copyInfo.dstSubresource.layerCount = 1;
				//copyInfo.extent.width = pRenderTarget->GetSize().x;
				//copyInfo.extent.height = pRenderTarget->GetSize().y;
				//copyInfo.extent.depth = 1;
				//
				//vkCmdCopyImage(commandBuffer, pRenderTarget->GetImage(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, m_vecSwapChainImages[m_frameIndex]->GetImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyInfo);
				//
				//ImageBarrier(commandBuffer, m_vecSwapChainImages[m_frameIndex]->GetImage(), VK_ACCESS_MEMORY_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_MEMORY_READ_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
				//ImageBarrier(commandBuffer, pRenderTarget->GetImage(), VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_TRANSFER_READ_BIT, VK_ACCESS_MEMORY_WRITE_BIT | VK_ACCESS_HOST_WRITE_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL);
				//
				//////////////////////////////////////////
				if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
				{
					throw_line("failed to record command buffer");
				}

				VkSubmitInfo submitInfo{};
				submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

				VkSemaphore waitSemaphores[] = { m_imageAvailableSemaphore };
				VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
				submitInfo.waitSemaphoreCount = 1;
				submitInfo.pWaitSemaphores = waitSemaphores;
				submitInfo.pWaitDstStageMask = waitStages;

				submitInfo.commandBufferCount = 1;
				submitInfo.pCommandBuffers = &m_vecCommandBuffers[m_frameIndex];

				VkSemaphore signalSemaphores[] = { m_renderFinishedSemaphore };
				submitInfo.signalSemaphoreCount = 1;
				submitInfo.pSignalSemaphores = signalSemaphores;

				if (vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
				{
					throw_line("failed to submit draw command buffer!");
				}

				VkPresentInfoKHR presentInfo{};
				presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

				presentInfo.waitSemaphoreCount = 1;
				presentInfo.pWaitSemaphores = signalSemaphores;

				VkSwapchainKHR swapChains[] = { m_swapChain };
				presentInfo.swapchainCount = 1;
				presentInfo.pSwapchains = swapChains;

				presentInfo.pImageIndices = &m_frameIndex;

				result = vkQueuePresentKHR(m_presentQueue, &presentInfo);

				if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
				{
					RecreateSwapChain();
				}
				else if (result != VK_SUCCESS)
				{
					throw_line("failed to present swap chain image!");
				}

				vkQueueWaitIdle(m_presentQueue);
			}

			void Device::Impl::InitializeWindow(uint32_t width, uint32_t height, bool isFullScreen, const string::StringID& applicationTitle, const string::StringID& applicationName)
			{
				if (glfwInit() == GLFW_FALSE)
				{
					throw_line("failed to glfw initialize");
				}

				m_applicationTitle = applicationTitle;
				m_applicationName = applicationName;

				m_screenSize.x = width;
				m_screenSize.y = height;

				glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

				const std::string title = string::WideToMulti(applicationTitle.c_str());
				m_pWindow = glfwCreateWindow(static_cast<int>(width), static_cast<int>(height), title.c_str(), nullptr, nullptr);

				glfwSetWindowUserPointer(m_pWindow, this);
				glfwSetWindowSizeCallback(m_pWindow, Device::Impl::OnWindowResized);
			}

			void Device::Impl::InitializeVK()
			{
				if (EnableValidationLayers == true && CheckValidationLayerSupport() == false)
				{
					throw_line("validation layers requested, but no available!");
				}

				CreateInstance();

				SetDebugCallback();
				CreateSurface();
				PickPhysicsDevice();
				CreateLogicalDevice();
				CreateSwapChain();
				CreateCommandPool();
				CreateCommandBuffers();
				CreateSemaphores();
				CreateSampler();
			}

			void Device::Impl::CleanupSwapChain()
			{
				vkFreeCommandBuffers(m_device, m_commandPool, static_cast<uint32_t>(m_vecCommandBuffers.size()), m_vecCommandBuffers.data());
				m_vecCommandBuffers.clear();

				m_vecSwapChainImages.clear();
				vkDestroySwapchainKHR(m_device, m_swapChain, nullptr);
			}

			void Device::Impl::RecreateSwapChain()
			{
				int width = 0;
				int height = 0;
				glfwGetWindowSize(m_pWindow, &width, &height);
				if (width == 0 || height == 0)
					return;

				if (width == static_cast<int>(m_screenSize.x) &&
					height == static_cast<int>(m_screenSize.y))
					return;

				m_screenSize.x = static_cast<uint32_t>(width);
				m_screenSize.y = static_cast<uint32_t>(height);

				vkDeviceWaitIdle(m_device);

				CleanupSwapChain();

				CreateSwapChain();
				CreateCommandBuffers();

				m_pGBuffers.clear();
				m_pGBuffers.resize(m_nFrameCount);
				for (uint32_t i = 0; i < m_nFrameCount; ++i)
				{
					m_pGBuffers[i] = std::make_unique<GBuffer>(m_swapChainExtent.width, m_swapChainExtent.height);
				}
				//m_pRenderManager->Resize();
			}

			void Device::Impl::CreateInstance()
			{
				VkApplicationInfo appInfo{};
				appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
				appInfo.pApplicationName = string::WideToMulti(m_applicationName.c_str()).c_str();
				appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
				appInfo.pEngineName = "EastEngine";
				appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
				appInfo.apiVersion = VK_API_VERSION_1_0;

				VkInstanceCreateInfo createInfo{};
				createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
				createInfo.pApplicationInfo = &appInfo;

				uint32_t nGlfwExtensionCount = 0;
				const char** ppGlfwExtensions = glfwGetRequiredInstanceExtensions(&nGlfwExtensionCount);

				std::vector<const char*> vecExtensionNames(ppGlfwExtensions, ppGlfwExtensions + nGlfwExtensionCount);

				if constexpr (EnableValidationLayers)
				{
					vecExtensionNames.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
				}

				createInfo.enabledExtensionCount = static_cast<uint32_t>(vecExtensionNames.size());
				createInfo.ppEnabledExtensionNames = vecExtensionNames.data();

				if constexpr (EnableValidationLayers)
				{
					createInfo.enabledLayerCount = static_cast<uint32_t>(ValidationLayers.size());
					createInfo.ppEnabledLayerNames = ValidationLayers.data();
				}
				else
				{
					createInfo.enabledLayerCount = 0;
				}

				VkResult result = vkCreateInstance(&createInfo, nullptr, &m_instance);
				if (result != VK_SUCCESS)
				{
					throw_line("failed to create instance!");
				}
			}

			void Device::Impl::CreateSurface()
			{
				if (glfwCreateWindowSurface(m_instance, m_pWindow, nullptr, &m_surface) != VK_SUCCESS)
				{
					throw_line("failed to create window surface");
				}
			}

			void Device::Impl::PickPhysicsDevice()
			{
				uint32_t nDeviceCount = 0;
				vkEnumeratePhysicalDevices(m_instance, &nDeviceCount, nullptr);

				if (nDeviceCount == 0)
				{
					throw_line("failed to find GPUs with Vulkan support");
				}

				std::vector<VkPhysicalDevice> vecDevices(nDeviceCount);
				vkEnumeratePhysicalDevices(m_instance, &nDeviceCount, vecDevices.data());

				for (const VkPhysicalDevice& device : vecDevices)
				{
					if (IsDeviceSuitable(device))
					{
						m_physicalDevice = device;
					}
				}

				if (m_physicalDevice == nullptr)
				{
					throw_line("failed to find a wuitable GPU");
				}
			}

			void Device::Impl::CreateLogicalDevice()
			{
				QueueFamilyIndices indices = FindQueueFamilies(m_physicalDevice);

				std::vector<VkDeviceQueueCreateInfo> vecQueueCreateInfos;
				std::set<int> setUniqueQueueFamilies = { indices.nGraphicsFamily, indices.nPresentFamily };

				float fQueuePriority = 1.f;
				for (int nQueueFamily : setUniqueQueueFamilies)
				{
					VkDeviceQueueCreateInfo queueCreateInfo{};
					queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
					queueCreateInfo.queueFamilyIndex = nQueueFamily;
					queueCreateInfo.queueCount = 1;
					queueCreateInfo.pQueuePriorities = &fQueuePriority;
					vecQueueCreateInfos.emplace_back(queueCreateInfo);
				}

				VkPhysicalDeviceFeatures deviceFeatures{};
				deviceFeatures.samplerAnisotropy = VK_TRUE;

				VkDeviceCreateInfo createInfo{};
				createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
				createInfo.pQueueCreateInfos = vecQueueCreateInfos.data();
				createInfo.queueCreateInfoCount = static_cast<uint32_t>(vecQueueCreateInfos.size());
				createInfo.pEnabledFeatures = &deviceFeatures;
				createInfo.enabledExtensionCount = static_cast<uint32_t>(DeviceExtensions.size());
				createInfo.ppEnabledExtensionNames = DeviceExtensions.data();

				if constexpr (EnableValidationLayers)
				{
					createInfo.enabledLayerCount = static_cast<uint32_t>(ValidationLayers.size());
					createInfo.ppEnabledLayerNames = ValidationLayers.data();
				}
				else
				{
					createInfo.enabledLayerCount = 0;
				}

				if (vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device))
				{
					throw_line("failed to create logical device");
				}

				vkGetDeviceQueue(m_device, indices.nGraphicsFamily, 0, &m_graphicsQueue);
				vkGetDeviceQueue(m_device, indices.nPresentFamily, 0, &m_presentQueue);
			}

			void Device::Impl::CreateSwapChain()
			{
				SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(m_physicalDevice);

				VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.vecFormats);
				VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.vecPresentModes);
				VkExtent2D extent = ChooseSwapExtent(swapChainSupport.capabilities);

				m_nFrameCount = swapChainSupport.capabilities.minImageCount + 1;
				if (swapChainSupport.capabilities.maxImageCount > 0 && m_nFrameCount > swapChainSupport.capabilities.maxImageCount)
				{
					m_nFrameCount = swapChainSupport.capabilities.maxImageCount;
				}

				VkSwapchainCreateInfoKHR createInfo{};
				createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
				createInfo.surface = m_surface;
				createInfo.minImageCount = m_nFrameCount;
				createInfo.imageFormat = surfaceFormat.format;
				createInfo.imageColorSpace = surfaceFormat.colorSpace;
				createInfo.imageExtent = extent;
				createInfo.imageArrayLayers = 1;
				createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

				QueueFamilyIndices indices = FindQueueFamilies(m_physicalDevice);
				uint32_t nQueueFamilyIndices[] = { static_cast<uint32_t>(indices.nGraphicsFamily), static_cast<uint32_t>(indices.nPresentFamily) };

				if (indices.nGraphicsFamily != indices.nPresentFamily)
				{
					// VK_SHARING_MODE_CONCURRENT : 명시적 소유권 이전 없이 여러 대기열 패밀리에서 이미지를 사용할 수 있다.
					createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
					createInfo.queueFamilyIndexCount = 2;
					createInfo.pQueueFamilyIndices = nQueueFamilyIndices;
				}
				else
				{
					// VK_SHARING_MODE_EXCLUSIVE : 이미지는 한 번에 하나의 대결 패밀리가 소유하며,
					// 다른 대기열 패밀리에서 사용하기 전에 소유권을 명시적으로 전송해야한다.
					// 이 옵션은 최상의 성ㄴ으을 제공한다.
					createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
					createInfo.queueFamilyIndexCount = 0;
					createInfo.pQueueFamilyIndices = nullptr;
				}

				createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
				createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
				createInfo.presentMode = presentMode;
				createInfo.clipped = VK_TRUE;

				createInfo.oldSwapchain = VK_NULL_HANDLE;

				if (vkCreateSwapchainKHR(m_device, &createInfo, nullptr, &m_swapChain) != VK_SUCCESS)
				{
					throw_line("failed to create swap chain");
				}

				vkGetSwapchainImagesKHR(m_device, m_swapChain, &m_nFrameCount, nullptr);

				std::vector<VkImage> vecImages;
				vecImages.resize(m_nFrameCount);
				vkGetSwapchainImagesKHR(m_device, m_swapChain, &m_nFrameCount, vecImages.data());

				m_vecSwapChainImages.resize(m_nFrameCount);
				for (uint32_t i = 0; i < m_nFrameCount; ++i)
				{
					m_vecSwapChainImages[i] = std::make_unique<SwapChainBuffer>(math::uint2(extent.width, extent.height), vecImages[i], surfaceFormat.format);
				}

				m_swapChainImageFormat = surfaceFormat.format;
				m_swapChainExtent = extent;

				m_viewport.x = 0.f;
				m_viewport.y = 0.f;
				m_viewport.width = static_cast<float>(m_swapChainExtent.width);
				m_viewport.height = static_cast<float>(m_swapChainExtent.height);
				m_viewport.minDepth = 0.f;
				m_viewport.maxDepth = 1.f;
			}

			void Device::Impl::CreateCommandPool()
			{
				QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(m_physicalDevice);

				VkCommandPoolCreateInfo poolInfo{};
				poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
				poolInfo.queueFamilyIndex = queueFamilyIndices.nGraphicsFamily;
				poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

				if (vkCreateCommandPool(m_device, &poolInfo, nullptr, &m_commandPool) != VK_SUCCESS)
				{
					throw_line("failed to create command pool");
				}
			}

			void Device::Impl::CreateCommandBuffers()
			{
				VkDevice device = Device::GetInstance()->GetInterface();
				VkCommandPool commandPool = Device::GetInstance()->GetCommandPool();

				VkExtent2D extent = Device::GetInstance()->GetSwapChainExtent2D();

				m_vecCommandBuffers.resize(m_nFrameCount);

				VkCommandBufferAllocateInfo allocInfo{};
				allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
				allocInfo.commandPool = commandPool;

				// VK_COMMAND_BUFFER_LEVEL
				// VK_COMMAND_BUFFER_LEVEL_PRIMARY : 실행을 위해 대기열에 제출할 수 있지만 다른 명령 버퍼에서 호출 할 수는 없습니다.
				// VK_COMMAND_BUFFER_LEVEL_SECONDARY : 직접 제출할 수 는 없지만 기본 명령 버퍼에서 호출 할 수 있습니다.
				allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
				allocInfo.commandBufferCount = m_nFrameCount;

				if (vkAllocateCommandBuffers(device, &allocInfo, m_vecCommandBuffers.data()) != VK_SUCCESS)
				{
					throw_line("failed to allocate command buffers");
				}
			}

			void Device::Impl::CreateSemaphores()
			{
				VkSemaphoreCreateInfo semaphoreInfo{};
				semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

				if (vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_imageAvailableSemaphore) != VK_SUCCESS ||
					vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_renderFinishedSemaphore) != VK_SUCCESS)
				{
					throw_line("failed to create semaphore");
				}
			}

			void Device::Impl::CreateSampler()
			{
				for (int i = 0; i < SamplerState::TypeCount; ++i)
				{
					SamplerState::Type emType = static_cast<SamplerState::Type>(i);
					VkSamplerCreateInfo samplerCreateInfo = util::GetSamplerCreateInfo(emType);

					if (vkCreateSampler(m_device, &samplerCreateInfo, nullptr, &m_samplers[i]) != VK_SUCCESS)
					{
						throw_line("failed to create texture sampler");
					}
				}
			}

			void Device::Impl::SetDebugCallback()
			{
				if constexpr (EnableValidationLayers == false)
					return;

				VkDebugReportCallbackCreateInfoEXT createInfo{};
				createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
				createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
				createInfo.pfnCallback = DebugCallback;

				if (CreateDebugReportCallbackEXT(m_instance, &createInfo, nullptr, &m_callback) != VK_SUCCESS)
				{
					throw_line("failed to set up debug callback");
				}
			}

			VkResult Device::Impl::CreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback)
			{
				auto func = reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT"));
				if (func != nullptr)
				{
					return func(instance, pCreateInfo, pAllocator, pCallback);
				}
				else
				{
					return VK_ERROR_EXTENSION_NOT_PRESENT;
				}
			}

			void Device::Impl::DestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator)
			{
				auto func = reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT"));
				if (func != nullptr)
				{
					return func(instance, callback, pAllocator);
				}
			}

			VkCommandBuffer Device::Impl::BeginSingleTimeCommands()
			{
				VkCommandBufferAllocateInfo allocInfo{};
				allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
				allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
				allocInfo.commandPool = m_commandPool;
				allocInfo.commandBufferCount = 1;

				VkCommandBuffer commandBuffer{};
				vkAllocateCommandBuffers(m_device, &allocInfo, &commandBuffer);

				VkCommandBufferBeginInfo beginInfo{};
				beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
				beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

				vkBeginCommandBuffer(commandBuffer, &beginInfo);

				return commandBuffer;
			}

			void Device::Impl::EndSingleTimeCommands(VkCommandBuffer commandBuffer)
			{
				vkEndCommandBuffer(commandBuffer);

				VkSubmitInfo submitInfo{};
				submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
				submitInfo.commandBufferCount = 1;
				submitInfo.pCommandBuffers = &commandBuffer;

				vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
				vkQueueWaitIdle(m_graphicsQueue);

				vkFreeCommandBuffers(m_device, m_commandPool, 1, &commandBuffer);
			}

			void Device::Impl::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
			{
				VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

				VkBufferCopy copyRegion{};
				copyRegion.size = size;
				vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

				EndSingleTimeCommands(commandBuffer);
			}

			void Device::Impl::CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
			{
				VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

				VkBufferImageCopy region{};
				region.bufferOffset = 0;
				region.bufferRowLength = 0;
				region.bufferImageHeight = 0;
				region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				region.imageSubresource.mipLevel = 0;
				region.imageSubresource.baseArrayLayer = 0;
				region.imageSubresource.layerCount = 1;
				region.imageOffset = { 0, 0, 0 };
				region.imageExtent = { width, height, 1 };

				vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

				EndSingleTimeCommands(commandBuffer);
			}

			void Device::Impl::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer* buffer, VkDeviceMemory* bufferMemory, void** ppData_out)
			{
				VkBufferCreateInfo bufferInfo{};
				bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
				bufferInfo.size = size;
				bufferInfo.usage = usage;
				bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

				if (vkCreateBuffer(m_device, &bufferInfo, nullptr, buffer) != VK_SUCCESS)
				{
					throw_line("failed to create buffer!");
				}

				VkMemoryRequirements memRequirements{};
				vkGetBufferMemoryRequirements(m_device, *buffer, &memRequirements);

				VkMemoryAllocateInfo allocInfo{};
				allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
				allocInfo.allocationSize = memRequirements.size;
				allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

				if (vkAllocateMemory(m_device, &allocInfo, nullptr, bufferMemory) != VK_SUCCESS)
				{
					throw_line("failed to allocate buffer memory!");
				}

				vkBindBufferMemory(m_device, *buffer, *bufferMemory, 0);

				if (ppData_out != nullptr)
				{
					VkResult result = vkMapMemory(m_device, *bufferMemory, 0, size, 0, ppData_out);
					if (result != VK_SUCCESS)
					{
						throw_line("failed to uniform buffer map");
					}
				}
			}

			void Device::Impl::CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage* image_out, VkDeviceMemory* imageMemory_out)
			{
				VkImageCreateInfo imageInfo{};
				imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
				imageInfo.imageType = VK_IMAGE_TYPE_2D;
				imageInfo.extent.width = width;
				imageInfo.extent.height = height;
				imageInfo.extent.depth = 1;
				imageInfo.mipLevels = 1;
				imageInfo.arrayLayers = 1;
				imageInfo.format = format;
				imageInfo.tiling = tiling;
				imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				imageInfo.usage = usage;
				imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
				imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
				imageInfo.flags = 0;

				if (vkCreateImage(m_device, &imageInfo, nullptr, image_out) != VK_SUCCESS)
				{
					throw_line("failed to create image");
				}

				VkMemoryRequirements memRequirements{};
				vkGetImageMemoryRequirements(m_device, *image_out, &memRequirements);

				VkMemoryAllocateInfo allocInfo{};
				allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
				allocInfo.allocationSize = memRequirements.size;
				allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

				if (vkAllocateMemory(m_device, &allocInfo, nullptr, imageMemory_out) != VK_SUCCESS)
				{
					throw_line("failed to allocate image memory");
				}

				vkBindImageMemory(m_device, *image_out, *imageMemory_out, 0);
			}

			VkImageView Device::Impl::CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags)
			{
				VkImageViewCreateInfo viewInfo{};
				viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
				viewInfo.image = image;
				viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
				viewInfo.format = format;
				viewInfo.subresourceRange.aspectMask = aspectFlags;
				viewInfo.subresourceRange.baseMipLevel = 0;
				viewInfo.subresourceRange.levelCount = 1;
				viewInfo.subresourceRange.baseArrayLayer = 0;
				viewInfo.subresourceRange.layerCount = 1;

				VkImageView imageView = nullptr;
				if (vkCreateImageView(m_device, &viewInfo, nullptr, &imageView) != VK_SUCCESS)
				{
					throw_line("failed to create texture image view");
				}

				return imageView;
			}

			void Device::Impl::TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
			{
				VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

				VkImageMemoryBarrier barrier{};
				barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
				barrier.oldLayout = oldLayout;
				barrier.newLayout = newLayout;
				barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				barrier.image = image;

				if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
				{
					barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

					if (IsHasStencilComponent(format) == true)
					{
						barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
					}
				}
				else
				{
					barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				}

				barrier.subresourceRange.baseMipLevel = 0;
				barrier.subresourceRange.levelCount = 1;
				barrier.subresourceRange.baseArrayLayer = 0;
				barrier.subresourceRange.layerCount = 1;

				VkPipelineStageFlags sourceStage{};
				VkPipelineStageFlags destinationStage{};

				if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
				{
					barrier.srcAccessMask = 0;
					barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

					sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
					destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
				}
				else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
				{
					barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
					barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

					sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
					destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
				}
				else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
				{
					barrier.srcAccessMask = 0;
					barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

					sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
					destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
				}
				else
				{
					throw_line("unsupported layout transition");
				}

				vkCmdPipelineBarrier(commandBuffer,
					sourceStage, destinationStage,
					0,
					0, nullptr,
					0, nullptr,
					1, &barrier);

				EndSingleTimeCommands(commandBuffer);
			}

			void Device::Impl::ImageBarrier(VkCommandBuffer commandBuffer, VkImage image, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, VkImageLayout oldLayout, VkImageLayout newLayout)
			{
				VkImageMemoryBarrier imb;
				memset(&imb, 0, sizeof(imb));
				imb.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
				imb.pNext = NULL;
				imb.srcAccessMask = srcAccessMask;
				imb.dstAccessMask = dstAccessMask;
				imb.oldLayout = oldLayout;
				imb.newLayout = newLayout;
				imb.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				imb.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				imb.image = image;
				imb.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				imb.subresourceRange.baseMipLevel = 0;
				imb.subresourceRange.levelCount = 1;
				imb.subresourceRange.baseArrayLayer = 0;
				imb.subresourceRange.layerCount = 1;

				vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, NULL, 0, NULL, 1, &imb);
			}

			bool Device::Impl::CheckValidationLayerSupport() const
			{
				uint32_t nLayerCount = 0;
				vkEnumerateInstanceLayerProperties(&nLayerCount, nullptr);

				if (nLayerCount == 0)
				{
					throw_line("unsupported validation layer");
				}

				std::vector<VkLayerProperties> vecAvailableLayers(nLayerCount);
				vkEnumerateInstanceLayerProperties(&nLayerCount, vecAvailableLayers.data());

				for (const char* strLayerName : ValidationLayers)
				{
					bool isLayerFound = false;

					for (const VkLayerProperties& layerProperties : vecAvailableLayers)
					{
						if (string::IsEquals(strLayerName, layerProperties.layerName) == true)
						{
							isLayerFound = true;
							break;
						}
					}

					if (isLayerFound == false)
						return false;
				}

				return true;
			}

			bool Device::Impl::CheckDeviceExtensionSupport(VkPhysicalDevice device) const
			{
				uint32_t nExtensionCount = 0;
				vkEnumerateDeviceExtensionProperties(device, nullptr, &nExtensionCount, nullptr);

				std::vector<VkExtensionProperties> vecAvailableExtensions(nExtensionCount);
				vkEnumerateDeviceExtensionProperties(device, nullptr, &nExtensionCount, vecAvailableExtensions.data());

				std::set<std::string> requiredExtensions(DeviceExtensions.begin(), DeviceExtensions.end());

				for (const VkExtensionProperties& extension : vecAvailableExtensions)
				{
					requiredExtensions.erase(extension.extensionName);
				}

				return requiredExtensions.empty();
			}

			bool Device::Impl::IsDeviceSuitable(VkPhysicalDevice device) const
			{
				QueueFamilyIndices indices = FindQueueFamilies(device);

				const bool isExtensionSupported = CheckDeviceExtensionSupport(device);

				bool isEnableSwapChainAdequate = false;
				if (isExtensionSupported == true)
				{
					SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(device);
					isEnableSwapChainAdequate = swapChainSupport.vecFormats.empty() == false && swapChainSupport.vecPresentModes.empty() == false;
				}

				VkPhysicalDeviceFeatures supportedFeatures{};
				vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

				return indices.IsComplete() == true && isExtensionSupported == true && isEnableSwapChainAdequate == true && supportedFeatures.samplerAnisotropy == VK_TRUE;
			}

			QueueFamilyIndices Device::Impl::FindQueueFamilies(VkPhysicalDevice device) const
			{
				QueueFamilyIndices indices;

				uint32_t nQueueFamilyCount = 0;
				vkGetPhysicalDeviceQueueFamilyProperties(device, &nQueueFamilyCount, nullptr);

				std::vector<VkQueueFamilyProperties> vecQueueFamilies(nQueueFamilyCount);
				vkGetPhysicalDeviceQueueFamilyProperties(device, &nQueueFamilyCount, vecQueueFamilies.data());

				int i = 0;
				for (const VkQueueFamilyProperties& queueFamily : vecQueueFamilies)
				{
					if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
					{
						indices.nGraphicsFamily = i;
					}

					VkBool32 presentSupport = VK_FALSE;
					vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_surface, &presentSupport);

					if (queueFamily.queueCount > 0 && presentSupport == VK_TRUE)
					{
						indices.nPresentFamily = i;
					}

					if (indices.IsComplete() == true)
						break;

					++i;
				}

				return indices;
			}

			// CheckDeviceExtensionSupport 으로 스왑체인 지원 여부를 체크하는 것만으로는 충분하지 않음
			// 실제 스왑체인이 윈도우 표면(surface)와 호환되지 않을 수 있기 때문
			// 스왑체인을 생성하면 인스턴스 및 장치 생성보다 많은 설정이 포함되므로 진행하기 전에 더 자세한 내용을 쿼리 해야 함
			// 확인해야할 속성 3가지
			// 1. 기본 표면 기능 : 스왑체인의 최소 / 최대 이미지 수, 최소 / 최대 너비 및 이미지 높이
			// 2. 표면 포맷 : 픽셀 포맷, 컬러 스페이스
			// 3. 사용가능한 프레젠테이션 모드
			SwapChainSupportDetails Device::Impl::QuerySwapChainSupport(VkPhysicalDevice device) const
			{
				SwapChainSupportDetails details;

				vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_surface, &details.capabilities);

				uint32_t nFormatCount = 0;
				vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &nFormatCount, nullptr);

				if (nFormatCount != 0)
				{
					details.vecFormats.resize(nFormatCount);
					vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &nFormatCount, details.vecFormats.data());
				}

				uint32_t nPresentModeCount = 0;
				vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &nPresentModeCount, nullptr);

				if (nPresentModeCount != 0)
				{
					details.vecPresentModes.resize(nPresentModeCount);
					vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &nPresentModeCount, details.vecPresentModes.data());
				}

				return details;
			}

			// QuerySwapChainSupport 으로 지원 가능여부를 확인할 수 있지만, 여전히 다양한 모드가 있을 수 있다.
			// 가능한 스왑체인을 위한 올바른 설정을 찾기 위해 3가지 유형의 설정이 필요
			// 1. 표면 포맷(색상 깊이)
			// 2. 프리젠테이션 모드 (이미지를 화면에 "스왑"하기 위한 조건)
			// 3. 스왑 범위 (스왑 체인의 이미지 해상도)
			VkSurfaceFormatKHR Device::Impl::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& vecAvailableFormats) const
			{
				if (vecAvailableFormats.size() == 1 && vecAvailableFormats[0].format == VK_FORMAT_UNDEFINED)
				{
					return { VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
				}

				for (const VkSurfaceFormatKHR& availableFormat : vecAvailableFormats)
				{
					if (availableFormat.format == VK_FORMAT_R8G8B8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
					{
						return availableFormat;
					}
				}

				return vecAvailableFormats[0];
			}

			// 화면에 이미지를 표시하기 위한 실제 조건을 나타내는 스왑 체인의 가장 중요한 설정
			// 4가지 모드가 존재
			// VK_PRESENT_MODE_IMMEDIATE_KHR : 응용 프로그램에서 제출한 이미지가 즉시 화면으로 전송되어 끊어질 수 있다.
			// VK_PRESENT_MODE_FIFO_KHR : 이미지가 큐의 뒤쪽에 삽입 할 때, 디스플레이가 큐의 제일 앞 이미지를 가져오는 방식
			//							현대 게임에서 볼 수 있는 수직 동기화와 가장 유사하다.
			//							디스플레이가 새로 고쳐지는 순간을 "수직 공백"이라고 한다.
			// VK_PRESENT_MODE_FIFO_RELAXED_KHR : 응용 프로그램이 느려, 마지막 수직 공백에서 큐가 비어있는 경우에만 이전 모드와 다르게 동작
			//							다음 수직 여백을 기다리지 않고 이미지가 도착하면 곧바로 전송
			//							이로 인해 육안으로 눈물이 생길 수 있다???
			// VK_PRESENT_MODE_MAILBOX_KHR : 2번째 모드의 다른 변형, 대기열이 가득 차면 응용프로그램을 차단하는 대신
			//							이미 대기열에 있는 이미지를 새로운 대기열로 간단하게 대체 할 수 있다.
			//							이 모드는 트리플 버퍼링을 구현하는데 사용 가능
			//							더블 버퍼링을 사용하는 표준 수직 동기화에서 적은 대기 시간으로 인해 발생하는 찢어짐 문제를 방지 할 수 있다.
			VkPresentModeKHR Device::Impl::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& vecAvailablePresentModes) const
			{
				VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;

				for (const VkPresentModeKHR& availablePresentMode : vecAvailablePresentModes)
				{
					if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
					{
						return availablePresentMode;
					}
					else if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR)
					{
						bestMode = availablePresentMode;
					}
				}

				return bestMode;
			}

			VkExtent2D Device::Impl::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) const
			{
				if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
				{
					return capabilities.currentExtent;
				}
				else
				{
					VkExtent2D actualExtent = { m_screenSize.x, m_screenSize.y };
					actualExtent.width = std::clamp(capabilities.minImageExtent.width, capabilities.maxImageExtent.width, actualExtent.width);
					actualExtent.height = std::clamp(capabilities.minImageExtent.height, capabilities.maxImageExtent.height, actualExtent.height);

					return actualExtent;
				}
			}

			VkFormat Device::Impl::FindSupportedFormat(const std::vector<VkFormat>& vecCandidates, VkImageTiling tiling, VkFormatFeatureFlags features) const
			{
				for (VkFormat format : vecCandidates)
				{
					VkFormatProperties props{};
					vkGetPhysicalDeviceFormatProperties(m_physicalDevice, format, &props);

					if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
					{
						return format;
					}
					else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
					{
						return format;
					}
				}

				throw_line("failed to find supported format");
			}

			uint32_t Device::Impl::FindMemoryType(uint32_t nTypeFilter, VkMemoryPropertyFlags properties) const
			{
				VkPhysicalDeviceMemoryProperties memProperties{};
				vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &memProperties);

				for (uint32_t i = 0; i < memProperties.memoryTypeCount; ++i)
				{
					if ((nTypeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
					{
						return i;
					}
				}

				throw_line("failed to find suitable memory type");
			}

			bool Device::Impl::IsHasStencilComponent(VkFormat format) const
			{
				return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
			}

			VkFormat Device::Impl::FindDepthFormat() const
			{
				return FindSupportedFormat(
					{
						VK_FORMAT_D32_SFLOAT,
						VK_FORMAT_D32_SFLOAT_S8_UINT,
						VK_FORMAT_D24_UNORM_S8_UINT,
					},
					VK_IMAGE_TILING_OPTIMAL,
					VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
					);
			}

			void Device::Impl::OnWindowResized(GLFWwindow* pWindow, int width, int height)
			{
				Device::Impl* pApplication = reinterpret_cast<Device::Impl*>(glfwGetWindowUserPointer(pWindow));
				pApplication->RecreateSwapChain();
			}

			Device::Device()
				: m_pImpl{ std::make_unique<Impl>() }
			{
			}

			Device::~Device()
			{
			}

			void Device::Initialize(uint32_t width, uint32_t height, bool isFullScreen, const string::StringID& applicationTitle, const string::StringID& applicationName, std::function<HRESULT(HWND, uint32_t, WPARAM, LPARAM)> messageHandler)
			{
				m_pImpl->Initialize(width, height, isFullScreen, applicationTitle, applicationName);
			}

			void Device::Run(std::function<bool()> funcUpdate)
			{
				m_pImpl->Run(funcUpdate);
			}

			void Device::Cleanup(float elapsedTime)
			{
				m_pImpl->Cleanup(elapsedTime);
			}

			const math::uint2& Device::GetScreenSize() const
			{
				return m_pImpl->GetScreenSize();
			}

			const math::Viewport& Device::GetViewport() const
			{
				return m_pImpl->GetViewport();
			}

			const std::vector<DisplayModeDesc>& Device::GetSupportedDisplayModeDesc() const
			{
				return m_pImpl->GetSupportedDisplayModeDesc();
			}

			size_t Device::GetSelectedDisplayModeIndex() const
			{
				return m_pImpl->GetSelectedDisplayModeIndex();
			}

			VkDevice Device::GetInterface() const
			{
				return m_pImpl->GetInterface();
			}

			VkCommandPool Device::GetCommandPool() const
			{
				return m_pImpl->GetCommandPool();
			}

			uint32_t Device::GetFrameIndex() const
			{
				return m_pImpl->GetFrameIndex();
			}

			uint32_t Device::GetFrameCount() const
			{
				return m_pImpl->GetFrameCount();
			}

			VkCommandBuffer Device::GetCommandBuffer(size_t index) const
			{
				return m_pImpl->GetCommandBuffer(index);
			}

			VkFormat Device::GetSwapChainImageFormat() const
			{
				return m_pImpl->GetSwapChainImageFormat();
			}

			VkExtent2D Device::GetSwapChainExtent2D() const
			{
				return m_pImpl->GetSwapChainExtent2D();
			}

			VkSemaphore Device::GetImageAvailableSemaphore() const
			{
				return m_pImpl->GetImageAvailableSemaphore();
			}

			VkSemaphore Device::GetRenderFinishedSemaphore() const
			{
				return m_pImpl->GetRenderFinishedSemaphore();
			}

			VkSampler Device::GetSampler(SamplerState::Type emType) const
			{
				return m_pImpl->GetSampler(emType);
			}

			GBuffer* Device::GetGBuffer(int frameIndex) const
			{
				return m_pImpl->GetGBuffer(frameIndex);
			}

			IImageBasedLight* Device::GetImageBasedLight() const
			{
				return m_pImpl->GetImageBasedLight();
			}

			void Device::SetImageBasedLight(IImageBasedLight* pImageBasedLight)
			{
				m_pImpl->SetImageBasedLight(pImageBasedLight);
			}

			RenderManager* Device::GetRenderManager() const
			{
				return m_pImpl->GetRenderManager();
			}

			const Texture* Device::GetEmptyTexture() const
			{
				return m_pImpl->GetEmptyTexture();
			}

			void Device::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
			{
				return m_pImpl->CopyBuffer(srcBuffer, dstBuffer, size);
			}

			void Device::CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
			{
				m_pImpl->CopyBufferToImage(buffer, image, width, height);
			}

			void Device::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer* buffer, VkDeviceMemory* bufferMemory, void** ppData_out)
			{
				return m_pImpl->CreateBuffer(size, usage, properties, buffer, bufferMemory, ppData_out);
			}

			void Device::CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage* image_out, VkDeviceMemory* imageMemory_out)
			{
				return m_pImpl->CreateImage(width, height, format, tiling, usage, properties, image_out, imageMemory_out);
			}

			VkImageView Device::CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags)
			{
				return m_pImpl->CreateImageView(image, format, aspectFlags);
			}

			void Device::TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
			{
				m_pImpl->TransitionImageLayout(image, format, oldLayout, newLayout);
			}

			VkFormat Device::FindDepthFormat() const
			{
				return m_pImpl->FindDepthFormat();
			}

			std::vector<char> Device::ReadFile(const std::string& filename)
			{
				std::ifstream file(filename, std::ios::ate | std::ios::binary);

				if (file.is_open() == false)
				{
					throw_line("failed to open file");
				}

				const size_t fileSize = static_cast<size_t>(file.tellg());
				std::vector<char> buffer(fileSize);

				file.seekg(0);
				file.read(buffer.data(), fileSize);

				file.close();

				return buffer;
			}
		}
	}
}