#pragma once

#include "Graphics/Interface/GraphicsInterface.h"

struct ID3D12Resource;
struct ID3D12DescriptorHeap;
struct ID3D12GraphicsCommandList;

struct D3D12_SHADER_RESOURCE_VIEW_DESC;
struct D3D12_CPU_DESCRIPTOR_HANDLE;
struct D3D12_GPU_DESCRIPTOR_HANDLE;

struct D3D12_RESOURCE_BARRIER;

enum D3D12_RESOURCE_STATES;

namespace est
{
	namespace graphics
	{
		namespace dx12
		{
			class Texture : public ITexture
			{
			public:
				Texture(const ITexture::Key& key);
				Texture(const ITexture::Key& key, const D3D12_RESOURCE_STATES* pDefaultState);
				virtual ~Texture();

			public:
				virtual const ITexture::Key& GetKey() const override;
				virtual const string::StringID& GetName() const override;

			public:
				virtual const math::uint2& GetSize() const override;
				virtual const std::wstring& GetPath() const override;

			public:
				bool Initialize(const TextureDesc& desc);
				bool Initialize(const D3D12_RESOURCE_DESC* pDesc);
				bool Load(const wchar_t* filePath);
				bool Bind(ID3D12Resource* pResource, const D3D12_SHADER_RESOURCE_VIEW_DESC* pDesc);

			public:
				ID3D12Resource* GetResource() const;
				uint32_t GetDescriptorIndex() const;

				const D3D12_CPU_DESCRIPTOR_HANDLE& GetCPUHandle(uint32_t frameIndex) const;
				const D3D12_GPU_DESCRIPTOR_HANDLE& GetGPUHandle(uint32_t frameIndex) const;

				void Transition(D3D12_RESOURCE_STATES changeState, D3D12_RESOURCE_BARRIER* pBarrier_out);
				D3D12_RESOURCE_STATES GetResourceState() const;

			private:
				class Impl;
				std::unique_ptr<Impl> m_pImpl;
			};
		}
	}
}