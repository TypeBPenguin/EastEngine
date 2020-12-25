#include "stdafx.h"
#include "TextureDX11.h"

#include "CommonLib/FileUtil.h"

#include "DeviceDX11.h"

#include "UtilDX11.h"

namespace est
{
	namespace graphics
	{
		namespace dx11
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
				SafeRelease(m_pShaderResourceView);
				SafeRelease(m_pTexture2D);
			}

			bool Texture::Map(MappedSubResourceData& mappedSubResourceData)
			{
				ID3D11DeviceContext* pDeviceContext = Device::GetInstance()->GetImmediateContext();
				return Map(pDeviceContext, mappedSubResourceData);
			}

			void Texture::Unmap()
			{
				ID3D11DeviceContext* pDeviceContext = Device::GetInstance()->GetImmediateContext();
				Unmap(pDeviceContext);
			}

			bool Texture::Map(ID3D11DeviceContext* pDeviceContext, MappedSubResourceData& mappedSubResourceData)
			{
				HRESULT hr = pDeviceContext->Map(m_pTexture2D, 0, D3D11_MAP_WRITE_DISCARD, 0, reinterpret_cast<D3D11_MAPPED_SUBRESOURCE*>(&mappedSubResourceData));
				if (FAILED(hr))
				{
					mappedSubResourceData = {};
					return false;
				}
				return true;
			}

			void Texture::Unmap(ID3D11DeviceContext* pDeviceContext)
			{
				pDeviceContext->Unmap(m_pTexture2D, 0);
			}

			bool Texture::Initialize(const TextureDesc& desc)
			{
				D3D11_TEXTURE2D_DESC tex_desc;
				tex_desc.Width = desc.Width;
				tex_desc.Height = desc.Height;
				tex_desc.MipLevels = 1;
				tex_desc.ArraySize = 1;
				tex_desc.Format = static_cast<DXGI_FORMAT>(desc.resourceFormat);
				tex_desc.SampleDesc.Count = 1;
				tex_desc.SampleDesc.Quality = 0;
				tex_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
				tex_desc.MiscFlags = 0;
				
				if (desc.isDynamic == true)
				{
					tex_desc.Usage = D3D11_USAGE_DYNAMIC;
					tex_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
				}
				else
				{
					tex_desc.Usage = D3D11_USAGE_DEFAULT;
					tex_desc.CPUAccessFlags = 0;
				}

				if (desc.subResourceData.pSysMem == nullptr)
				{
					return Initialize(&tex_desc, nullptr);
				}
				else
				{
					D3D11_SUBRESOURCE_DATA subresource_data;
					subresource_data.pSysMem = desc.subResourceData.pSysMem;
					subresource_data.SysMemPitch = desc.subResourceData.SysMemPitch;
					subresource_data.SysMemSlicePitch = desc.subResourceData.SysMemSlicePitch;
					return Initialize(&tex_desc, &subresource_data);
				}
			}

			bool Texture::Initialize(const D3D11_TEXTURE2D_DESC* pDesc, const D3D11_SUBRESOURCE_DATA* pInitialData)
			{
				const string::StringID textureName(m_key);
				const std::string debugName = string::WideToMulti(textureName.c_str());

				SetState(State::eLoading);

				ID3D11Device* pDevice = Device::GetInstance()->GetInterface();

				HRESULT hr = pDevice->CreateTexture2D(pDesc, pInitialData, &m_pTexture2D);
				if (FAILED(hr))
				{
					const std::string error = string::Format("failed to create texture 2d : %s", textureName.c_str());
					throw_line(error.c_str());
				}

				util::SetDebugName(m_pTexture2D, debugName.c_str());

				if (pDesc->BindFlags & D3D11_BIND_SHADER_RESOURCE)
				{
					D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = CD3D11_SHADER_RESOURCE_VIEW_DESC(pDesc->ArraySize == 1 ? D3D11_SRV_DIMENSION_TEXTURE2D : D3D11_SRV_DIMENSION_TEXTURE2DARRAY, pDesc->Format, 0, pDesc->MipLevels, 0, pDesc->ArraySize, 0);
					if (FAILED(pDevice->CreateShaderResourceView(m_pTexture2D, &srvDesc, &m_pShaderResourceView)))
					{
						const std::string error = string::Format("failed to create shader resource view : %s", textureName.c_str());
						throw_line(error.c_str());
					}

					util::SetDebugName(m_pShaderResourceView, debugName.c_str());
				}

				m_size.x = static_cast<uint32_t>(pDesc->Width);
				m_size.y = static_cast<uint32_t>(pDesc->Height);

				SetState(State::eComplete);

				return true;
			}

			bool Texture::Load(const wchar_t* filePath)
			{
				TRACER_EVENT(L"TextureLoad");
				TRACER_PUSHARGS(L"FilePath", filePath);

				CoInitializer coInitializer;

				SetState(State::eLoading);

				m_path = filePath;

				HRESULT hr{ S_OK };
				DirectX::ScratchImage image{};

				ID3D11Device* pDevice = Device::GetInstance()->GetInterface();

				const std::wstring strFileExtension(file::GetFileExtension(filePath));
				if (string::IsEqualsNoCase(strFileExtension.c_str(), L".dds"))
				{
					hr = DirectX::LoadFromDDSFile(filePath, DirectX::DDS_FLAGS_NONE, nullptr, image);
				}
				else if (string::IsEqualsNoCase(strFileExtension.c_str(), L".tga"))
				{
					hr = DirectX::LoadFromTGAFile(filePath, nullptr, image);
				}
				else
				{
					hr = DirectX::LoadFromWICFile(filePath, DirectX::WIC_FLAGS_NONE, nullptr, image);
				}

				if (image.GetMetadata().dimension == DirectX::TEX_DIMENSION_TEXTURE3D)
				{
					SetState(State::eInvalid);
					LOG_WARNING(L"not support this dimension[texture3D] : %s", filePath);
					return false;
				}

				if (FAILED(hr))
				{
					SetState(State::eInvalid);
					LOG_WARNING(L"Can't found Texture File : %s", filePath);
					return false;
				}

				if (image.GetMetadata().mipLevels == 1)
				{
					DirectX::ScratchImage mipChain;
					hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_DEFAULT, 0, mipChain);
					if (SUCCEEDED(hr))
					{
						image = std::move(mipChain);
					}
					else
					{
						LOG_ERROR(L"Can't GenerateMipMaps : %s", filePath);
					}
				}

				hr = DirectX::CreateTexture(pDevice, image.GetImages(), image.GetImageCount(), image.GetMetadata(), reinterpret_cast<ID3D11Resource**>(&m_pTexture2D));
				if (FAILED(hr))
				{
					SetState(State::eInvalid);
					LOG_ERROR(L"faield to Create CreateTexture : %s", filePath);
					return false;
				}

				const std::string debugName = string::WideToMulti(file::GetFileName(filePath));
				util::SetDebugName(m_pTexture2D, debugName);

				const DirectX::TexMetadata& metadata = image.GetMetadata();

				D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc{};
				SRVDesc.Format = metadata.format;

				switch (metadata.dimension)
				{
				case DirectX::TEX_DIMENSION_TEXTURE1D:
					if (metadata.arraySize > 1)
					{
						SRVDesc.ViewDimension = D3D_SRV_DIMENSION_TEXTURE1DARRAY;
						SRVDesc.Texture1DArray.MipLevels = static_cast<UINT>(metadata.mipLevels);
						SRVDesc.Texture1DArray.ArraySize = static_cast<UINT>(metadata.arraySize);
					}
					else
					{
						SRVDesc.ViewDimension = D3D_SRV_DIMENSION_TEXTURE1D;
						SRVDesc.Texture1D.MipLevels = static_cast<UINT>(metadata.mipLevels);
					}
					break;
				case DirectX::TEX_DIMENSION_TEXTURE2D:
					if (metadata.IsCubemap())
					{
						if (metadata.arraySize > 6)
						{
							assert((metadata.arraySize % 6) == 0);
							SRVDesc.ViewDimension = D3D_SRV_DIMENSION_TEXTURECUBEARRAY;
							SRVDesc.TextureCubeArray.MipLevels = static_cast<UINT>(metadata.mipLevels);
							SRVDesc.TextureCubeArray.NumCubes = static_cast<UINT>(metadata.arraySize / 6);
						}
						else
						{
							SRVDesc.ViewDimension = D3D_SRV_DIMENSION_TEXTURECUBE;
							SRVDesc.TextureCube.MipLevels = static_cast<UINT>(metadata.mipLevels);
						}
					}
					else if (metadata.arraySize > 1)
					{
						SRVDesc.ViewDimension = D3D_SRV_DIMENSION_TEXTURE2DARRAY;
						SRVDesc.Texture2DArray.MipLevels = static_cast<UINT>(metadata.mipLevels);
						SRVDesc.Texture2DArray.ArraySize = static_cast<UINT>(metadata.arraySize);
					}
					else
					{
						SRVDesc.ViewDimension = D3D_SRV_DIMENSION_TEXTURE2D;
						SRVDesc.Texture2D.MipLevels = static_cast<UINT>(metadata.mipLevels);
					}
					break;
				case DirectX::TEX_DIMENSION_TEXTURE3D:
					assert(metadata.arraySize == 1);
					SRVDesc.ViewDimension = D3D_SRV_DIMENSION_TEXTURE3D;
					SRVDesc.Texture3D.MipLevels = static_cast<UINT>(metadata.mipLevels);
					break;
				default:
					return false;
				}

				hr = pDevice->CreateShaderResourceView(m_pTexture2D, &SRVDesc, &m_pShaderResourceView);
				if (FAILED(hr))
				{
					SetState(State::eInvalid);
					LOG_ERROR(L"Can't CreateShaderResourceView : %s", filePath);
					return false;
				}

				util::SetDebugName(m_pShaderResourceView, debugName);

				m_size.x = static_cast<uint32_t>(metadata.width);
				m_size.y = static_cast<uint32_t>(metadata.height);

				SetState(State::eComplete);

				return true;
			}
		}
	}
}