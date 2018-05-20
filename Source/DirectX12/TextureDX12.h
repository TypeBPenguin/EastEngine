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
				virtual const ITexture::Key& GetKey() const override { return m_key; }

			public:
				virtual const math::UInt2& GetSize() const override { return m_n2Size; }
				virtual const std::string& GetPath() const override { return m_strPath; }

			public:
				bool Initialize(const D3D12_RESOURCE_DESC* pDesc, bool isDynamic = false);
				bool Load(const char* strFilePath);
				bool Bind(ID3D12Resource* pResource, const D3D12_SHADER_RESOURCE_VIEW_DESC* pDesc);

			public:
				ID3D12Resource* GetResource() const { return m_pResource; }
				uint32_t GetDescriptorIndex() const { return m_nDescriptorIndex; }

				D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(int nFrameIndex) const;
				D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(int nFrameIndex) const;

			private:
				const ITexture::Key m_key;

				math::UInt2 m_n2Size;
				std::string m_strPath;

				uint32_t m_nDescriptorIndex{ std::numeric_limits<uint32_t>::max() };
				ID3D12Resource* m_pResource{ nullptr };
			};
		}
	}
}