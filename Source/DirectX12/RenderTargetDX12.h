#pragma once

#include "DefineDX12.h"
#include "TextureDX12.h"

namespace eastengine
{
	namespace graphics
	{
		namespace dx12
		{
			class RenderTarget
			{
			private:
				struct tKey { static constexpr const char* DefaultValue() { return ""; } };

			public:
				using Key = PhantomType<tKey, string::StringID>;

			public:
				RenderTarget(const Key& key);
				~RenderTarget();

			public:
				static std::unique_ptr<RenderTarget> Create(ID3D12Resource* pResource, const math::Color& clearColor = math::Color::Black);
				static std::unique_ptr<RenderTarget> Create(const D3D12_RESOURCE_DESC* pResourceDesc, const math::Color& clearColor, D3D12_RESOURCE_STATES resourceState = D3D12_RESOURCE_STATE_RENDER_TARGET, uint32_t nMipSlice = 0, uint32_t nFirstArraySlice = 0, uint32_t nArraySize = -1);
				static Key BuildKey(const D3D12_RESOURCE_DESC* pDesc, const math::Color& clearColor);

			public:
				void Clear(ID3D12GraphicsCommandList* pCommandList);
				D3D12_RESOURCE_BARRIER Transition(D3D12_RESOURCE_STATES changeState);
				D3D12_RESOURCE_STATES GetResourceState() const { return m_pTexture->GetResourceState(); }

			public:
				const Key& GetKey() const { return m_key; }
				D3D12_RESOURCE_DESC GetDesc() const;

				uint32_t GetDescriptorIndex() const { return m_nDescriptorIndex; }
				D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle() const;
				D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle() const;

				ID3D12Resource* GetResource() const;
				Texture* GetTexture() const { return m_pTexture.get(); }

			private:
				const Key m_key;

				uint32_t m_nDescriptorIndex{ eInvalidDescriptorIndex };
				std::unique_ptr<Texture> m_pTexture;
				math::Color m_colorClearValue{ math::Color::Transparent };
			};
		}
	}
}

namespace std
{
	template <>
	struct hash<eastengine::graphics::dx12::RenderTarget::Key>
	{
		const size_t operator()(const eastengine::graphics::dx12::RenderTarget::Key& key) const
		{
			return reinterpret_cast<size_t>(key.Value().Key());
		}
	};
}