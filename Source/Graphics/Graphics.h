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
		void Cleanup(float fElapsedTime);
		void Update(float fElapsedTime);

		APIs GetAPI();
		HWND GetHwnd();
		HINSTANCE GetHInstance();
		void AddMessageHandler(const string::StringID& strName, std::function<void(HWND, uint32_t, WPARAM, LPARAM)> funcHandler);
		void RemoveMessageHandler(const string::StringID& strName);

		const math::UInt2& GetScreenSize();

		IImageBasedLight* GetImageBasedLight();
		IVTFManager* GetVTFManager();

		IVertexBuffer* CreateVertexBuffer(const uint8_t* pData, size_t nBufferSize, uint32_t nVertexCount);
		IIndexBuffer* CreateIndexBuffer(const uint8_t* pData, size_t nBufferSize, uint32_t nIndexCount);
		ITexture* CreateTexture(const char* strFilePath);
		ITexture* CreateTextureAsync(const char* strFilePath);
		ITexture* CreateTexture(const TextureDesc& desc);

		IMaterial* CreateMaterial(const MaterialInfo* pInfo);
		IMaterial* CreateMaterial(const char* strFileName, const char* strFilePath);
		IMaterial* CloneMaterial(const IMaterial* pMaterial);

		template <typename T>
		void ReleaseResource(T** ppResource);

		void PushRenderJob(const RenderJobStatic& renderJob);
		void PushRenderJob(const RenderJobSkinned& renderJob);
		void PushRenderJob(const RenderJobTerrain& renderJob);
	}
}