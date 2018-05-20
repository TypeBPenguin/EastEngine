#include "stdafx.h"
#include "RenderTargetDX12.h"

#include "DeviceDX12.h"
#include "DescriptorHeapDX12.h"

namespace eastengine
{
	namespace graphics
	{
		namespace dx12
		{
			static std::atomic<uint32_t> s_nRenderTargetIndex = 0;

			RenderTarget::RenderTarget(const Key& key)
				: m_key(key)
			{
			}

			RenderTarget::~RenderTarget()
			{
				DescriptorHeap* pDescriptorHeap = Device::GetInstance()->GetRTVDescriptorHeap();
				pDescriptorHeap->FreePersistent(m_nDescriptorIndex);
			}

			std::unique_ptr<RenderTarget> RenderTarget::Create(ID3D12Resource* pResource, const math::Color& clearColor)
			{
				ID3D12Device* pDevice = Device::GetInstance()->GetInterface();
				DescriptorHeap* pDescriptorHeap = Device::GetInstance()->GetRTVDescriptorHeap();

				D3D12_RESOURCE_DESC desc = pResource->GetDesc();

				Key key = RenderTarget::BuildKey(&desc, clearColor);
				std::unique_ptr<RenderTarget> pRenderTarget = std::make_unique<RenderTarget>(key);

				PersistentDescriptorAlloc rtvAlloc = pDescriptorHeap->AllocatePersistent();
				pRenderTarget->m_nDescriptorIndex = rtvAlloc.nIndex;

				pDevice->CreateRenderTargetView(pResource, nullptr, rtvAlloc.cpuHandles[0]);

				String::StringID strKey;
				strKey.Format("SwapChainRenderTarget_%d", s_nRenderTargetIndex++);

				pRenderTarget->m_pTexture = std::make_unique<Texture>(Texture::Key{ strKey.Key() });
				if (pRenderTarget->m_pTexture->Bind(pResource, nullptr) == false)
				{
					throw_line("failed to create render target texture");
				}
				pResource->SetName(String::MultiToWide(strKey.c_str()).c_str());

				return pRenderTarget;
			}

			std::unique_ptr<RenderTarget> RenderTarget::Create(const wchar_t* wstrDebugName, const D3D12_RESOURCE_DESC* pResourceDesc, const math::Color& clearColor, D3D12_RESOURCE_STATES resourceState, uint32_t nMipSlice, uint32_t nFirstArraySlice, uint32_t nArraySize)
			{
				ID3D12Device* pDevice = Device::GetInstance()->GetInterface();
				DescriptorHeap* pRTVDescriptorHeap = Device::GetInstance()->GetRTVDescriptorHeap();

				Key key = RenderTarget::BuildKey(pResourceDesc, clearColor);
				std::unique_ptr<RenderTarget> pRenderTarget = std::make_unique<RenderTarget>(key);

				D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
				rtvDesc.Format = pResourceDesc->Format;
				rtvDesc.ViewDimension = (pResourceDesc->DepthOrArraySize == 1) ? D3D12_RTV_DIMENSION_TEXTURE2D : D3D12_RTV_DIMENSION_TEXTURE2DARRAY;

				switch (rtvDesc.ViewDimension)
				{
				case D3D12_RTV_DIMENSION_TEXTURE2D:
					rtvDesc.Texture2D.MipSlice = nMipSlice;
					break;
				case D3D12_RTV_DIMENSION_TEXTURE2DARRAY:
					rtvDesc.Texture2DArray.MipSlice = nMipSlice;
					rtvDesc.Texture2DArray.FirstArraySlice = nFirstArraySlice;
					rtvDesc.Texture2DArray.ArraySize = nArraySize;
					break;
				case D3D12_RTV_DIMENSION_TEXTURE2DMS:
					break;
				case D3D12_RTV_DIMENSION_TEXTURE2DMSARRAY:
					rtvDesc.Texture2DMSArray.FirstArraySlice = nFirstArraySlice;
					rtvDesc.Texture2DMSArray.ArraySize = nArraySize;
					break;
				default: break;
				}

				D3D12_CLEAR_VALUE clearValue{};
				clearValue.Format = pResourceDesc->Format;
				clearValue.Color[0] = clearColor.r;
				clearValue.Color[1] = clearColor.g;
				clearValue.Color[2] = clearColor.b;
				clearValue.Color[3] = clearColor.a;

				PersistentDescriptorAlloc rtvAlloc = pRTVDescriptorHeap->AllocatePersistent();
				pRenderTarget->m_nDescriptorIndex = rtvAlloc.nIndex;

				ID3D12Resource* pResource = nullptr;
				CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_DEFAULT);
				HRESULT hr = pDevice->CreateCommittedResource(&heapProperties,
					D3D12_HEAP_FLAG_NONE,
					pResourceDesc,
					resourceState,
					&clearValue,
					IID_PPV_ARGS(&pResource));
				if (FAILED(hr))
				{
					throw_line("failed to create RenderTarget");
				}
				pResource->SetName(wstrDebugName);

				pDevice->CreateRenderTargetView(pResource, &rtvDesc, rtvAlloc.cpuHandles[0]);

				String::StringID strKey;
				strKey.Format("RenderTarget_%d", s_nRenderTargetIndex++);

				pRenderTarget->m_pTexture = std::make_unique<Texture>(Texture::Key{ strKey.Key() });
				if (pRenderTarget->m_pTexture->Bind(pResource, nullptr) == false)
				{
					throw_line("failed to create render target texture");
				}

				return pRenderTarget;
			}

			RenderTarget::Key RenderTarget::BuildKey(const D3D12_RESOURCE_DESC* pDesc, const math::Color& clearColor)
			{
				String::StringID strKey;
				strKey.Format("RenderTarget_%d_%llu_%llu_%u_%u_%u_%u_%u_%u_%u_%u_%d_%d_%d_%d",
					pDesc->Dimension,
					pDesc->Alignment,
					pDesc->Width,
					pDesc->Height,
					pDesc->DepthOrArraySize,
					pDesc->MipLevels,
					pDesc->Format,
					pDesc->SampleDesc,
					pDesc->Layout,
					pDesc->Flags,
					static_cast<int>(clearColor.r * 255),
					static_cast<int>(clearColor.g * 255),
					static_cast<int>(clearColor.b * 255),
					static_cast<int>(clearColor.a * 255));

				return { strKey.Key() };
			}

			D3D12_RESOURCE_DESC RenderTarget::GetDesc() const
			{
				return GetResource()->GetDesc();
			}

			D3D12_CPU_DESCRIPTOR_HANDLE RenderTarget::GetCPUHandle() const
			{
				DescriptorHeap* pDescriptorHeap = Device::GetInstance()->GetRTVDescriptorHeap();
				return pDescriptorHeap->GetCPUHandleFromIndex(m_nDescriptorIndex);
			}

			D3D12_GPU_DESCRIPTOR_HANDLE RenderTarget::GetGPUHandle() const
			{
				DescriptorHeap* pDescriptorHeap = Device::GetInstance()->GetSRVDescriptorHeap();
				return pDescriptorHeap->GetGPUHandleFromIndex(m_nDescriptorIndex);
			}

			ID3D12Resource* RenderTarget::GetResource() const
			{
				assert(m_pTexture != nullptr);
				return m_pTexture->GetResource();
			}
		}
	}
}