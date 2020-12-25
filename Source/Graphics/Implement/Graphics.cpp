#include "stdafx.h"
#include "Graphics.h"

#include "CommonLib/Lock.h"
#include "CommonLib/FileUtil.h"

#include "Graphics/Interface/Camera.h"
#include "Graphics/Interface/LightManager.h"
#include "Graphics/Interface/imguiHelper.h"
#include "Graphics/Interface/OcclusionCulling.h"

#include "Graphics/APIs/DirectX11/DeviceDX11.h"
#include "Graphics/APIs/DirectX11/VertexBufferDX11.h"
#include "Graphics/APIs/DirectX11/IndexBufferDX11.h"
#include "Graphics/APIs/DirectX11/TextureDX11.h"
#include "Graphics/APIs/DirectX11/RenderManagerDX11.h"
#include "Graphics/APIs/DirectX11/VTFManagerDX11.h"
#include "Graphics/APIs/DirectX11/RenderTargetDX11.h"

#include "Graphics/APIs/DirectX12/DeviceDX12.h"
#include "Graphics/APIs/DirectX12/VertexBufferDX12.h"
#include "Graphics/APIs/DirectX12/IndexBufferDX12.h"
#include "Graphics/APIs/DirectX12/TextureDX12.h"
#include "Graphics/APIs/DirectX12/RenderManagerDX12.h"
#include "Graphics/APIs/DirectX12/VTFManagerDX12.h"
#include "Graphics/APIs/DirectX12/RenderTargetDX12.h"

#include "Graphics/APIs/Vulkan/DeviceVulkan.h"
#include "Graphics/APIs/Vulkan/VertexBufferVulkan.h"
#include "Graphics/APIs/Vulkan/IndexBufferVulkan.h"
#include "Graphics/APIs/Vulkan/TextureVulkan.h"
#include "Graphics/APIs/Vulkan/RenderManagerVulkan.h"

#include "TextureManager.h"
#include "Material.h"
#include "ImageBasedLight.h"

namespace est
{
	namespace graphics
	{
		class IGraphicsAPI
		{
		public:
			IGraphicsAPI() = default;
			virtual ~IGraphicsAPI() = default;

		public:
			virtual void Initialize(uint32_t width, uint32_t height, bool isFullScreen, const string::StringID& applicationTitle, const string::StringID& applicationName, std::function<HRESULT(HWND, uint32_t, WPARAM, LPARAM)> messageHandler) = 0;
			virtual void Release() = 0;
			virtual void Run(std::function<bool()> funcUpdate) = 0;
			virtual void Update(float elapsedTime) = 0;
			virtual void PostUpdate(float elapsedTime) = 0;

		public:
			virtual void ScreenShot(ScreenShotFormat format, const std::wstring& path, std::function<void(bool, const std::wstring&)> screenShotCallback) = 0;

		public:
			virtual APIs GetType() const = 0;
			virtual HWND GetHwnd() const = 0;
			virtual HINSTANCE GetHInstance() const = 0;

			virtual const math::uint2& GetScreenSize() const = 0;
			virtual const math::Viewport& GetViewport() const = 0;

			virtual bool IsFullScreen() const = 0;
			virtual void SetFullScreen(bool isFullScreen, std::function<void(bool)> callback) = 0;

			virtual const std::vector<DisplayModeDesc>& GetSupportedDisplayModeDesc() const = 0;
			virtual size_t GetSelectedDisplayModeIndex() const = 0;
			virtual void ChangeDisplayMode(size_t displayModeIndex, std::function<void(bool)> callback) = 0;

			virtual IImageBasedLight* GetImageBasedLight() const = 0;
			virtual IVTFManager* GetVTFManager() const = 0;

			virtual VertexBufferPtr CreateVertexBuffer(const uint8_t* pData, uint32_t vertexCount, size_t formatSize, bool isDynamic) = 0;
			virtual IndexBufferPtr CreateIndexBuffer(const uint8_t* pData, uint32_t indexCount, size_t formatSize, bool isDynamic) = 0;
			virtual TexturePtr CreateTexture(const wchar_t* filePath) = 0;
			virtual TexturePtr CreateTextureAsync(const wchar_t* filePath) = 0;
			virtual TexturePtr CreateTexture(const TextureDesc& desc) = 0;
			virtual MaterialPtr CreateMaterial(const IMaterial::Data* pMaterialData) = 0;
			virtual MaterialPtr CreateMaterial(const wchar_t* fileName, const wchar_t* filePath) = 0;
			virtual MaterialPtr CloneMaterial(const IMaterial* pMaterialSource) = 0;

			virtual DirectionalLightPtr CreateDirectionalLight(const string::StringID& name, bool isEnableShadow, const DirectionalLightData& lightData) = 0;
			virtual PointLightPtr CreatePointLight(const string::StringID& name, bool isEnableShadow, const PointLightData& lightData) = 0;
			virtual SpotLightPtr CreateSpotLight(const string::StringID& name, bool isEnableShadow, const SpotLightData& lightData) = 0;
			virtual size_t GetLightCount(ILight::Type type) const = 0;
			virtual LightPtr GetLight(ILight::Type type, size_t index) const = 0;

			virtual void PushRenderJob(const RenderJobStatic& renderJob) = 0;
			virtual void PushRenderJob(const RenderJobSkinned& renderJob) = 0;
			virtual void PushRenderJob(const RenderJobTerrain& renderJob) = 0;
			virtual void PushRenderJob(const RenderJobVertex& renderJob) = 0;
		};

		template <
			APIs APIType,
			typename Device,
			typename RenderManager,
			typename VertexBuffer,
			typename IndexBuffer,
			typename Texture
		>
			class TGraphicsAPI : public IGraphicsAPI
		{
		public:
			TGraphicsAPI() = default;
			virtual ~TGraphicsAPI() = default;

		public:
			virtual void Initialize(uint32_t width, uint32_t height, bool isFullScreen, const string::StringID& applicationTitle, const string::StringID& applicationName, std::function<HRESULT(HWND, uint32_t, WPARAM, LPARAM)> messageHandler) override
			{
				s_pLightManager = LightManager::GetInstance();

				Device::GetInstance()->Initialize(width, height, isFullScreen, applicationTitle, applicationName, messageHandler);
				m_pTextureManager = std::make_unique<TextureManager>();

				m_pImageBasedLight = std::make_unique<ImageBasedLight>();
				Device::GetInstance()->SetImageBasedLight(m_pImageBasedLight.get());

				Camera::DescProjection cameraProjection;
				cameraProjection.width = width;
				cameraProjection.height = height;
				cameraProjection.fov = math::PIDIV4;
				cameraProjection.nearClip = 0.1f;
				cameraProjection.farClip = 1000.f;
				GetCamera().SetProjection(cameraProjection);

				s_pOcclusionCulling = OcclusionCulling::GetInstance();
				s_pOcclusionCulling->Initialize(width, height);
			}

			virtual void Release() override
			{
				RenderManager* pRenderManager = Device::GetInstance()->GetRenderManager();
				pRenderManager->AllCleanup();

				OcclusionCulling::DestroyInstance();
				s_pOcclusionCulling = nullptr;

				s_pLightManager->RemoveAll();
				LightManager::DestroyInstance();
				s_pLightManager = nullptr;

				m_pImageBasedLight.reset();

				m_pTextureManager.reset();
				Device::DestroyInstance();
			}

			virtual void Run(std::function<bool()> funcUpdate) override
			{
				Device::GetInstance()->Run(funcUpdate);
			}

			virtual void Update(float elapsedTime) override
			{
				m_pTextureManager->Cleanup(elapsedTime);

				GetPrevDebugInfo() = GetDebugInfo();
				GetDebugInfo() = DebugInfo();
				GetDebugInfo().isEnableCollection = GetPrevDebugInfo().isEnableCollection;

				GetCamera().Update(elapsedTime);

				s_pOcclusionCulling->Enable(GetOptions().OnOcclusionCulling);
				s_pOcclusionCulling->Update(&GetCamera());

				s_pOcclusionCulling->WakeThreads();
				s_pOcclusionCulling->ClearBuffer();
			}

			virtual void PostUpdate(float elapsedTime) override
			{
				s_pLightManager->Update(elapsedTime);

				s_pOcclusionCulling->Flush();
				s_pOcclusionCulling->SuspendThreads();
			}

		public:
			virtual void ScreenShot(ScreenShotFormat format, const std::wstring& path, std::function<void(bool, const std::wstring&)> screenShotCallback) override
			{
				Device::GetInstance()->ScreenShot(format, path, screenShotCallback);
			}

		public:
			virtual APIs GetType() const override
			{
				return APIType;
			}

			virtual HWND GetHwnd() const override
			{
				if constexpr (APIType == APIs::eDX11 || APIType == APIs::eDX12)
				{
					return Device::GetInstance()->GetHwnd();
				}
				else
				{
					return nullptr;
				}
			}

			virtual HINSTANCE GetHInstance() const override
			{
				if constexpr (APIType == APIs::eDX11 || APIType == APIs::eDX12)
				{
					return Device::GetInstance()->GetHInstance();
				}
				else
				{
					return nullptr;
				}
			}

			virtual const math::uint2& GetScreenSize() const override
			{
				return Device::GetInstance()->GetScreenSize();
			}

			virtual const math::Viewport& GetViewport() const override
			{
				return Device::GetInstance()->GetViewport();
			}

			virtual bool IsFullScreen() const override
			{
				return Device::GetInstance()->IsFullScreen();
			}

			virtual void SetFullScreen(bool isFullScreen, std::function<void(bool)> callback) override
			{
				Device::GetInstance()->SetFullScreen(isFullScreen, callback);
			}

			virtual const std::vector<DisplayModeDesc>& GetSupportedDisplayModeDesc() const override
			{
				return Device::GetInstance()->GetSupportedDisplayModeDesc();
			}

			virtual size_t GetSelectedDisplayModeIndex() const override
			{
				return Device::GetInstance()->GetSelectedDisplayModeIndex();
			}

			virtual void ChangeDisplayMode(size_t displayModeIndex, std::function<void(bool)> callback) override
			{
				Device::GetInstance()->ChangeDisplayMode(displayModeIndex, callback);
			}

			virtual IImageBasedLight* GetImageBasedLight() const override
			{
				return Device::GetInstance()->GetImageBasedLight();
			}

			virtual IVTFManager* GetVTFManager() const override
			{
				if constexpr (APIType == APIs::eDX11 || APIType == APIs::eDX12)
				{
					return Device::GetInstance()->GetVTFManager();
				}
				else
				{
					return nullptr;
				}
			}

			virtual VertexBufferPtr CreateVertexBuffer(const uint8_t* pData, uint32_t vertexCount, size_t formatSize, bool isDynamic) override
			{
				return std::make_shared<VertexBuffer>(pData, vertexCount, formatSize, isDynamic);
			}

			virtual IndexBufferPtr CreateIndexBuffer(const uint8_t* pData, uint32_t indexCount, size_t formatSize, bool isDynamic) override
			{
				return std::make_shared<IndexBuffer>(pData, indexCount, formatSize, isDynamic);
			}

			virtual TexturePtr CreateTexture(const wchar_t* filePath) override
			{
				const ITexture::Key key{ string::StringID(filePath) };
				TexturePtr pITexture = m_pTextureManager->GetTexture(key);
				if (pITexture != nullptr)
					return pITexture;

				std::shared_ptr<Texture> pTexture = std::make_shared<Texture>(key);
				pTexture->Load(filePath);

				return m_pTextureManager->PushTexture(pTexture);
			}

			virtual TexturePtr CreateTextureAsync(const wchar_t* filePath) override
			{
				const ITexture::Key key{ string::StringID(filePath) };
				TexturePtr pITexture = m_pTextureManager->GetTexture(key);
				if (pITexture != nullptr)
					return pITexture;

				std::shared_ptr<Texture> pTexture = std::make_shared<Texture>(key);

				return m_pTextureManager->AsyncLoadTexture(pTexture, filePath, [](TexturePtr pTextureInterface, const std::wstring& path)
				{
					Texture* pTexture = static_cast<Texture*>(pTextureInterface.get());
					return pTexture->Load(path.c_str());
				});
			}

			virtual TexturePtr CreateTexture(const TextureDesc& desc) override
			{
				const ITexture::Key key{ desc.name };
				TexturePtr pITexture = m_pTextureManager->GetTexture(key);
				if (pITexture != nullptr)
					return pITexture;

				std::shared_ptr<Texture> pTexture = std::make_shared<Texture>(key);
				pTexture->Initialize(desc);

				return m_pTextureManager->PushTexture(pTexture);
			}

			virtual MaterialPtr CreateMaterial(const IMaterial::Data* pMaterialData) override
			{
				return Material::Create(pMaterialData);
			}

			virtual MaterialPtr CreateMaterial(const wchar_t* fileName, const wchar_t* filePath) override
			{
				return Material::Create(fileName, filePath);
			}

			virtual MaterialPtr CloneMaterial(const IMaterial* pMaterialSource) override
			{
				if (pMaterialSource == nullptr)
					return nullptr;

				return Material::Clone(static_cast<const Material*>(pMaterialSource));
			}

			virtual DirectionalLightPtr CreateDirectionalLight(const string::StringID& name, bool isEnableShadow, const DirectionalLightData& lightData) override
			{
				return s_pLightManager->CreateDirectionalLight(name, isEnableShadow, lightData);
			}

			virtual PointLightPtr CreatePointLight(const string::StringID& name, bool isEnableShadow, const PointLightData& lightData) override
			{
				return s_pLightManager->CreatePointLight(name, isEnableShadow, lightData);
			}

			virtual SpotLightPtr CreateSpotLight(const string::StringID& name, bool isEnableShadow, const SpotLightData& lightData) override
			{
				return s_pLightManager->CreateSpotLight(name, isEnableShadow, lightData);
			}

			virtual size_t GetLightCount(ILight::Type type) const
			{
				return s_pLightManager->GetLightCount(type);
			}

			virtual LightPtr GetLight(ILight::Type type, size_t index) const
			{
				return s_pLightManager->GetLight(type, index);
			}

			virtual void PushRenderJob(const RenderJobStatic& renderJob) override
			{
				if (s_pOcclusionCulling->IsEnable() == true)
				{
					const MaterialPtr& pMaterial = renderJob.pMaterial;
					if (pMaterial == nullptr || pMaterial->GetBlendState() == BlendState::eOff)
					{
						const collision::Frustum& cameraFrustum = s_pOcclusionCulling->GetCameraFrustum();
						if (cameraFrustum.Contains(renderJob.occlusionCullingData.aabb) != collision::EmContainment::eDisjoint)
						{
							s_pOcclusionCulling->RenderTriangles(renderJob.worldMatrix, renderJob.occlusionCullingData.pVertices, renderJob.occlusionCullingData.pIndices, renderJob.occlusionCullingData.indexCount);
						}
					}
				}

				RenderManager* pRenderManager = Device::GetInstance()->GetRenderManager();
				pRenderManager->PushJob(renderJob);
			}

			virtual void PushRenderJob(const RenderJobSkinned& renderJob) override
			{
				RenderManager* pRenderManager = Device::GetInstance()->GetRenderManager();
				pRenderManager->PushJob(renderJob);
			}

			virtual void PushRenderJob(const RenderJobTerrain& renderJob) override
			{
				RenderManager* pRenderManager = Device::GetInstance()->GetRenderManager();
				pRenderManager->PushJob(renderJob);
			}

			virtual void PushRenderJob(const RenderJobVertex& renderJob) override
			{
				RenderManager* pRenderManager = Device::GetInstance()->GetRenderManager();
				pRenderManager->PushJob(renderJob);
			}

		protected:
			std::unique_ptr<TextureManager> m_pTextureManager;
			std::unique_ptr<ImageBasedLight> m_pImageBasedLight;

			LightManager* s_pLightManager{ nullptr };
			OcclusionCulling* s_pOcclusionCulling{ nullptr };
		};

#define DeclGraphicsAPI(APIType) using GraphicsAPI = TGraphicsAPI<APIType, Device, RenderManager, VertexBuffer, IndexBuffer, Texture>

		namespace dx11
		{
			DeclGraphicsAPI(APIs::eDX11);
		}

		namespace dx12
		{
			DeclGraphicsAPI(APIs::eDX12);
		}

		namespace vulkan
		{
			DeclGraphicsAPI(APIs::eVulkan);
		}

		std::unique_ptr<IGraphicsAPI> s_pGraphicsAPI;

		void Initialize(APIs emAPI, uint32_t width, uint32_t height, bool isFullScreen, bool isVSync, const string::StringID& applicationTitle, const string::StringID& applicationName, std::function<HRESULT(HWND, uint32_t, WPARAM, LPARAM)> messageHandler)
		{
			if (s_pGraphicsAPI != nullptr)
			{
				throw_line("unsupported change graphics api");
			}

			switch (emAPI)
			{
			case eDX11:
				s_pGraphicsAPI = std::make_unique<dx11::GraphicsAPI>();
				LOG_MESSAGE(L"Graphics API : DirectX11");
				break;
			case eDX12:
				s_pGraphicsAPI = std::make_unique<dx12::GraphicsAPI>();
				LOG_MESSAGE(L"Graphics API : DirectX12");
				break;
			case eVulkan:
				s_pGraphicsAPI = std::make_unique<vulkan::GraphicsAPI>();
				LOG_MESSAGE(L"Graphics API : Vulkan");
				break;
			default:
				throw_line("unknown request");
				break;
			}

			GetOptions().OnVSync = isVSync;

			s_pGraphicsAPI->Initialize(width, height, isFullScreen, applicationTitle, applicationName, messageHandler);

			std::string fontPath = string::WideToMulti(file::GetEngineDataPath());
			fontPath.append("Font\\ArialUni.ttf");

			ImGuiIO& io = ImGui::GetIO();
			io.Fonts->AddFontFromFileTTF(fontPath.c_str(), 16.f, nullptr, io.Fonts->GetGlyphRangesKorean());
		}

		void Release()
		{
			SafeRelease(s_pGraphicsAPI);
		}

		void Run(std::function<bool()> funcUpdate)
		{
			s_pGraphicsAPI->Run(funcUpdate);
		}

		void Update(float elapsedTime)
		{
			s_pGraphicsAPI->Update(elapsedTime);
		}

		void PostUpdate(float elapsedTime)
		{
			s_pGraphicsAPI->PostUpdate(elapsedTime);
		}

		void ScreenShot(ScreenShotFormat format, const std::wstring& path, std::function<void(bool, const std::wstring&)> screenShotCallback)
		{
			s_pGraphicsAPI->ScreenShot(format, path, screenShotCallback);
		}

		APIs GetAPI()
		{
			return s_pGraphicsAPI->GetType();
		}

		HWND GetHwnd()
		{
			return s_pGraphicsAPI->GetHwnd();
		}

		HINSTANCE GetHInstance()
		{
			return s_pGraphicsAPI->GetHInstance();
		}

		const math::uint2& GetScreenSize()
		{
			return s_pGraphicsAPI->GetScreenSize();
		}

		const math::Viewport& GetViewport()
		{
			return s_pGraphicsAPI->GetViewport();
		}

		bool IsFullScreen()
		{
			return s_pGraphicsAPI->IsFullScreen();
		}

		void SetFullScreen(bool isFullScreen, std::function<void(bool)> callback)
		{
			s_pGraphicsAPI->SetFullScreen(isFullScreen, callback);
		}

		const std::vector<DisplayModeDesc>& GetSupportedDisplayModeDesc()
		{
			return s_pGraphicsAPI->GetSupportedDisplayModeDesc();
		}

		size_t GetSelectedDisplayModeIndex()
		{
			return s_pGraphicsAPI->GetSelectedDisplayModeIndex();
		}

		void ChangeDisplayMode(size_t displayModeIndex, std::function<void(bool)> callback)
		{
			s_pGraphicsAPI->ChangeDisplayMode(displayModeIndex, callback);
		}

		IImageBasedLight* GetImageBasedLight()
		{
			return s_pGraphicsAPI->GetImageBasedLight();
		}

		IVTFManager* GetVTFManager()
		{
			return s_pGraphicsAPI->GetVTFManager();
		}

		VertexBufferPtr CreateVertexBuffer(const uint8_t* pData, uint32_t vertexCount, size_t formatSize, bool isDynamic)
		{
			return s_pGraphicsAPI->CreateVertexBuffer(pData, vertexCount, formatSize, isDynamic);
		}

		IndexBufferPtr CreateIndexBuffer(const uint8_t* pData, uint32_t indexCount, size_t formatSize, bool isDynamic)
		{
			return s_pGraphicsAPI->CreateIndexBuffer(pData, indexCount, formatSize, isDynamic);
		}

		TexturePtr CreateTexture(const wchar_t* filePath)
		{
			return s_pGraphicsAPI->CreateTexture(filePath);
		}

		TexturePtr CreateTextureAsync(const wchar_t* filePath)
		{
			return s_pGraphicsAPI->CreateTextureAsync(filePath);
		}

		TexturePtr CreateTexture(const TextureDesc& desc)
		{
			return s_pGraphicsAPI->CreateTexture(desc);
		}

		MaterialPtr CreateMaterial(const IMaterial::Data* pMaterialData)
		{
			return s_pGraphicsAPI->CreateMaterial(pMaterialData);
		}

		MaterialPtr CreateMaterial(const wchar_t* fileName, const wchar_t* filePath)
		{
			return s_pGraphicsAPI->CreateMaterial(fileName, filePath);
		}

		MaterialPtr CloneMaterial(const IMaterial* pMaterial)
		{
			return s_pGraphicsAPI->CloneMaterial(pMaterial);
		}

		DirectionalLightPtr CreateDirectionalLight(const string::StringID& name, bool isEnableShadow, const DirectionalLightData& lightData)
		{
			return s_pGraphicsAPI->CreateDirectionalLight(name, isEnableShadow, lightData);
		}

		PointLightPtr CreatePointLight(const string::StringID& name, bool isEnableShadow, const PointLightData& lightData)
		{
			return s_pGraphicsAPI->CreatePointLight(name, isEnableShadow, lightData);
		}

		SpotLightPtr CreateSpotLight(const string::StringID& name, bool isEnableShadow, const SpotLightData& lightData)
		{
			return s_pGraphicsAPI->CreateSpotLight(name, isEnableShadow, lightData);
		}

		size_t GetLightCount(ILight::Type type)
		{
			return s_pGraphicsAPI->GetLightCount(type);
		}

		LightPtr GetLight(ILight::Type type, size_t index)
		{
			return s_pGraphicsAPI->GetLight(type, index);
		}

		void PushRenderJob(const RenderJobStatic& renderJob)
		{
			if (renderJob.pVertexBuffer == nullptr)
				return;

			s_pGraphicsAPI->PushRenderJob(renderJob);
		}

		void PushRenderJob(const RenderJobSkinned& renderJob)
		{
			if (renderJob.pVertexBuffer == nullptr)
				return;

			s_pGraphicsAPI->PushRenderJob(renderJob);
		}

		void PushRenderJob(const RenderJobTerrain& renderJob)
		{
			if (renderJob.pVertexBuffer == nullptr)
				return;

			s_pGraphicsAPI->PushRenderJob(renderJob);
		}

		void PushRenderJob(const RenderJobVertex& renderJob)
		{
			if (renderJob.pVertexBuffer == nullptr)
				return;

			s_pGraphicsAPI->PushRenderJob(renderJob);
		}

		void OcclusionCullingWriteBMP(const wchar_t* strPath)
		{
			OcclusionCulling::GetInstance()->Write(strPath);
		}
	}

	namespace imguiHelper
	{
		ImTextureID GetTextureID(const graphics::ITexture* pTexture)
		{
			if (pTexture == nullptr)
				return {};

			using namespace graphics;

			switch (GetAPI())
			{
			case APIs::eDX11:
			{
				const dx11::Texture* pTextureDX11 = static_cast<const dx11::Texture*>(pTexture);
				return reinterpret_cast<ImTextureID>(pTextureDX11->GetShaderResourceView());
			}
			break;
			case APIs::eDX12:
			{
				const uint32_t frameIndex = (dx12::Device::GetInstance()->GetFrameIndex() + 1) % dx12::eFrameBufferCount;
				const dx12::Texture* pTextureDX12 = static_cast<const dx12::Texture*>(pTexture);
				return *reinterpret_cast<const ImTextureID*>(&pTextureDX12->GetGPUHandle(frameIndex));
			}
			break;
			case APIs::eVulkan:
				assert(false);
				return {};
			default:
				assert(false);
				return {};
			}
		}
	}
}