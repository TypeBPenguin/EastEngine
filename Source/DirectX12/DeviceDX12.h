#pragma once

#include "CommonLib/Singleton.h"

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

namespace eastengine
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
				void Initialize(uint32_t nWidth, uint32_t nHeight, bool isFullScreen, const string::StringID& strApplicationTitle, const string::StringID& strApplicationName);

				void Run(std::function<void()> funcUpdate);

				void Cleanup(float fElapsedTime);

			public:
				RenderTarget* GetRenderTarget(const D3D12_RESOURCE_DESC* pDesc, const math::Color& clearColor, bool isIncludeLastUseRenderTarget = true);
				void ReleaseRenderTargets(RenderTarget** ppRenderTarget, uint32_t nSize = 1, bool isSetLastRenderTarget = true);

				void ReleaseResource(ID3D12DeviceChild* pResource);
				void ReleaseResourceRTV(uint32_t nDescriptorIndex);
				void ReleaseResourceSRV(uint32_t nDescriptorIndex);
				void ReleaseResourceDSV(uint32_t nDescriptorIndex);
				void ReleaseResourceUAV(uint32_t nDescriptorIndex);

			public:
				HWND GetHwnd() const;
				HINSTANCE GetHInstance() const;
				void AddMessageHandler(const string::StringID& strName, std::function<void(HWND, uint32_t, WPARAM, LPARAM)> funcHandler);
				void RemoveMessageHandler(const string::StringID& strName);

				const math::uint2& GetScreenSize() const;
				const D3D12_VIEWPORT* GetViewport() const;
				const math::Rect* GetScissorRect() const;

				ID3D12Device* GetInterface() const;
				ID3D12CommandQueue* GetCommandQueue() const;

				ID3D12Fence* GetFence() const;
				uint64_t GetFenceValue() const;

				uint32_t GetFrameIndex() const;

				RenderTarget* GetSwapChainRenderTarget(uint32_t nFrameIndex) const;
				RenderTarget* GetLastUsedRenderTarget() const;

				GBuffer* GetGBuffer(uint32_t nFrameIndex) const;
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

				void ResetCommandList(size_t nIndex, ID3D12PipelineState* pPipelineState);
				ID3D12GraphicsCommandList2* GetCommandList(size_t nIndex) const;
				void GetCommandLists(ID3D12GraphicsCommandList2** ppCommandLists_out) const;
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