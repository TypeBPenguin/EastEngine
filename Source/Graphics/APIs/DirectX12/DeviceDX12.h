#pragma once

#include "CommonLib/Singleton.h"
#include "Graphics/Interface/GraphicsInterface.h"

struct D3D12_VIEWPORT;

struct ID3D12Device;
struct ID3D12CommandQueue;
struct ID3D12CommandList;
struct ID3D12GraphicsCommandList2;
struct ID3D12PipelineState;
struct ID3D12DeviceChild;

struct ID3D12Fence;
struct ID3D12Resource;
struct ID3D12DescriptorHeap;

struct D3D12_RESOURCE_DESC;
struct D3D12_DESCRIPTOR_RANGE;

namespace est
{
	namespace graphics
	{
		class IImageBasedLight;

		namespace dx12
		{
			class RenderManager;
			class RenderTarget;
			class GBuffer;
			class Uploader;
			class DescriptorHeap;
			class VTFManager;

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
				RenderTarget* GetRenderTarget(const D3D12_RESOURCE_DESC* pDesc, const math::Color& clearColor);
				void ReleaseRenderTargets(RenderTarget** ppRenderTarget, uint32_t nSize = 1);

				void ReleaseResource(ID3D12DeviceChild* pResource);
				void ReleaseResourceRTV(uint32_t descriptorIndex);
				void ReleaseResourceSRV(uint32_t descriptorIndex);
				void ReleaseResourceDSV(uint32_t descriptorIndex);
				void ReleaseResourceUAV(uint32_t descriptorIndex);

			public:
				HWND GetHwnd() const;
				HINSTANCE GetHInstance() const;

				const math::uint2& GetScreenSize() const;
				const math::Viewport& GetViewport() const;
				const math::Rect& GetScissorRect() const;

				bool IsFullScreen() const;
				void SetFullScreen(bool isFullScreen, std::function<void(bool)> callback);

				const std::vector<DisplayModeDesc>& GetSupportedDisplayModeDesc() const;
				size_t GetSelectedDisplayModeIndex() const;
				void ChangeDisplayMode(size_t displayModeIndex, std::function<void(bool)> callback);

				ID3D12Device* GetInterface() const;
				ID3D12CommandQueue* GetCommandQueue() const;

				ID3D12Fence* GetFence() const;
				uint64_t GetFenceValue() const;

				uint32_t GetFrameIndex() const;

				RenderTarget* GetSwapChainRenderTarget(uint32_t frameIndex) const;
				RenderTarget* GetBackBufferSwapChainRenderTarget(uint32_t frameIndex) const;

				GBuffer* GetGBuffer(uint32_t frameIndex) const;
				IImageBasedLight* GetImageBasedLight() const;
				void SetImageBasedLight(IImageBasedLight* pImageBasedLight);

				RenderManager* GetRenderManager() const;
				VTFManager* GetVTFManager() const;

				Uploader* GetUploader() const;

				DescriptorHeap* GetRTVDescriptorHeap() const;
				DescriptorHeap* GetSRVDescriptorHeap() const;
				DescriptorHeap* GetDSVDescriptorHeap() const;
				DescriptorHeap* GetUAVDescriptorHeap() const;
				DescriptorHeap* GetSamplerDescriptorHeap() const;

			public:
				size_t GetCommandListCount() const;

				void ResetCommandList(size_t index, ID3D12PipelineState* pPipelineState);
				ID3D12GraphicsCommandList2* GetCommandList(size_t index) const;

				void GetCommandLists(std::vector<ID3D12GraphicsCommandList2*>& commandList_out) const;
				void ExecuteCommandList(ID3D12CommandList* pCommandList);
				void ExecuteCommandLists(ID3D12CommandList* const* ppCommandList, size_t nCount);
				void ExecuteCommandLists(ID3D12GraphicsCommandList2* const* ppCommandList, size_t nCount);

				ID3D12GraphicsCommandList2* CreateBundle(ID3D12PipelineState* pPipelineState);

				const D3D12_DESCRIPTOR_RANGE* GetStandardDescriptorRanges() const;

			private:
				class Impl;
				std::unique_ptr<Impl> m_pImpl;
			};
		}
	}
}