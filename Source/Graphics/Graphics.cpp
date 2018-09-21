#include "stdafx.h"
#include "Graphics.h"

#include "CommonLib/Lock.h"
#include "CommonLib/FileUtil.h"

#include "GraphicsInterface/Camera.h"
#include "GraphicsInterface/LightManager.h"
#include "GraphicsInterface/imguiHelper.h"

#include "DirectX12/DeviceDX12.h"
#include "DirectX12/VertexBufferDX12.h"
#include "DirectX12/IndexBufferDX12.h"
#include "DirectX12/TextureDX12.h"
#include "DirectX12/RenderManagerDX12.h"
#include "DirectX12/VTFManagerDX12.h"

#include "DirectX11/DeviceDX11.h"
#include "DirectX11/VertexBufferDX11.h"
#include "DirectX11/IndexBufferDX11.h"
#include "DirectX11/TextureDX11.h"
#include "DirectX11/RenderManagerDX11.h"
#include "DirectX11/VTFManagerDX11.h"

#include "Vulkan/DeviceVulkan.h"
#include "Vulkan/VertexBufferVulkan.h"
#include "Vulkan/IndexBufferVulkan.h"
#include "Vulkan/TextureVulkan.h"
#include "Vulkan/RenderManagerVulkan.h"

#include "TextureManager.h"
#include "Material.h"
#include "ImageBasedLight.h"

namespace eastengine
{
	namespace graphics
	{
		class IGraphicsAPI
		{
		public:
			IGraphicsAPI() = default;
			virtual ~IGraphicsAPI() = default;

		public:
			virtual void Initialize(uint32_t nWidth, uint32_t nHeight, bool isFullScreen, const String::StringID& strApplicationTitle, const String::StringID& strApplicationName) = 0;
			virtual void Release() = 0;
			virtual void Run(std::function<void()> funcUpdate) = 0;
			virtual void Flush(float fElapsedTime) = 0;

		public:
			virtual APIs GetType() const = 0;
			virtual HWND GetHwnd() const = 0;
			virtual HINSTANCE GetHInstance() const = 0;
			virtual void AddMessageHandler(const String::StringID& strName, std::function<void(HWND, uint32_t, WPARAM, LPARAM)> funcHandler) = 0;
			virtual void RemoveMessageHandler(const String::StringID& strName) = 0;

			virtual const math::UInt2& GetScreenSize() const = 0;
			virtual IImageBasedLight* GetImageBasedLight() const = 0;
			virtual IVTFManager* GetVTFManager() const = 0;

			virtual IVertexBuffer* CreateVertexBuffer(const uint8_t* pData, size_t nBufferSize, uint32_t nVertexCount) = 0;
			virtual IIndexBuffer* CreateIndexBuffer(const uint8_t* pData, size_t nBufferSize, uint32_t nVertexCount) = 0;
			virtual ITexture* CreateTexture(const char* strFilePath) = 0;
			virtual ITexture* CreateTextureAsync(const char* strFilePath) = 0;
			virtual IMaterial* CreateMaterial(const MaterialInfo* pInfo) = 0;
			virtual IMaterial* CreateMaterial(const char* strFileName, const char* strFilePath) = 0;
			virtual IMaterial* CloneMaterial(const IMaterial* pMaterialSource) = 0;

			virtual void PushRenderJob(const RenderJobStatic& renderJob) = 0;
			virtual void PushRenderJob(const RenderJobSkinned& renderJob) = 0;

			virtual void ReleaseResource(IResource* pResource) = 0;
			virtual void FlushGarbageCollection() = 0;
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
			virtual void Initialize(uint32_t nWidth, uint32_t nHeight, bool isFullScreen, const String::StringID& strApplicationTitle, const String::StringID& strApplicationName) override
			{
				Device::GetInstance()->Initialize(nWidth, nHeight, isFullScreen, strApplicationTitle, strApplicationName);
				m_pTextureManager = std::make_unique<TextureManager>();

				m_pImageBasedLight = std::make_unique<ImageBasedLight>();
				Device::GetInstance()->SetImageBasedLight(m_pImageBasedLight.get());
			}

			virtual void Release() override
			{
				m_pImageBasedLight.reset();

				FlushGarbageCollection();

				if (m_umapVertexBuffers.empty() == false ||
					m_umapIndexBuffers.empty() == false ||
					m_umapMaterials.empty() == false)
				{
					std::string error = String::Format("please release all resource : vertexbuffer[%lld], indexbuffer[%lld], material[%lld]", m_umapVertexBuffers.size(), m_umapIndexBuffers.size(), m_umapMaterials.size());
					OutputDebugString(error.c_str());
					LOG_ERROR("%s", error.c_str());
					Sleep(5000);
				}
				m_umapVertexBuffers.clear();
				m_umapIndexBuffers.clear();
				m_umapMaterials.clear();

				m_pTextureManager.reset();
				Device::DestroyInstance();
			}

			virtual void Run(std::function<void()> funcUpdate) override
			{
				Device::GetInstance()->Run(funcUpdate);
			}

			virtual void Flush(float fElapsedTime) override
			{
				FlushGarbageCollection();
				m_pTextureManager->Flush(fElapsedTime);
				Device::GetInstance()->Flush(fElapsedTime);
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

			virtual void AddMessageHandler(const String::StringID& strName, std::function<void(HWND, uint32_t, WPARAM, LPARAM)> funcHandler) override
			{
				if constexpr (APIType == APIs::eDX11 || APIType == APIs::eDX12)
				{
					Device::GetInstance()->AddMessageHandler(strName, funcHandler);
				}
			}

			virtual void RemoveMessageHandler(const String::StringID& strName) override
			{
				if constexpr (APIType == APIs::eDX11 || APIType == APIs::eDX12)
				{
					Device::GetInstance()->RemoveMessageHandler(strName);
				}
			}

			virtual const math::UInt2& GetScreenSize() const override
			{
				return Device::GetInstance()->GetScreenSize();
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

			virtual IVertexBuffer* CreateVertexBuffer(const uint8_t* pData, size_t nBufferSize, uint32_t nVertexCount) override
			{
				thread::AutoLock autoLock(&m_lock);

				std::unique_ptr<VertexBuffer> pVertexBuffer = std::make_unique<VertexBuffer>(pData, nBufferSize, nVertexCount);
				auto iter = m_umapVertexBuffers.emplace(pVertexBuffer.get(), std::move(pVertexBuffer));
				if (iter.second == false)
				{
					throw_line("failed to vertex buffer emplace to unordered map");
				}

				iter.first->second->IncreaseReference();

				return iter.first->second.get();
			}

			virtual IIndexBuffer* CreateIndexBuffer(const uint8_t* pData, size_t nBufferSize, uint32_t nVertexCount) override
			{
				thread::AutoLock autoLock(&m_lock);

				std::unique_ptr<IndexBuffer> pIndexBuffer = std::make_unique<IndexBuffer>(pData, nBufferSize, nVertexCount);
				auto iter = m_umapIndexBuffers.emplace(pIndexBuffer.get(), std::move(pIndexBuffer));
				if (iter.second == false)
				{
					throw_line("failed to index buffer emplace to unordered map");
				}

				iter.first->second->IncreaseReference();

				return iter.first->second.get();
			}

			virtual ITexture* CreateTexture(const char* strFilePath) override
			{
				ITexture::Key key{ String::StringID(strFilePath) };
				ITexture* pITexture = m_pTextureManager->GetTexture(key);
				if (pITexture != nullptr)
					return pITexture;

				Texture* pTexture = new Texture(key);
				pTexture->Load(strFilePath);
				pTexture->IncreaseReference();

				m_pTextureManager->PushTexture(pTexture);

				return pTexture;
			}

			virtual ITexture* CreateTextureAsync(const char* strFilePath) override
			{
				ITexture::Key key{ String::StringID(strFilePath) };
				ITexture* pITexture = m_pTextureManager->GetTexture(key);
				if (pITexture != nullptr)
					return pITexture;

				Texture* pTexture = new Texture(key);
				pTexture->IncreaseReference();

				m_pTextureManager->AsyncLoadTexture(pTexture, strFilePath, [pTexture](const std::string& strPath)
				{
					return pTexture->Load(strPath.c_str());
				});

				return pTexture;
			}

			virtual IMaterial* CreateMaterial(const MaterialInfo* pInfo) override
			{
				std::unique_ptr<Material> pMaterial = Material::Create(pInfo);
				pMaterial->IncreaseReference();

				thread::AutoLock autoLock(&m_lock);

				auto iter = m_umapMaterials.emplace(pMaterial.get(), std::move(pMaterial));
				if (iter.second == false)
				{
					throw_line("failed to material emplace to unordered map");
				}

				return iter.first->second.get();
			}

			virtual IMaterial* CreateMaterial(const char* strFileName, const char* strFilePath) override
			{
				std::unique_ptr<Material> pMaterial = Material::Create(strFileName, strFilePath);
				pMaterial->IncreaseReference();

				thread::AutoLock autoLock(&m_lock);

				auto iter = m_umapMaterials.emplace(pMaterial.get(), std::move(pMaterial));
				if (iter.second == false)
				{
					throw_line("failed to material emplace to unordered map");
				}

				return iter.first->second.get();
			}

			virtual IMaterial* CloneMaterial(const IMaterial* pMaterialSource) override
			{
				if (pMaterialSource == nullptr)
					return nullptr;

				std::unique_ptr<Material> pMaterial = Material::Clone(static_cast<const Material*>(pMaterialSource));
				pMaterial->IncreaseReference();

				thread::AutoLock autoLock(&m_lock);

				auto iter = m_umapMaterials.emplace(pMaterial.get(), std::move(pMaterial));
				if (iter.second == false)
				{
					throw_line("failed to material emplace to unordered map");
				}

				return iter.first->second.get();
			}

			virtual void PushRenderJob(const RenderJobStatic& renderJob) override
			{
				RenderManager* pRenderManager = Device::GetInstance()->GetRenderManager();
				pRenderManager->PushJob(renderJob);
			}

			virtual void PushRenderJob(const RenderJobSkinned& renderJob) override
			{
				RenderManager* pRenderManager = Device::GetInstance()->GetRenderManager();
				pRenderManager->PushJob(renderJob);
			}

			virtual void ReleaseResource(IResource* pResource) override
			{
				if (pResource->GetResourceType() == StrID::Texture)
				{
					pResource->DecreaseReference();
				}
				else
				{
					if (pResource->DecreaseReference() <= 0)
					{
						thread::AutoLock autoLock(&m_lock);
						m_vecGarbages.emplace_back(pResource);
					}
				}
			}

			virtual void FlushGarbageCollection() override
			{
				thread::AutoLock autoLock(&m_lock);

				std::sort(m_vecGarbages.begin(), m_vecGarbages.end());
				m_vecGarbages.erase(std::unique(m_vecGarbages.begin(), m_vecGarbages.end()), m_vecGarbages.end());

				const size_t nSize = m_vecGarbages.size();
				for (size_t i = 0; i < nSize; ++i)
				{
					if (m_vecGarbages[i]->GetReferenceCount() > 0)
						continue;

					const String::StringID& strResourceType = m_vecGarbages[i]->GetResourceType();
					if (strResourceType == StrID::VertexBuffer)
					{
						m_umapVertexBuffers.erase(m_vecGarbages[i]);
					}
					else if (strResourceType == StrID::IndexBuffer)
					{
						m_umapIndexBuffers.erase(m_vecGarbages[i]);
					}
					else if (strResourceType == StrID::Material)
					{
						m_umapMaterials.erase(m_vecGarbages[i]);
					}
				}
				m_vecGarbages.clear();
			}

		protected:
			thread::Lock m_lock;
			std::vector<IResource*> m_vecGarbages;

			std::unique_ptr<TextureManager> m_pTextureManager;
			std::unique_ptr<ImageBasedLight> m_pImageBasedLight;

			std::unordered_map<IResource*, std::unique_ptr<VertexBuffer>> m_umapVertexBuffers;
			std::unordered_map<IResource*, std::unique_ptr<IndexBuffer>> m_umapIndexBuffers;
			std::unordered_map<IResource*, std::unique_ptr<Material>> m_umapMaterials;
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

		void Initialize(APIs emAPI, uint32_t nWidth, uint32_t nHeight, bool isFullScreen, const String::StringID& strApplicationTitle, const String::StringID& strApplicationName)
		{
			if (s_pGraphicsAPI != nullptr)
			{
				throw_line("unsupported change graphics api");
			}

			switch (emAPI)
			{
			case eDX11:
				s_pGraphicsAPI = std::make_unique<dx11::GraphicsAPI>();
				LOG_MESSAGE("Graphics API : DirectX11");
				break;
			case eDX12:
				s_pGraphicsAPI = std::make_unique<dx12::GraphicsAPI>();
				LOG_MESSAGE("Graphics API : DirectX12");
				break;
			case eVulkan:
				s_pGraphicsAPI = std::make_unique<vulkan::GraphicsAPI>();
				LOG_MESSAGE("Graphics API : Vulkan");
				break;
			default:
				throw_line("unknown request");
				break;
			}

			s_pGraphicsAPI->Initialize(nWidth, nHeight, isFullScreen, strApplicationTitle, strApplicationName);

			std::string strFontPath = file::GetPath(file::eFont);
			strFontPath.append("ArialUni.ttf");

			ImGuiIO& io = ImGui::GetIO();
			io.Fonts->AddFontFromFileTTF(strFontPath.c_str(), 16.f, nullptr, io.Fonts->GetGlyphRangesKorean());
		}

		void Release()
		{
			Camera::DestroyInstance();
			LightManager::DestroyInstance();
			SafeRelease(s_pGraphicsAPI);
		}

		void Run(std::function<void()> funcUpdate)
		{
			s_pGraphicsAPI->Run(funcUpdate);
		}

		void Flush(float fElapsedTime)
		{
			s_pGraphicsAPI->Flush(fElapsedTime);
		}

		void Update(float fElapsedTime)
		{
			Camera::GetInstance()->Update(fElapsedTime);
			LightManager::GetInstance()->Update(fElapsedTime);
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

		void AddMessageHandler(const String::StringID& strName, std::function<void(HWND, uint32_t, WPARAM, LPARAM)> funcHandler)
		{
			s_pGraphicsAPI->AddMessageHandler(strName, funcHandler);
		}

		void RemoveMessageHandler(const String::StringID& strName)
		{
			s_pGraphicsAPI->RemoveMessageHandler(strName);
		}

		const math::UInt2& GetScreenSize()
		{
			return s_pGraphicsAPI->GetScreenSize();
		}

		IImageBasedLight* GetImageBasedLight()
		{
			return s_pGraphicsAPI->GetImageBasedLight();
		}

		IVTFManager* GetVTFManager()
		{
			return s_pGraphicsAPI->GetVTFManager();
		}

		IVertexBuffer* CreateVertexBuffer(const uint8_t* pData, size_t nBufferSize, uint32_t nVertexCount)
		{
			return s_pGraphicsAPI->CreateVertexBuffer(pData, nBufferSize, nVertexCount);
		}

		IIndexBuffer* CreateIndexBuffer(const uint8_t* pData, size_t nBufferSize, uint32_t nIndexCount)
		{
			return s_pGraphicsAPI->CreateIndexBuffer(pData, nBufferSize, nIndexCount);
		}

		ITexture* CreateTexture(const char* strFilePath)
		{
			return s_pGraphicsAPI->CreateTexture(strFilePath);
		}

		ITexture* CreateTextureAsync(const char* strFilePath)
		{
			return s_pGraphicsAPI->CreateTextureAsync(strFilePath);
		}

		IMaterial* CreateMaterial(const MaterialInfo* pInfo)
		{
			return s_pGraphicsAPI->CreateMaterial(pInfo);
		}

		IMaterial* CreateMaterial(const char* strFileName, const char* strFilePath)
		{
			return s_pGraphicsAPI->CreateMaterial(strFileName, strFilePath);
		}

		IMaterial* CloneMaterial(const IMaterial* pMaterial)
		{
			return s_pGraphicsAPI->CloneMaterial(pMaterial);
		}

		template <>
		void ReleaseResource(IVertexBuffer** ppResource)
		{
			if (ppResource == nullptr || *ppResource == nullptr)
				return;

			s_pGraphicsAPI->ReleaseResource(*ppResource);

			*ppResource = nullptr;
		}

		template <>
		void ReleaseResource(IIndexBuffer** ppResource)
		{
			if (ppResource == nullptr || *ppResource == nullptr)
				return;

			s_pGraphicsAPI->ReleaseResource(*ppResource);

			*ppResource = nullptr;
		}

		template <>
		void ReleaseResource(ITexture** ppResource)
		{
			if (ppResource == nullptr || *ppResource == nullptr)
				return;

			s_pGraphicsAPI->ReleaseResource(*ppResource);

			*ppResource = nullptr;
		}

		template <>
		void ReleaseResource(IMaterial** ppResource)
		{
			if (ppResource == nullptr || *ppResource == nullptr)
				return;

			s_pGraphicsAPI->ReleaseResource(*ppResource);

			*ppResource = nullptr;
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
				return *(ImTextureID*)pTextureDX11->GetShaderResourceView();
			}
			break;
			case APIs::eDX12:
			{
				const dx12::Texture* pTextureDX12 = static_cast<const dx12::Texture*>(pTexture);
				return *(ImTextureID*)&pTextureDX12->GetGPUHandle(0);
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