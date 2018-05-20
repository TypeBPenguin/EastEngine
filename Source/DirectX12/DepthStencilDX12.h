#pragma once

#include "DefineDX12.h"
#include "TextureDX12.h"

namespace eastengine
{
	namespace graphics
	{
		namespace dx12
		{
			class DepthStencil
			{
			public:
				DepthStencil();
				~DepthStencil();

			public:
				static std::unique_ptr<DepthStencil> Create(const D3D12_RESOURCE_DESC* pResourceDesc, D3D12_RESOURCE_STATES resourceState = D3D12_RESOURCE_STATE_DEPTH_WRITE, uint32_t nMipSlice = 0, uint32_t nFirstArraySlice = 0, uint32_t nArraySize = -1);

			public:
				uint32_t GetDescriptorIndex() const { return m_nDescriptorIndex; }
				D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle() const;
				D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle() const;

				ID3D12Resource* GetResource() const;
				Texture* GetTexture() const { return m_pTexture.get(); }

			private:
				uint32_t m_nDescriptorIndex{ eInvalidDescriptorIndex };
				std::unique_ptr<Texture> m_pTexture;
			};
		}
	}
}