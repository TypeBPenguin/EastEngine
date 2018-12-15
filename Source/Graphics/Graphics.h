#pragma once

#include "GraphicsInterface/GraphicsInterface.h"
#include "GraphicsInterface/RenderJob.h"

namespace eastengine
{
	namespace graphics
	{
		void Initialize(APIs emAPI, uint32_t nWidth, uint32_t nHeight, bool isFullScreen, const string::StringID& strApplicationTitle, const string::StringID& strApplicationName);
		void Release();
		void Run(std::function<void()> funcUpdate);
		void Cleanup(float elapsedTime);
		void Update(float elapsedTime);
		void PostUpdate();

		APIs GetAPI();
		HWND GetHwnd();
		HINSTANCE GetHInstance();
		void AddMessageHandler(const string::StringID& name, std::function<void(HWND, uint32_t, WPARAM, LPARAM)> funcHandler);
		void RemoveMessageHandler(const string::StringID& name);

		const math::uint2& GetScreenSize();

		IImageBasedLight* GetImageBasedLight();
		IVTFManager* GetVTFManager();

		IVertexBuffer* CreateVertexBuffer(const uint8_t* pData, uint32_t vertexCount, size_t formatSize, bool isDynamic);
		IIndexBuffer* CreateIndexBuffer(const uint8_t* pData, uint32_t indexCount, size_t formatSize, bool isDynamic);
		ITexture* CreateTexture(const char* strFilePath);
		ITexture* CreateTextureAsync(const char* strFilePath);
		ITexture* CreateTexture(const TextureDesc& desc);

		IMaterial* CreateMaterial(const MaterialInfo* pInfo);
		IMaterial* CreateMaterial(const char* strFileName, const char* strFilePath);
		IMaterial* CloneMaterial(const IMaterial* pMaterial);

		IDirectionalLight* CreateDirectionalLight(const string::StringID& name, bool isEnableShadow, const DirectionalLightData& lightData);
		IPointLight* CreatePointLight(const string::StringID& name, bool isEnableShadow, const PointLightData& lightData);
		ISpotLight* CreateSpotLight(const string::StringID& name, bool isEnableShadow, const SpotLightData& lightData);

		template <typename T>
		void ReleaseResource(T** ppResource);

		void PushRenderJob(const RenderJobStatic& renderJob);
		void PushRenderJob(const RenderJobSkinned& renderJob);
		void PushRenderJob(const RenderJobTerrain& renderJob);
		void PushRenderJob(const RenderJobVertex& renderJob);

		void OcclusionCullingWriteBMP(const char* strPath);
	}
}