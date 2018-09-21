#pragma once

#include "GraphicsInterface/GraphicsInterface.h"

struct ID3D12Resource;
struct ID3D12DescriptorHeap;
struct ID3D12GraphicsCommandList;

struct D3D12_SHADER_RESOURCE_VIEW_DESC;
struct D3D12_CPU_DESCRIPTOR_HANDLE;
struct D3D12_GPU_DESCRIPTOR_HANDLE;

namespace eastengine
{
	namespace graphics
	{
		namespace dx12
		{
			class Texture : public ITexture
			{
			public:
				Texture(const ITexture::Key& key);
				virtual ~Texture();

			public:
				virtual const ITexture::Key& GetKey() const override;
				virtual const String::StringID& GetName() const override;

			public:
				virtual const math::UInt2& GetSize() const override;
				virtual const std::string& GetPath() const override;

			public:
				bool Initialize(const D3D12_RESOURCE_DESC* pDesc);
				bool Load(const char* strFilePath);
				bool Bind(ID3D12Resource* pResource, const D3D12_SHADER_RESOURCE_VIEW_DESC* pDesc);

			public:
				ID3D12Resource* GetResource() const;
				uint32_t GetDescriptorIndex() const;

				const D3D12_CPU_DESCRIPTOR_HANDLE& GetCPUHandle(int nFrameIndex) const;
				const D3D12_GPU_DESCRIPTOR_HANDLE& GetGPUHandle(int nFrameIndex) const;

			private:
				class Impl;
				std::unique_ptr<Impl> m_pImpl;
			};
		}
	}
}