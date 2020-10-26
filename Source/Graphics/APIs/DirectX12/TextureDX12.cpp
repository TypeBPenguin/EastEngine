#include "stdafx.h"
#include "TextureDX12.h"

#include "CommonLib/FileUtil.h"

#include "UtilDX12.h"
#include "DeviceDX12.h"
#include "DescriptorHeapDX12.h"
#include "UploadDX12.h"

namespace est
{
	namespace graphics
	{
		namespace dx12
		{
			struct CoInitializer
			{
				CoInitializer()
				{
					CoInitialize(nullptr);
				}

				~CoInitializer()
				{
					CoUninitialize();
				}
			};

			class Texture::Impl
			{
			public:
				Impl(const ITexture::Key& key);
				Impl(const ITexture::Key& key, const D3D12_RESOURCE_STATES* pDefaultState);
				~Impl();

			public:
				const ITexture::Key& GetKey() const { return m_key; }
				const string::StringID& GetName() const { return m_key.Value(); }

			public:
				const math::uint2& GetSize() const { return m_n2Size; }
				const std::wstring& GetPath() const { return m_path; }

			public:
				bool Initialize(const D3D12_RESOURCE_DESC* pDesc);
				bool Load(const wchar_t* filePath);
				bool Bind(ID3D12Resource* pResource, const D3D12_SHADER_RESOURCE_VIEW_DESC* pDesc);

			public:
				ID3D12Resource* GetResource() const { return m_pResource; }
				uint32_t GetDescriptorIndex() const { return m_descriptorIndex; }

				const D3D12_CPU_DESCRIPTOR_HANDLE& GetCPUHandle(uint32_t frameIndex) const { return m_cpuHandle; }
				const D3D12_GPU_DESCRIPTOR_HANDLE& GetGPUHandle(uint32_t frameIndex) const { return m_gpuHandle; }

				D3D12_RESOURCE_BARRIER Transition(D3D12_RESOURCE_STATES changeState);
				D3D12_RESOURCE_STATES GetResourceState() const { return m_state; }

			private:
				const ITexture::Key m_key;

				math::uint2 m_n2Size;
				std::wstring m_path;

				uint32_t m_descriptorIndex{ eInvalidDescriptorIndex };
				D3D12_CPU_DESCRIPTOR_HANDLE m_cpuHandle{};
				D3D12_GPU_DESCRIPTOR_HANDLE m_gpuHandle{};

				ID3D12Resource* m_pResource{ nullptr };

				D3D12_RESOURCE_STATES m_state{ D3D12_RESOURCE_STATE_COMMON };
			};

			Texture::Impl::Impl(const ITexture::Key& key)
				: m_key(key)
				, m_descriptorIndex(eInvalidDescriptorIndex)
			{
			}

			Texture::Impl::Impl(const ITexture::Key& key, const D3D12_RESOURCE_STATES* pDefaultState)
				: m_key(key)
				, m_descriptorIndex(eInvalidDescriptorIndex)
				, m_state(*pDefaultState)
			{
			}

			Texture::Impl::~Impl()
			{
				util::ReleaseResource(m_pResource);
				m_pResource = nullptr;

				util::ReleaseResourceSRV(m_descriptorIndex);
			}

			bool Texture::Impl::Initialize(const D3D12_RESOURCE_DESC* pDesc)
			{
				const string::StringID name(m_key);

				ID3D12Device* pDevice = Device::GetInstance()->GetInterface();

				CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_DEFAULT, 0, 0);
				ID3D12Resource* pResource = nullptr;
				HRESULT hr = pDevice->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, pDesc,
					D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&pResource));
				if (FAILED(hr))
				{
					LOG_ERROR(L"failed to create resource : %s", name.c_str());
					return false;
				}

				m_n2Size.x = static_cast<uint32_t>(pDesc->Width);
				m_n2Size.y = static_cast<uint32_t>(pDesc->Height);

				pResource->SetName(name.c_str());

				const uint16_t numSubResources = pDesc->MipLevels * pDesc->DepthOrArraySize;
				D3D12_PLACED_SUBRESOURCE_FOOTPRINT* layouts = static_cast<D3D12_PLACED_SUBRESOURCE_FOOTPRINT*>(_malloca(sizeof(D3D12_PLACED_SUBRESOURCE_FOOTPRINT) * numSubResources));
				uint32_t* numRows = static_cast<uint32_t*>(_malloca(sizeof(uint32_t) * numSubResources));
				uint64_t* rowSizes = static_cast<uint64_t*>(_malloca(sizeof(uint64_t) * numSubResources));

				uint64_t nTextureMemSize = 0;
				pDevice->GetCopyableFootprints(pDesc, 0, uint32_t(numSubResources), 0, layouts, numRows, rowSizes, &nTextureMemSize);

				Uploader* pUploader = Device::GetInstance()->GetUploader();

				// Get a GPU upload buffer
				UploadContext uploadContext = pUploader->BeginResourceUpload(nTextureMemSize);
				uint8_t* uploadMem = reinterpret_cast<uint8_t*>(uploadContext.pCPUAddress);

				for (uint64_t arrayIdx = 0; arrayIdx < pDesc->DepthOrArraySize; ++arrayIdx)
				{
					for (uint64_t mipIdx = 0; mipIdx < pDesc->MipLevels; ++mipIdx)
					{
						const uint64_t subResourceIdx = mipIdx + (arrayIdx * pDesc->MipLevels);

						const D3D12_PLACED_SUBRESOURCE_FOOTPRINT& subResourceLayout = layouts[subResourceIdx];
						const uint32_t subResourceHeight = numRows[subResourceIdx];
						const uint64_t subResourcePitch = subResourceLayout.Footprint.RowPitch;
						const uint64_t subResourceDepth = subResourceLayout.Footprint.Depth;
						uint8_t* dstSubResourceMem = reinterpret_cast<uint8_t*>(uploadMem) + subResourceLayout.Offset;

						for (uint64_t z = 0; z < subResourceDepth; ++z)
						{
							for (uint32_t y = 0; y < subResourceHeight; ++y)
							{
								ZeroMemory(dstSubResourceMem, subResourcePitch);
								dstSubResourceMem += subResourcePitch;
							}
						}
					}
				}

				for (uint64_t subResourceIdx = 0; subResourceIdx < numSubResources; ++subResourceIdx)
				{
					D3D12_TEXTURE_COPY_LOCATION dst{};
					dst.pResource = pResource;
					dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
					dst.SubresourceIndex = uint32_t(subResourceIdx);
					D3D12_TEXTURE_COPY_LOCATION src{};
					src.pResource = uploadContext.pResource;
					src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
					src.PlacedFootprint = layouts[subResourceIdx];
					src.PlacedFootprint.Offset += uploadContext.nResourceOffset;
					uploadContext.pCommandList->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);
				}

				pUploader->EndResourceUpload(uploadContext);

				if ((pDesc->Flags & D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE) != D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE)
				{
					if (Bind(pResource, nullptr) == false)
					{
						throw_line("failed to bind texture resource");
					}
				}

				_freea(layouts);
				_freea(numRows);
				_freea(rowSizes);

				return true;
			}

			bool Texture::Impl::Load(const wchar_t* filePath)
			{
				TRACER_EVENT(L"TextureLoad");
				TRACER_PUSHARGS(L"FilePath", filePath);

				CoInitializer coInitializer;

				m_path = filePath;

				HRESULT hr{ S_OK };
				DirectX::ScratchImage image{};

				ID3D12Device* pDevice = Device::GetInstance()->GetInterface();

				const std::wstring strFileExtension(file::GetFileExtension(filePath));
				if (string::IsEqualsNoCase(strFileExtension.c_str(), L".dds"))
				{
					hr = DirectX::LoadFromDDSFile(filePath, DirectX::DDS_FLAGS_NONE, nullptr, image);

					if (FAILED(hr))
					{
						LOG_ERROR(L"failed to load texture : %s", filePath);
						return false;
					}
				}
				else if (string::IsEqualsNoCase(strFileExtension.c_str(), L".tga"))
				{
					DirectX::ScratchImage tempImage;
					hr = DirectX::LoadFromTGAFile(filePath, nullptr, tempImage);

					if (FAILED(hr))
					{
						LOG_ERROR(L"failed to load texture : %s", filePath);
						return false;
					}

					hr = DirectX::GenerateMipMaps(*tempImage.GetImage(0, 0, 0), DirectX::TEX_FILTER_DEFAULT, 0, image, false);
				}
				else
				{
					DirectX::ScratchImage tempImage;
					hr = DirectX::LoadFromWICFile(filePath, DirectX::WIC_FLAGS_NONE, nullptr, tempImage);

					if (FAILED(hr))
					{
						LOG_ERROR(L"failed to load texture : %s", filePath);
						return false;
					}

					hr = DirectX::GenerateMipMaps(*tempImage.GetImage(0, 0, 0), DirectX::TEX_FILTER_DEFAULT, 0, image, false);
				}

				if (FAILED(hr))
				{
					LOG_ERROR(L"failed to load texture : %s", filePath);
					return false;
				}

				const DirectX::TexMetadata& metaData = image.GetMetadata();
				DXGI_FORMAT format = metaData.format;
				//if (forceSRGB)
				//{
				//	format = DirectX::MakeSRGB(format);
				//}

				const bool is3D = metaData.dimension == DirectX::TEX_DIMENSION_TEXTURE3D;

				D3D12_RESOURCE_DESC textureDesc{};
				textureDesc.MipLevels = static_cast<uint16_t>(metaData.mipLevels);
				textureDesc.Format = format;
				textureDesc.Width = metaData.width;
				textureDesc.Height = static_cast<uint32_t>(metaData.height);
				textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
				textureDesc.DepthOrArraySize = static_cast<uint16_t>(is3D ? (metaData.depth) : (metaData.arraySize));
				textureDesc.SampleDesc.Count = 1;
				textureDesc.SampleDesc.Quality = 0;
				textureDesc.Dimension = is3D ? D3D12_RESOURCE_DIMENSION_TEXTURE3D : D3D12_RESOURCE_DIMENSION_TEXTURE2D;
				textureDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
				textureDesc.Alignment = 0;

				CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_DEFAULT, 0, 0);
				hr = pDevice->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &textureDesc,
					D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&m_pResource));
				if (FAILED(hr))
				{
					LOG_ERROR(L"failed to create resource : %s", filePath);
					return false;
				}

				m_n2Size.x = static_cast<uint32_t>(textureDesc.Width);
				m_n2Size.y = static_cast<uint32_t>(textureDesc.Height);

				const std::wstring wname = file::GetFileName(filePath);
				m_pResource->SetName(wname.c_str());

				DescriptorHeap* pDescriptorHeap = Device::GetInstance()->GetSRVDescriptorHeap();
				PersistentDescriptorAlloc srvAlloc = pDescriptorHeap->AllocatePersistent();
				m_descriptorIndex = srvAlloc.index;

				//for (int i = 0; i < eFrameBufferCount; ++i)
				{
					m_cpuHandle = pDescriptorHeap->GetCPUHandleFromIndex(m_descriptorIndex);
					m_gpuHandle = pDescriptorHeap->GetGPUHandleFromIndex(m_descriptorIndex);
				}

				D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
				srvDesc.Format = format;
				srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
				if (metaData.IsCubemap() == true)
				{
					assert(metaData.arraySize == 6);
					srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
					srvDesc.TextureCube.MostDetailedMip = 0;
					srvDesc.TextureCube.MipLevels = static_cast<uint32_t>(metaData.mipLevels);
					srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;
				}
				else
				{
					switch (metaData.dimension)
					{
					case DirectX::TEX_DIMENSION_TEXTURE1D:
						if (metaData.arraySize > 1)
						{
							srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
							srvDesc.Texture1DArray.MipLevels = static_cast<uint32_t>(metaData.mipLevels);
							srvDesc.Texture1DArray.ArraySize = static_cast<uint32_t>(metaData.arraySize);
						}
						else
						{
							srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
							srvDesc.Texture1D.MipLevels = static_cast<uint32_t>(metaData.mipLevels);
						}
						break;
					case DirectX::TEX_DIMENSION_TEXTURE2D:
						if (metaData.arraySize > 1)
						{
							srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
							srvDesc.Texture2DArray.MipLevels = static_cast<uint32_t>(metaData.mipLevels);
							srvDesc.Texture2DArray.ArraySize = static_cast<uint32_t>(metaData.arraySize);
						}
						else
						{
							srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
							srvDesc.Texture2D.MipLevels = static_cast<uint32_t>(metaData.mipLevels);
						}
						break;
					case DirectX::TEX_DIMENSION_TEXTURE3D:
						assert(metaData.arraySize == 1);
						srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
						srvDesc.Texture3D.MipLevels = static_cast<uint32_t>(metaData.mipLevels);
						break;
					default:
						throw_line("unknown dimension");
					}
				}

				pDevice->CreateShaderResourceView(m_pResource, &srvDesc, srvAlloc.cpuHandle);

				const uint64_t numSubResources = metaData.mipLevels * metaData.arraySize;
				D3D12_PLACED_SUBRESOURCE_FOOTPRINT* layouts = static_cast<D3D12_PLACED_SUBRESOURCE_FOOTPRINT*>(_malloca(sizeof(D3D12_PLACED_SUBRESOURCE_FOOTPRINT) * numSubResources));
				uint32_t* numRows = static_cast<uint32_t*>(_malloca(sizeof(uint32_t) * numSubResources));
				uint64_t* rowSizes = static_cast<uint64_t*>(_malloca(sizeof(uint64_t) * numSubResources));

				uint64_t nTextureMemSize = 0;
				pDevice->GetCopyableFootprints(&textureDesc, 0, uint32_t(numSubResources), 0, layouts, numRows, rowSizes, &nTextureMemSize);

				Uploader* pUploader = Device::GetInstance()->GetUploader();

				// Get a GPU upload buffer
				UploadContext uploadContext = pUploader->BeginResourceUpload(nTextureMemSize);
				uint8_t* uploadMem = reinterpret_cast<uint8_t*>(uploadContext.pCPUAddress);

				for (uint64_t arrayIdx = 0; arrayIdx < metaData.arraySize; ++arrayIdx)
				{
					for (uint64_t mipIdx = 0; mipIdx < metaData.mipLevels; ++mipIdx)
					{
						const uint64_t subResourceIdx = mipIdx + (arrayIdx * metaData.mipLevels);

						const D3D12_PLACED_SUBRESOURCE_FOOTPRINT& subResourceLayout = layouts[subResourceIdx];
						const uint64_t subResourceHeight = numRows[subResourceIdx];
						const uint64_t subResourcePitch = subResourceLayout.Footprint.RowPitch;
						const uint64_t subResourceDepth = subResourceLayout.Footprint.Depth;
						uint8_t* dstSubResourceMem = reinterpret_cast<uint8_t*>(uploadMem) + subResourceLayout.Offset;

						for (uint64_t z = 0; z < subResourceDepth; ++z)
						{
							const DirectX::Image* subImage = image.GetImage(mipIdx, arrayIdx, z);
							assert(subImage != nullptr);
							const uint8_t* srcSubResourceMem = subImage->pixels;

							for (uint64_t y = 0; y < subResourceHeight; ++y)
							{
								memcpy(dstSubResourceMem, srcSubResourceMem, std::min(subResourcePitch, subImage->rowPitch));
								dstSubResourceMem += subResourcePitch;
								srcSubResourceMem += subImage->rowPitch;
							}
						}
					}
				}

				for (uint64_t subResourceIdx = 0; subResourceIdx < numSubResources; ++subResourceIdx)
				{
					D3D12_TEXTURE_COPY_LOCATION dst{};
					dst.pResource = m_pResource;
					dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
					dst.SubresourceIndex = uint32_t(subResourceIdx);
					D3D12_TEXTURE_COPY_LOCATION src{};
					src.pResource = uploadContext.pResource;
					src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
					src.PlacedFootprint = layouts[subResourceIdx];
					src.PlacedFootprint.Offset += uploadContext.nResourceOffset;
					uploadContext.pCommandList->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);
				}

				pUploader->EndResourceUpload(uploadContext);

				_freea(layouts);
				_freea(numRows);
				_freea(rowSizes);

				return true;
			}

			bool Texture::Impl::Bind(ID3D12Resource* pResource, const D3D12_SHADER_RESOURCE_VIEW_DESC* pDesc)
			{
				m_pResource = pResource;

				ID3D12Device* pDevice = Device::GetInstance()->GetInterface();

				DescriptorHeap* pDescriptorHeap = Device::GetInstance()->GetSRVDescriptorHeap();
				PersistentDescriptorAlloc srvAlloc = pDescriptorHeap->AllocatePersistent();
				m_descriptorIndex = srvAlloc.index;

				//for (int i = 0; i < eFrameBufferCount; ++i)
				{
					m_cpuHandle = pDescriptorHeap->GetCPUHandleFromIndex(m_descriptorIndex);
					m_gpuHandle = pDescriptorHeap->GetGPUHandleFromIndex(m_descriptorIndex);
				}

				const D3D12_RESOURCE_DESC desc = pResource->GetDesc();
				m_n2Size.x = static_cast<uint32_t>(desc.Width);
				m_n2Size.y = static_cast<uint32_t>(desc.Height);

				D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
				if (pDesc == nullptr)
				{
					pDesc = &srvDesc;

					srvDesc.Format = desc.Format;
					srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

					switch (desc.Dimension)
					{
					case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
						if (desc.DepthOrArraySize > 1)
						{
							srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
							srvDesc.Texture1DArray.MipLevels = static_cast<uint32_t>(desc.MipLevels);
							srvDesc.Texture1DArray.ArraySize = static_cast<uint32_t>(desc.DepthOrArraySize);
						}
						else
						{
							srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
							srvDesc.Texture1D.MipLevels = static_cast<uint32_t>(desc.MipLevels);
						}
						break;
					case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
						if (desc.DepthOrArraySize > 1)
						{
							srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
							srvDesc.Texture2DArray.MipLevels = static_cast<uint32_t>(desc.MipLevels);
							srvDesc.Texture2DArray.ArraySize = static_cast<uint32_t>(desc.DepthOrArraySize);
						}
						else
						{
							srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
							srvDesc.Texture2D.MipLevels = static_cast<uint32_t>(desc.MipLevels);
						}
						break;
					case D3D12_RESOURCE_DIMENSION_TEXTURE3D:
						assert(desc.DepthOrArraySize == 1);
						srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
						srvDesc.Texture3D.MipLevels = static_cast<uint32_t>(desc.MipLevels);
						break;
					default:
						throw_line("unknown dimension");
					}
				}

				pDevice->CreateShaderResourceView(m_pResource, pDesc, srvAlloc.cpuHandle);

				return true;
			}

			D3D12_RESOURCE_BARRIER Texture::Impl::Transition(D3D12_RESOURCE_STATES changeState)
			{
				assert(m_state != changeState);
				D3D12_RESOURCE_STATES beforeState = m_state;
				m_state = changeState;
				return CD3DX12_RESOURCE_BARRIER::Transition(GetResource(), beforeState, changeState);
			}

			Texture::Texture(const ITexture::Key& key)
				: m_pImpl{ std::make_unique<Impl>(key) }
			{
			}

			Texture::Texture(const ITexture::Key& key, const D3D12_RESOURCE_STATES* pDefaultState)
				: m_pImpl{ std::make_unique<Impl>(key, pDefaultState) }
			{
			}

			Texture::~Texture()
			{
			}

			const ITexture::Key& Texture::GetKey() const
			{
				return m_pImpl->GetKey();
			}

			const string::StringID& Texture::GetName() const
			{
				return m_pImpl->GetName();
			}

			const math::uint2& Texture::GetSize() const
			{
				return m_pImpl->GetSize();
			}

			const std::wstring& Texture::GetPath() const
			{
				return m_pImpl->GetPath();
			}

			bool Texture::Initialize(const TextureDesc& desc)
			{
				const CD3DX12_RESOURCE_DESC resource_desc = CD3DX12_RESOURCE_DESC::Tex2D(static_cast<DXGI_FORMAT>(desc.resourceFormat), desc.Width, desc.Height, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_NONE, D3D12_TEXTURE_LAYOUT_UNKNOWN);
				bool isSuccess = Initialize(&resource_desc);
				if (isSuccess == false)
					return false;

				ID3D12Device* pDevice = Device::GetInstance()->GetInterface();
				Uploader* pUploader = Device::GetInstance()->GetUploader();

				D3D12_PLACED_SUBRESOURCE_FOOTPRINT placedTexture2D{};
				uint32_t numRows = 0;
				uint64_t rowSizes = 0;
				uint64_t nTextureMemSize = 0;
				pDevice->GetCopyableFootprints(&resource_desc, 0, 1, 0, &placedTexture2D, &numRows, &rowSizes, &nTextureMemSize);

				UploadContext uploadContext = pUploader->BeginResourceUpload(nTextureMemSize);

				if (GetResourceState() != D3D12_RESOURCE_STATE_COPY_DEST)
				{
					const D3D12_RESOURCE_BARRIER transition[] =
					{
						m_pImpl->Transition(D3D12_RESOURCE_STATE_COPY_DEST),
					};
					uploadContext.pCommandList->ResourceBarrier(_countof(transition), transition);
				}

				D3D12_SUBRESOURCE_DATA textureData{};
				textureData.pData = desc.subResourceData.pSysMem;
				textureData.RowPitch = desc.subResourceData.SysMemPitch;
				textureData.SlicePitch = desc.subResourceData.SysMemSlicePitch;
				UpdateSubresources(uploadContext.pCommandList, GetResource(), uploadContext.pResource, 0, 0, 1, &textureData);

				if (GetResourceState() != D3D12_RESOURCE_STATE_COMMON)
				{
					const D3D12_RESOURCE_BARRIER transition[] =
					{
						m_pImpl->Transition(D3D12_RESOURCE_STATE_COMMON),
					};
					uploadContext.pCommandList->ResourceBarrier(_countof(transition), transition);
				}

				pUploader->EndResourceUpload(uploadContext);

				return true;
			}

			bool Texture::Initialize(const D3D12_RESOURCE_DESC* pDesc)
			{
				SetState(State::eLoading);

				const bool isSuccess = m_pImpl->Initialize(pDesc);
				if (isSuccess == true)
				{
					SetState(State::eComplete);
				}

				return isSuccess;
			}

			bool Texture::Load(const wchar_t* filePath)
			{
				SetState(State::eLoading);

				const bool isSuccess = m_pImpl->Load(filePath);
				if (isSuccess == true)
				{
					SetState(State::eComplete);
				}

				return isSuccess;
			}

			bool Texture::Bind(ID3D12Resource* pResource, const D3D12_SHADER_RESOURCE_VIEW_DESC* pDesc)
			{
				SetState(State::eLoading);

				const bool isSuccess = m_pImpl->Bind(pResource, pDesc);
				if (isSuccess == true)
				{
					SetState(State::eComplete);
				}

				return isSuccess;
			}

			ID3D12Resource* Texture::GetResource() const
			{
				return m_pImpl->GetResource();
			}

			uint32_t Texture::GetDescriptorIndex() const
			{
				return m_pImpl->GetDescriptorIndex();
			}

			const D3D12_CPU_DESCRIPTOR_HANDLE& Texture::GetCPUHandle(uint32_t frameIndex) const
			{
				return m_pImpl->GetCPUHandle(frameIndex);
			}

			const D3D12_GPU_DESCRIPTOR_HANDLE& Texture::GetGPUHandle(uint32_t frameIndex) const
			{
				return m_pImpl->GetGPUHandle(frameIndex);
			}

			void Texture::Transition(D3D12_RESOURCE_STATES changeState, D3D12_RESOURCE_BARRIER* pBarrier_out)
			{
				*pBarrier_out = m_pImpl->Transition(changeState);
			}

			D3D12_RESOURCE_STATES Texture::GetResourceState() const
			{
				return m_pImpl->GetResourceState();
			}
		}
	}
}