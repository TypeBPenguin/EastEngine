#pragma once

#include "DefineDX12.h"
#include "TextureDX12.h"

namespace est
{
	namespace graphics
	{
		namespace dx12
		{
			class DepthStencil
			{
			private:
				struct tKey { static constexpr const wchar_t* DefaultValue() { return L""; } };

			public:
				using Key = PhantomType<tKey, string::StringID>;

			public:
				DepthStencil(const Key& key);
				~DepthStencil();

			public:
				static std::unique_ptr<DepthStencil> Create(const D3D12_RESOURCE_DESC* pResourceDesc, D3D12_RESOURCE_STATES resourceState = D3D12_RESOURCE_STATE_DEPTH_WRITE, uint32_t nMipSlice = 0, uint32_t nFirstArraySlice = 0, uint32_t nArraySize = -1);
				static Key BuildKey(const D3D12_RESOURCE_DESC* pDesc);

			public:
				void Clear(ID3D12GraphicsCommandList* pCommandList);
				D3D12_RESOURCE_BARRIER Transition(D3D12_RESOURCE_STATES changeState);
				D3D12_RESOURCE_STATES GetResourceState() const { return m_pTexture->GetResourceState(); }

			public:
				const Key& GetKey() const;
				D3D12_RESOURCE_DESC GetDesc() const;

				uint32_t GetDescriptorIndex() const { return m_descriptorIndex; }
				D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle() const;
				D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle() const;

				ID3D12Resource* GetResource() const;
				Texture* GetTexture() const { return m_pTexture.get(); }

			private:
				const Key m_key;
				uint32_t m_descriptorIndex{ eInvalidDescriptorIndex };
				std::unique_ptr<Texture> m_pTexture;
			};
		}
	}
}

namespace std
{
	template <>
	struct hash<est::graphics::dx12::DepthStencil::Key>
	{
		const size_t operator()(const est::graphics::dx12::DepthStencil::Key& key) const
		{
			return reinterpret_cast<size_t>(key.Value().GetData());
		}
	};
}