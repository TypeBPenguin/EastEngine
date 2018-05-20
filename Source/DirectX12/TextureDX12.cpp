#include "stdafx.h"
#include "TextureDX12.h"

#include "CommonLib/FileUtil.h"

#include "UtilDX12.h"
#include "DeviceDX12.h"
#include "DescriptorHeapDX12.h"
#include "UploadDX12.h"

namespace eastengine
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

			Texture::Texture(const ITexture::Key& key)
				: m_key(key)
			{
			}

			Texture::~Texture()
			{
				SafeRelease(m_pResource);

				DescriptorHeap* pDescriptorHeap = Device::GetInstance()->GetSRVDescriptorHeap();
				pDescriptorHeap->FreePersistent(m_nDescriptorIndex);
			}

			bool Texture::Initialize(const D3D12_RESOURCE_DESC* pDesc, bool isDynamic)
			{
				String::StringID strName(m_key.value);

				ID3D12Device* pDevice = Device::GetInstance()->GetInterface();

				ID3D12Resource* pResource = nullptr;

				CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_DEFAULT, 0, 0);
				if (isDynamic == true)
				{
					heapProperties.Type = D3D12_HEAP_TYPE_READBACK;
					heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
					heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;
				}

				HRESULT hr = pDevice->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, pDesc,
					D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&pResource));
				if (FAILED(hr))
				{
					LOG_ERROR("failed to create resource : %s", strName.c_str());
					return false;
				}

				std::wstring wstrName = String::MultiToWide(strName.c_str());
				pResource->SetName(wstrName.c_str());

				const uint64_t numSubResources = pDesc->MipLevels * pDesc->DepthOrArraySize;
				D3D12_PLACED_SUBRESOURCE_FOOTPRINT* layouts = static_cast<D3D12_PLACED_SUBRESOURCE_FOOTPRINT*>(_alloca(sizeof(D3D12_PLACED_SUBRESOURCE_FOOTPRINT) * numSubResources));
				uint32_t* numRows = static_cast<uint32_t*>(_alloca(sizeof(uint32_t) * numSubResources));
				uint64_t* rowSizes = static_cast<uint64_t*>(_alloca(sizeof(uint64_t) * numSubResources));

				uint64_t textureMemSize = 0;
				pDevice->GetCopyableFootprints(pDesc, 0, uint32_t(numSubResources), 0, layouts, numRows, rowSizes, &textureMemSize);

				Uploader* pUploader = Device::GetInstance()->GetUploader();

				// Get a GPU upload buffer
				UploadContext uploadContext = pUploader->BeginResourceUpload(textureMemSize);
				uint8_t* uploadMem = reinterpret_cast<uint8_t*>(uploadContext.pCPUAddress);

				for (uint64_t arrayIdx = 0; arrayIdx < pDesc->DepthOrArraySize; ++arrayIdx)
				{
					for (uint64_t mipIdx = 0; mipIdx < pDesc->MipLevels; ++mipIdx)
					{
						const uint64_t subResourceIdx = mipIdx + (arrayIdx * pDesc->MipLevels);

						const D3D12_PLACED_SUBRESOURCE_FOOTPRINT& subResourceLayout = layouts[subResourceIdx];
						const uint64_t subResourceHeight = numRows[subResourceIdx];
						const uint64_t subResourcePitch = subResourceLayout.Footprint.RowPitch;
						const uint64_t subResourceDepth = subResourceLayout.Footprint.Depth;
						uint8_t* dstSubResourceMem = reinterpret_cast<uint8_t*>(uploadMem) + subResourceLayout.Offset;

						for (uint64_t z = 0; z < subResourceDepth; ++z)
						{
							for (uint64_t y = 0; y < subResourceHeight; ++y)
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

				SetAlive(true);
				SetState(State::eComplete);

				return true;
			}

			bool Texture::Load(const char* strFilePath)
			{
				TRACER_EVENT("TextureLoad");
				TRACER_PUSHARGS("FilePath", strFilePath);

				CoInitializer coInitializer;

				SetState(State::eLoading);

				m_strPath = strFilePath;

				HRESULT hr{ S_OK };
				DirectX::ScratchImage image{};

				ID3D12Device* pDevice = Device::GetInstance()->GetInterface();

				std::string strFileExtension(file::GetFileExtension(strFilePath));

				if (String::IsEqualsNoCase(strFileExtension.c_str(), "dds"))
				{
					hr = DirectX::LoadFromDDSFile(String::MultiToWide(strFilePath).c_str(), DirectX::DDS_FLAGS_NONE, nullptr, image);
				}
				else if (String::IsEqualsNoCase(strFileExtension.c_str(), "tga"))
				{
					DirectX::ScratchImage tempImage;
					hr = DirectX::LoadFromTGAFile(String::MultiToWide(strFilePath).c_str(), nullptr, tempImage);
					hr = DirectX::GenerateMipMaps(*tempImage.GetImage(0, 0, 0), DirectX::TEX_FILTER_DEFAULT, 0, image, false);
				}
				else
				{
					DirectX::ScratchImage tempImage;
					hr = DirectX::LoadFromWICFile(String::MultiToWide(strFilePath).c_str(), DirectX::WIC_FLAGS_NONE, nullptr, tempImage);
					hr = DirectX::GenerateMipMaps(*tempImage.GetImage(0, 0, 0), DirectX::TEX_FILTER_DEFAULT, 0, image, false);
				}

				if (FAILED(hr))
				{
					LOG_ERROR("failed to load texture : %s", strFilePath);
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
					LOG_ERROR("failed to create resource : %s", strFilePath);
					return false;
				}

				std::wstring wstrName = String::MultiToWide(file::GetFileName(strFilePath));
				m_pResource->SetName(wstrName.c_str());

				DescriptorHeap* pDescriptorHeap = Device::GetInstance()->GetSRVDescriptorHeap();
				PersistentDescriptorAlloc srvAlloc = pDescriptorHeap->AllocatePersistent();
				m_nDescriptorIndex = srvAlloc.nIndex;

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

				for (uint32_t i = 0; i < pDescriptorHeap->GetHeapCount(); ++i)
				{
					pDevice->CreateShaderResourceView(m_pResource, &srvDesc, srvAlloc.cpuHandles[i]);
				}

				const uint64_t numSubResources = metaData.mipLevels * metaData.arraySize;
				D3D12_PLACED_SUBRESOURCE_FOOTPRINT* layouts = static_cast<D3D12_PLACED_SUBRESOURCE_FOOTPRINT*>(_alloca(sizeof(D3D12_PLACED_SUBRESOURCE_FOOTPRINT) * numSubResources));
				uint32_t* numRows = static_cast<uint32_t*>(_alloca(sizeof(uint32_t) * numSubResources));
				uint64_t* rowSizes = static_cast<uint64_t*>(_alloca(sizeof(uint64_t) * numSubResources));

				uint64_t textureMemSize = 0;
				pDevice->GetCopyableFootprints(&textureDesc, 0, uint32_t(numSubResources), 0, layouts, numRows, rowSizes, &textureMemSize);

				Uploader* pUploader = Device::GetInstance()->GetUploader();

				// Get a GPU upload buffer
				UploadContext uploadContext = pUploader->BeginResourceUpload(textureMemSize);
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

				SetAlive(true);
				SetState(State::eComplete);

				return true;
			}

			bool Texture::Bind(ID3D12Resource* pResource, const D3D12_SHADER_RESOURCE_VIEW_DESC* pDesc)
			{
				m_pResource = pResource;

				ID3D12Device* pDevice = Device::GetInstance()->GetInterface();

				DescriptorHeap* pDescriptorHeap = Device::GetInstance()->GetSRVDescriptorHeap();
				PersistentDescriptorAlloc srvAlloc = pDescriptorHeap->AllocatePersistent();
				m_nDescriptorIndex = srvAlloc.nIndex;

				D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
				if (pDesc == nullptr)
				{
					pDesc = &srvDesc;

					D3D12_RESOURCE_DESC desc = pResource->GetDesc();
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

				for (uint32_t i = 0; i < pDescriptorHeap->GetHeapCount(); ++i)
				{
					pDevice->CreateShaderResourceView(m_pResource, pDesc, srvAlloc.cpuHandles[i]);
				}

				return true;
			}

			D3D12_CPU_DESCRIPTOR_HANDLE Texture::GetCPUHandle(int nFrameIndex) const
			{
				DescriptorHeap* pDescriptorHeap = Device::GetInstance()->GetSRVDescriptorHeap();
				return pDescriptorHeap->GetCPUHandleFromIndex(m_nDescriptorIndex, nFrameIndex);
			}

			D3D12_GPU_DESCRIPTOR_HANDLE Texture::GetGPUHandle(int nFrameIndex) const
			{
				DescriptorHeap* pDescriptorHeap = Device::GetInstance()->GetSRVDescriptorHeap();
				return pDescriptorHeap->GetGPUHandleFromIndex(m_nDescriptorIndex, nFrameIndex);
			}
		}
	}
}