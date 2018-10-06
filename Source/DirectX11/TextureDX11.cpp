#include "stdafx.h"
#include "TextureDX11.h"

#include "CommonLib/FileUtil.h"

#include "DeviceDX11.h"

#include "UtilDX11.h"

namespace eastengine
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

			bool Texture::Initialize(const TextureDesc& desc)
			{
				D3D11_SUBRESOURCE_DATA subresource_data;
				subresource_data.pSysMem = desc.subResourceData.pSysMem;
				subresource_data.SysMemPitch = desc.subResourceData.SysMemPitch;
				subresource_data.SysMemSlicePitch = desc.subResourceData.SysMemSlicePitch;

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

				return Initialize(&tex_desc, &subresource_data);
			}

			bool Texture::Initialize(const D3D11_TEXTURE2D_DESC* pDesc, const D3D11_SUBRESOURCE_DATA* pInitialData)
			{
				const String::StringID strTextureName(m_key.value);

				SetState(State::eLoading);

				ID3D11Device* pDevice = Device::GetInstance()->GetInterface();

				HRESULT hr = pDevice->CreateTexture2D(pDesc, pInitialData, &m_pTexture2D);
				if (FAILED(hr))
				{
					const std::string error = String::Format("failed to create texture 2d : %s", strTextureName.c_str());
					throw_line(error.c_str());
				}

				util::SetDebugName(m_pTexture2D, strTextureName.c_str());

				if (pDesc->BindFlags & D3D11_BIND_SHADER_RESOURCE)
				{
					D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = CD3D11_SHADER_RESOURCE_VIEW_DESC(pDesc->ArraySize == 1 ? D3D11_SRV_DIMENSION_TEXTURE2D : D3D11_SRV_DIMENSION_TEXTURE2DARRAY, pDesc->Format, 0, pDesc->MipLevels, 0, pDesc->ArraySize, 0);
					if (FAILED(pDevice->CreateShaderResourceView(m_pTexture2D, &srvDesc, &m_pShaderResourceView)))
					{
						const std::string error = String::Format("failed to create shader resource view : %s", strTextureName.c_str());
						throw_line(error.c_str());
					}

					util::SetDebugName(m_pShaderResourceView, strTextureName.c_str());
				}

				m_n2Size.x = static_cast<uint32_t>(pDesc->Width);
				m_n2Size.y = static_cast<uint32_t>(pDesc->Height);

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

				ID3D11Device* pDevice = Device::GetInstance()->GetInterface();

				std::string strFileExtension(file::GetFileExtension(strFilePath));

				if (String::IsEqualsNoCase(strFileExtension.c_str(), "dds"))
				{
					hr = DirectX::LoadFromDDSFile(String::MultiToWide(strFilePath).c_str(), DirectX::DDS_FLAGS_NONE, nullptr, image);
				}
				else if (String::IsEqualsNoCase(strFileExtension.c_str(), "tga"))
				{
					hr = DirectX::LoadFromTGAFile(String::MultiToWide(strFilePath).c_str(), nullptr, image);
				}
				else
				{
					hr = DirectX::LoadFromWICFile(String::MultiToWide(strFilePath).c_str(), DirectX::WIC_FLAGS_NONE, nullptr, image);
				}

				if (image.GetMetadata().dimension == DirectX::TEX_DIMENSION_TEXTURE3D)
				{
					SetState(State::eInvalid);
					LOG_WARNING("not support this dimension[texture3D] : %s", strFilePath);
					return false;
				}

				if (FAILED(hr))
				{
					SetState(State::eInvalid);
					LOG_WARNING("Can't found Texture File : %s", strFilePath);
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
						LOG_ERROR("Can't GenerateMipMaps : %s", strFilePath);
					}
				}

				hr = DirectX::CreateTexture(pDevice, image.GetImages(), image.GetImageCount(), image.GetMetadata(), reinterpret_cast<ID3D11Resource**>(&m_pTexture2D));
				if (FAILED(hr))
				{
					SetState(State::eInvalid);
					LOG_ERROR("faield to Create CreateTexture : %s", strFilePath);
					return false;
				}

				std::string strName = file::GetFileName(strFilePath);
				util::SetDebugName(m_pTexture2D, strName);

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
					LOG_ERROR("Can't CreateShaderResourceView : %s", strFilePath);
					return false;
				}

				util::SetDebugName(m_pShaderResourceView, strName);

				m_n2Size.x = static_cast<uint32_t>(metadata.width);
				m_n2Size.y = static_cast<uint32_t>(metadata.height);

				SetState(State::eComplete);
				SetAlive(true);

				return true;
			}
		}
	}
}