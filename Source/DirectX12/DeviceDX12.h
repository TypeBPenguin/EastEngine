#pragma once

#include "CommonLib/Singleton.h"

struct D3D12_VIEWPORT;

struct ID3D12Device;
struct ID3D12CommandAllocator;
struct ID3D12CommandQueue;
struct ID3D12GraphicsCommandList2;
struct ID3D12PipelineState;

struct ID3D12Fence;
struct ID3D12Resource;
struct ID3D12DescriptorHeap;

struct D3D12_RESOURCE_DESC;
struct D3D12_DESCRIPTOR_RANGE;

namespace eastengine
{
	namespace graphics
	{
		class ImageBasedLight;

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
				void Initialize(uint32_t nWidth, uint32_t nHeight, bool isFullScreen, const String::StringID& strApplicationTitle, const String::StringID& strApplicationName);

				void Run(std::function<void()> funcUpdate);

				void Flush(float fElapsedTime);

			public:
				RenderTarget* GetRenderTarget(const D3D12_RESOURCE_DESC* pDesc, const math::Color& clearColor, bool isIncludeLastUseRenderTarget = true);
				void ReleaseRenderTarget(RenderTarget** ppRenderTarget, uint32_t nSize = 1, bool isSetLastRenderTarget = true);

			public:
				HWND GetHwnd() const;
				HINSTANCE GetHInstance() const;
				void AddMessageHandler(std::function<void(HWND, uint32_t, WPARAM, LPARAM)> funcHandler);

				const math::UInt2& GetScreenSize() const;
				const D3D12_VIEWPORT* GetViewport() const;
				const math::Rect* GetScissorRect() const;

				ID3D12Device* GetInterface() const;
				ID3D12CommandAllocator* GetCommandAllocator() const;
				ID3D12CommandQueue* GetCommandQueue() const;

				ID3D12Fence* GetFence() const;
				uint64_t GetFenceValue() const;

				uint32_t GetFrameIndex() const;

				RenderTarget* GetSwapChainRenderTarget(int nFrameIndex) const;
				RenderTarget* GetLastUseRenderTarget() const;

				GBuffer* GetGBuffer(int nFrameIndex) const;
				ImageBasedLight* GetImageBasedLight() const;
				RenderManager* GetRenderManager() const;
				VTFManager* GetVTFManager() const;

				Uploader* GetUploader() const;

				DescriptorHeap* GetRTVDescriptorHeap() const;
				DescriptorHeap* GetSRVDescriptorHeap() const;
				DescriptorHeap* GetDSVDescriptorHeap() const;
				DescriptorHeap* GetUAVDescriptorHeap() const;
				DescriptorHeap* GetSamplerDescriptorHeap() const;

			public:
				ID3D12GraphicsCommandList2* PopCommandList(ID3D12PipelineState* pPipeliseState);
				void PushCommandList(ID3D12GraphicsCommandList2* pCommandList);

				const D3D12_DESCRIPTOR_RANGE* GetStandardDescriptorRanges() const;

			private:
				class Impl;
				std::unique_ptr<Impl> m_pImpl;
			};
		}
	}
}