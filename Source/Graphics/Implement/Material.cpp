#include "stdafx.h"
#include "Material.h"

#include "CommonLib/FileStream.h"
#include "Graphics.h"

namespace sid
{
	RegisterStringID(DefaultMaterial);
}

namespace est
{
	namespace graphics
	{
		Material::Material()
		{
		}

		Material::~Material()
		{
		}

		std::shared_ptr<Material> Material::Create(const IMaterial::Data* pMaterialData)
		{
			std::shared_ptr<Material> pMaterial = std::make_shared<Material>();
			pMaterial->SetState(IResource::eReady);
			if (pMaterialData == nullptr)
			{
				pMaterial->SetName(sid::DefaultMaterial);
			}
			else
			{
				pMaterial->m_data = *pMaterialData;
				pMaterial->LoadTexture();
			}

			pMaterial->SetState(IResource::eComplete);
			return pMaterial;
		}

		std::shared_ptr<Material> Material::Create(const wchar_t* fileName, const wchar_t* filePath)
		{
			std::wstring fullPath(filePath);
			fullPath.append(fileName);

			file::Stream file;
			if (file.Open(fullPath.c_str(), file::eReadBinary) == false)
			{
				LOG_WARNING(L"아 실패함");
				return nullptr;
			}

			BinaryReader binaryReader = file.GetBinaryReader();

			IMaterial::Data materialData;
			materialData.name = binaryReader.ReadString();
			materialData.path = filePath;
			materialData.colorAlbedo = binaryReader.Read<math::Color>();
			materialData.colorEmissive = binaryReader.Read < math::Color>();
			materialData.paddingRoughMetEmi = binaryReader.Read<math::float4>();
			materialData.surSpecTintAniso = binaryReader.Read<math::float4>();
			materialData.sheenTintClearcoatGloss = binaryReader.Read<math::float4>();

			materialData.isVisible = binaryReader;
			materialData.stippleTransparencyFactor = binaryReader;
			materialData.tessellationFactor = binaryReader;
			materialData.isAlbedoAlphaChannelMaskMap = binaryReader;

			for (int i = 0; i < IMaterial::TypeCount; ++i)
			{
				const string::StringID textureName = binaryReader.ReadString();
				if (textureName != sid::None)
				{
					materialData.textureNameArray[i] = textureName;
				}
			}

			int type = binaryReader;
			materialData.samplerState = static_cast<SamplerState::Type>(type);

			type = binaryReader;
			materialData.blendState = static_cast<BlendState::Type>(type);

			type = binaryReader;
			materialData.rasterizerState = static_cast<RasterizerState::Type>(type);

			type = binaryReader;
			materialData.depthStencilState = static_cast<DepthStencilState::Type>(type);

			file.Close();

			return Material::Create(&materialData);
		}

		std::shared_ptr<Material> Material::Clone(const Material* pSource)
		{
			if (pSource == nullptr)
				return nullptr;

			std::shared_ptr<Material> pMaterial = std::make_shared<Material>();
			pMaterial->SetState(IResource::eReady);
			pMaterial->m_data = pSource->m_data;

			pMaterial->LoadTexture();

			pMaterial->SetState(IResource::eComplete);
			return pMaterial;
		}

		bool Material::SaveFile(const wchar_t* filePath) const
		{
			std::wstring fullPath(filePath);
			fullPath.append(GetName().c_str());
			fullPath.append(L".emtl");

			file::Stream file;
			if (file.Open(fullPath.c_str(), file::eWriteBinary) == false)
			{
				LOG_WARNING(L"아 실패함");
				return false;
			}

			file << GetName().c_str();

			file.Write(&GetAlbedoColor().r, 4);
			file.Write(&GetEmissiveColor().r, 4);

			file.Write(&GetPaddingRoughMetEmi().x, 4);
			file.Write(&GetSurSpecTintAniso().x, 4);
			file.Write(&GetSheenTintClearcoatGloss().x, 4);

			file << IsVisible();
			file << GetStippleTransparencyFactor();
			file << GetTessellationFactor();
			file << IsAlbedoAlphaChannelMaskMap();

			for (int i = 0; i < IMaterial::TypeCount; ++i)
			{
				const string::StringID& name = GetTextureName(static_cast<IMaterial::Type>(i));
				if (name.empty() == true)
				{
					file << "None";
				}
				else
				{
					file << name.c_str();
				}
			}

			file << static_cast<int>(GetSamplerState());
			file << static_cast<int>(GetBlendState());
			file << static_cast<int>(GetRasterizerState());
			file << static_cast<int>(GetDepthStencilState());

			file.Close();

			return true;
		}

		void Material::LoadTexture()
		{
			for (uint32_t i = 0; i < IMaterial::TypeCount; ++i)
			{
				IMaterial::Type emType = static_cast<IMaterial::Type>(i);

				if (GetTextureName(emType).empty() == true)
				{
					SetTexture(emType, nullptr);
				}
				else
				{
					std::wstring strTexPath = GetPath();
					strTexPath.append(L"Texture\\");
					strTexPath.append(GetTextureName(emType).c_str());

					TexturePtr pTexture;
					if (m_data.isAsyncTextureLoad == true)
					{
						pTexture = CreateTextureAsync(strTexPath.c_str());
					}
					else
					{
						pTexture = CreateTexture(strTexPath.c_str());
					}

					SetTexture(emType, pTexture);
				}
			}
		}

		void Material::SetTexture(IMaterial::Type emType, const TexturePtr& pTexture)
		{
			m_pTextures[emType] = pTexture;
		}

		bool Material::IsLoadComplete() const
		{
			for (auto& pTexture : m_pTextures)
			{
				if (pTexture != nullptr && pTexture->GetState() != IResource::eComplete)
					return false;
			}

			return true;
		}
	}
}