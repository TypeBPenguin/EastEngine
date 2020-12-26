#include "stdafx.h"
#include "DepthStencilDX12.h"

#include "UtilDX12.h"
#include "DeviceDX12.h"
#include "DescriptorHeapDX12.h"

namespace est
{
	namespace graphics
	{
		namespace dx12
		{
			DepthStencil::DepthStencil(const Key& key)
				: m_key(key)
			{
			}

			DepthStencil::~DepthStencil()
			{
				util::ReleaseResourceDSV(m_descriptorIndex);
			}

			std::unique_ptr<DepthStencil> DepthStencil::Create(const D3D12_RESOURCE_DESC* pResourceDesc, D3D12_RESOURCE_STATES resourceState, uint32_t nMipSlice, uint32_t nFirstArraySlice, uint32_t nArraySize)
			{
				ID3D12Device* pDevice = Device::GetInstance()->GetInterface();
				DescriptorHeap* pDescriptorHeap = Device::GetInstance()->GetDSVDescriptorHeap();

				const Key key = DepthStencil::BuildKey(pResourceDesc);
				const string::StringID name(key);

				D3D12_RESOURCE_DESC desc = *pResourceDesc;
				desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

				DXGI_FORMAT srvFormat = pResourceDesc->Format;
				if (pResourceDesc->Format == DXGI_FORMAT_D16_UNORM)
				{
					desc.Format = DXGI_FORMAT_R16_TYPELESS;
					srvFormat = DXGI_FORMAT_R16_UNORM;
				}
				else if (pResourceDesc->Format == DXGI_FORMAT_D24_UNORM_S8_UINT)
				{
					desc.Format = DXGI_FORMAT_R24G8_TYPELESS;
					srvFormat = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
				}
				else if (pResourceDesc->Format == DXGI_FORMAT_D32_FLOAT)
				{
					desc.Format = DXGI_FORMAT_R32_TYPELESS;
					srvFormat = DXGI_FORMAT_R32_FLOAT;
				}
				else if (pResourceDesc->Format == DXGI_FORMAT_D32_FLOAT_S8X24_UINT)
				{
					desc.Format = DXGI_FORMAT_R32G8X24_TYPELESS;
					srvFormat = DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
				}
				else
				{
					throw_line("Invalid depth buffer format");
				}

				D3D12_CLEAR_VALUE clearValue{};
				clearValue.Format = pResourceDesc->Format;
				clearValue.DepthStencil.Depth = 1.f;
				clearValue.DepthStencil.Stencil = 0;

				std::unique_ptr<DepthStencil> pDepthStencil = std::make_unique<DepthStencil>(key);

				PersistentDescriptorAlloc dsvAlloc = pDescriptorHeap->AllocatePersistent();
				pDepthStencil->m_descriptorIndex = dsvAlloc.index;

				ID3D12Resource* pResource = nullptr;
				CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_DEFAULT);
				HRESULT hr = pDevice->CreateCommittedResource(&heapProperties,
					D3D12_HEAP_FLAG_NONE,
					&desc,
					resourceState,
					&clearValue,
					IID_PPV_ARGS(&pResource));
				if (FAILED(hr))
				{
					throw_line("failed to create DepthStencil");
				}
				pResource->SetName(name.c_str());

				D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
				dsvDesc.Format = pResourceDesc->Format;
				dsvDesc.ViewDimension = (desc.DepthOrArraySize == 1) ? D3D12_DSV_DIMENSION_TEXTURE2D : D3D12_DSV_DIMENSION_TEXTURE2DARRAY;

				switch (dsvDesc.ViewDimension)
				{
				case D3D12_DSV_DIMENSION_TEXTURE2D:
					dsvDesc.Texture2D.MipSlice = nMipSlice;
					break;
				case D3D12_DSV_DIMENSION_TEXTURE2DARRAY:
					dsvDesc.Texture2DArray.MipSlice = nMipSlice;
					dsvDesc.Texture2DArray.FirstArraySlice = nFirstArraySlice;
					dsvDesc.Texture2DArray.ArraySize = nArraySize;
					break;
				case D3D12_DSV_DIMENSION_TEXTURE2DMS:
					break;
				case D3D12_DSV_DIMENSION_TEXTURE2DMSARRAY:
					dsvDesc.Texture2DMSArray.FirstArraySlice = nFirstArraySlice;
					dsvDesc.Texture2DMSArray.ArraySize = nArraySize;
					break;
				default: break;
				}

				pDevice->CreateDepthStencilView(pResource, &dsvDesc, dsvAlloc.cpuHandle);

				D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
				srvDesc.Format = srvFormat;
				srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
				if (pResourceDesc->DepthOrArraySize == 1)
				{
					srvDesc.Texture2D.MipLevels = 1;
					srvDesc.Texture2D.MostDetailedMip = 0;
					srvDesc.Texture2D.PlaneSlice = 0;
					srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
					srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
				}
				else if (pResourceDesc->DepthOrArraySize > 1)
				{
					srvDesc.Texture2DArray.ArraySize = static_cast<uint32_t>(pResourceDesc->DepthOrArraySize);
					srvDesc.Texture2DArray.FirstArraySlice = 0;
					srvDesc.Texture2DArray.MipLevels = 1;
					srvDesc.Texture2DArray.MostDetailedMip = 0;
					srvDesc.Texture2DArray.PlaneSlice = 0;
					srvDesc.Texture2DArray.ResourceMinLODClamp = 0.0f;
					srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
				}

				pDepthStencil->m_pTexture = std::make_unique<Texture>(Texture::Key(name), &resourceState);
				if (pDepthStencil->m_pTexture->Bind(pResource, &srvDesc) == false)
				{
					throw_line("failed to create depth stencil texture");
				}

				return pDepthStencil;
			}

			DepthStencil::Key DepthStencil::BuildKey(const D3D12_RESOURCE_DESC* pDesc)
			{
				return DepthStencil::Key(string::Format(L"DepthStencil_%d_%llu_%llu_%u_%u_%u_%u_%u_%u_%u",
					pDesc->Dimension,
					pDesc->Alignment,
					pDesc->Width,
					pDesc->Height,
					pDesc->DepthOrArraySize,
					pDesc->MipLevels,
					pDesc->Format,
					pDesc->SampleDesc,
					pDesc->Layout,
					pDesc->Flags));
			}

			void DepthStencil::Clear(ID3D12GraphicsCommandList* pCommandList)
			{
				pCommandList->ClearDepthStencilView(GetCPUHandle(), D3D12_CLEAR_FLAG_DEPTH, 1.f, 0, 0, nullptr);
			}

			D3D12_RESOURCE_BARRIER DepthStencil::Transition(D3D12_RESOURCE_STATES changeState)
			{
				D3D12_RESOURCE_BARRIER barrier{};
				m_pTexture->Transition(changeState, &barrier);

				return barrier;
			}

			const DepthStencil::Key& DepthStencil::GetKey() const
			{
				return m_key;
			}

			D3D12_RESOURCE_DESC DepthStencil::GetDesc() const
			{
				return GetResource()->GetDesc();
			}

			D3D12_CPU_DESCRIPTOR_HANDLE DepthStencil::GetCPUHandle() const
			{
				DescriptorHeap* pDescriptorHeap = Device::GetInstance()->GetDSVDescriptorHeap();
				return pDescriptorHeap->GetCPUHandleFromIndex(m_descriptorIndex);
			}

			D3D12_GPU_DESCRIPTOR_HANDLE DepthStencil::GetGPUHandle() const
			{
				DescriptorHeap* pDescriptorHeap = Device::GetInstance()->GetDSVDescriptorHeap();
				return pDescriptorHeap->GetGPUHandleFromIndex(m_descriptorIndex);
			}

			ID3D12Resource* DepthStencil::GetResource() const
			{
				assert(m_pTexture != nullptr);
				return m_pTexture->GetResource();
			}
		}
	}
}