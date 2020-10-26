#pragma once

#include "Graphics/Interface/GraphicsInterface.h"
#include "Graphics/Interface/RenderJob.h"

namespace est
{
	namespace graphics
	{
		void Initialize(APIs emAPI, uint32_t width, uint32_t height, bool isFullScreen, bool isVSync, const string::StringID& applicationTitle, const string::StringID& applicationName);
		void Release();
		void Run(std::function<bool()> funcUpdate);
		void Cleanup(float elapsedTime);
		void Update(float elapsedTime);
		void PostUpdate(float elapsedTime);

		APIs GetAPI();
		HWND GetHwnd();
		HINSTANCE GetHInstance();
		void AddMessageHandler(const string::StringID& name, std::function<void(HWND, uint32_t, WPARAM, LPARAM)> funcHandler);
		void RemoveMessageHandler(const string::StringID& name);

		const math::uint2& GetScreenSize();

		Camera* GetCamera();
		IImageBasedLight* GetImageBasedLight();
		IVTFManager* GetVTFManager();

		VertexBufferPtr CreateVertexBuffer(const uint8_t* pData, uint32_t vertexCount, size_t formatSize, bool isDynamic);
		IndexBufferPtr CreateIndexBuffer(const uint8_t* pData, uint32_t indexCount, size_t formatSize, bool isDynamic);
		TexturePtr CreateTexture(const wchar_t* filePath);
		TexturePtr CreateTextureAsync(const wchar_t* filePath);
		TexturePtr CreateTexture(const TextureDesc& desc);

		MaterialPtr CreateMaterial(const IMaterial::Data* pMaterialData);
		MaterialPtr CreateMaterial(const wchar_t* fileName, const wchar_t* filePath);
		MaterialPtr CloneMaterial(const IMaterial* pMaterial);

		DirectionalLightPtr CreateDirectionalLight(const string::StringID& name, bool isEnableShadow, const DirectionalLightData& lightData);
		PointLightPtr CreatePointLight(const string::StringID& name, bool isEnableShadow, const PointLightData& lightData);
		SpotLightPtr CreateSpotLight(const string::StringID& name, bool isEnableShadow, const SpotLightData& lightData);
		size_t GetLightCount(ILight::Type type);
		LightPtr GetLight(ILight::Type type, size_t index);

		template <typename T>
		void ReleaseResource(std::shared_ptr<T>& ppResource);

		void PushRenderJob(const RenderJobStatic& renderJob);
		void PushRenderJob(const RenderJobSkinned& renderJob);
		void PushRenderJob(const RenderJobTerrain& renderJob);
		void PushRenderJob(const RenderJobVertex& renderJob);

		void OcclusionCullingWriteBMP(const wchar_t* path);
	}
}