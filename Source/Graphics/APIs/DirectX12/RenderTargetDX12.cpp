#include "stdafx.h"
#include "RenderTargetDX12.h"

#include "UtilDX12.h"
#include "DeviceDX12.h"
#include "DescriptorHeapDX12.h"

namespace est
{
	namespace graphics
	{
		namespace dx12
		{
			class RenderTarget::Impl
			{
			public:
				Impl(const RenderTarget::Key& key);
				~Impl();

			public:
				void Clear(ID3D12GraphicsCommandList* pCommandList);
				D3D12_RESOURCE_BARRIER Transition(D3D12_RESOURCE_STATES changeState);
				D3D12_RESOURCE_STATES GetResourceState() const { return m_pTexture->GetResourceState(); }

			public:
				const Key& GetKey() const { return m_key; }
				D3D12_RESOURCE_DESC GetDesc() const;

				uint32_t GetDescriptorIndex() const { return m_descriptorIndex; }
				const D3D12_CPU_DESCRIPTOR_HANDLE& GetCPUHandle() const { return m_cpuHandle; }

				ID3D12Resource* GetResource() const;
				Texture* GetTexture() const { return m_pTexture.get(); }

			public:
				const Key m_key;

				uint32_t m_descriptorIndex{ eInvalidDescriptorIndex };
				D3D12_CPU_DESCRIPTOR_HANDLE m_cpuHandle{};

				std::unique_ptr<Texture> m_pTexture;
				math::Color m_colorClearValue{ math::Color::Transparent };
			};

			RenderTarget::Impl::Impl(const RenderTarget::Key& key)
				: m_key(key)
			{
			}

			RenderTarget::Impl::~Impl()
			{
				util::ReleaseResourceRTV(m_descriptorIndex);
			}

			void RenderTarget::Impl::Clear(ID3D12GraphicsCommandList* pCommandList)
			{
				pCommandList->ClearRenderTargetView(GetCPUHandle(), &m_colorClearValue.r, 0, nullptr);
			}

			D3D12_RESOURCE_BARRIER RenderTarget::Impl::Transition(D3D12_RESOURCE_STATES changeState)
			{
				D3D12_RESOURCE_BARRIER barrier{};
				m_pTexture->Transition(changeState, &barrier);
				return barrier;
			}

			D3D12_RESOURCE_DESC RenderTarget::Impl::GetDesc() const
			{
				return GetResource()->GetDesc();
			}

			ID3D12Resource* RenderTarget::Impl::GetResource() const
			{
				assert(m_pTexture != nullptr);
				return m_pTexture->GetResource();
			}

			RenderTarget::RenderTarget(const Key& key)
				: m_pImpl{ std::make_unique<Impl>(key) }
			{
			}

			RenderTarget::~RenderTarget()
			{
			}

			std::unique_ptr<RenderTarget> RenderTarget::Create(ID3D12Resource* pResource, const math::Color& clearColor)
			{
				ID3D12Device* pDevice = Device::GetInstance()->GetInterface();
				DescriptorHeap* pDescriptorHeap = Device::GetInstance()->GetRTVDescriptorHeap();

				D3D12_RESOURCE_DESC desc = pResource->GetDesc();

				const Key key = RenderTarget::BuildKey(&desc, clearColor);
				const string::StringID name(key);

				std::unique_ptr<RenderTarget> pRenderTarget = std::make_unique<RenderTarget>(key);
				pRenderTarget->m_pImpl->m_colorClearValue = clearColor;

				PersistentDescriptorAlloc rtvAlloc = pDescriptorHeap->AllocatePersistent();
				pRenderTarget->m_pImpl->m_descriptorIndex = rtvAlloc.index;

				pRenderTarget->m_pImpl->m_cpuHandle = pDescriptorHeap->GetCPUHandleFromIndex(pRenderTarget->m_pImpl->m_descriptorIndex);

				pDevice->CreateRenderTargetView(pResource, nullptr, rtvAlloc.cpuHandle);

				const D3D12_RESOURCE_STATES states{ D3D12_RESOURCE_STATE_PRESENT };
				pRenderTarget->m_pImpl->m_pTexture = std::make_unique<Texture>(Texture::Key(name), &states);
				if (pRenderTarget->m_pImpl->m_pTexture->Bind(pResource, nullptr) == false)
				{
					throw_line("failed to create render target texture");
				}
				pResource->SetName(name.c_str());

				return pRenderTarget;
			}

			std::unique_ptr<RenderTarget> RenderTarget::Create(const D3D12_RESOURCE_DESC* pResourceDesc, const math::Color& clearColor, D3D12_RESOURCE_STATES resourceState, uint32_t nMipSlice, uint32_t nFirstArraySlice, uint32_t nArraySize)
			{
				ID3D12Device* pDevice = Device::GetInstance()->GetInterface();
				DescriptorHeap* pRTVDescriptorHeap = Device::GetInstance()->GetRTVDescriptorHeap();

				const Key key = RenderTarget::BuildKey(pResourceDesc, clearColor);
				const string::StringID name(key);

				std::unique_ptr<RenderTarget> pRenderTarget = std::make_unique<RenderTarget>(key);
				pRenderTarget->m_pImpl->m_colorClearValue = clearColor;

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
				pRenderTarget->m_pImpl->m_descriptorIndex = rtvAlloc.index;

				pRenderTarget->m_pImpl->m_cpuHandle = pRTVDescriptorHeap->GetCPUHandleFromIndex(pRenderTarget->m_pImpl->m_descriptorIndex);

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
				pResource->SetName(name.c_str());

				pDevice->CreateRenderTargetView(pResource, &rtvDesc, rtvAlloc.cpuHandle);

				pRenderTarget->m_pImpl->m_pTexture = std::make_unique<Texture>(Texture::Key(name), &resourceState);
				if (pRenderTarget->m_pImpl->m_pTexture->Bind(pResource, nullptr) == false)
				{
					throw_line("failed to create render target texture");
				}

				return pRenderTarget;
			}

			RenderTarget::Key RenderTarget::BuildKey(const D3D12_RESOURCE_DESC* pDesc, const math::Color& clearColor)
			{
				return RenderTarget::Key(string::Format(L"RenderTarget_%d_%llu_%llu_%u_%u_%u_%u_%u_%u_%u_%u_%u_%d_%d",
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
					static_cast<int>(clearColor.a * 255)));
			}

			void RenderTarget::Clear(ID3D12GraphicsCommandList* pCommandList)
			{
				m_pImpl->Clear(pCommandList);
			}

			D3D12_RESOURCE_BARRIER RenderTarget::Transition(D3D12_RESOURCE_STATES changeState)
			{
				return m_pImpl->Transition(changeState);
			}

			D3D12_RESOURCE_STATES RenderTarget::GetResourceState() const
			{
				return m_pImpl->GetResourceState();
			}

			const RenderTarget::Key& RenderTarget::GetKey() const
			{
				return m_pImpl->GetKey();
			}

			D3D12_RESOURCE_DESC RenderTarget::GetDesc() const
			{
				return m_pImpl->GetDesc();
			}

			uint32_t RenderTarget::GetDescriptorIndex() const
			{
				return m_pImpl->GetDescriptorIndex();
			}

			const D3D12_CPU_DESCRIPTOR_HANDLE& RenderTarget::GetCPUHandle() const
			{
				return m_pImpl->GetCPUHandle();
			}

			ID3D12Resource* RenderTarget::GetResource() const
			{
				return m_pImpl->GetResource();
			}

			Texture* RenderTarget::GetTexture() const
			{
				return m_pImpl->GetTexture();
			}
		}
	}
}