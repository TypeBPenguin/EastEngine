#include "stdafx.h"
#include "Material.h"

#include "CommonLib/FileStream.h"
#include "Graphics.h"

namespace StrID
{
	RegisterStringID(DefaultMaterial);
}

namespace eastengine
{
	namespace graphics
	{
		Material::Material()
		{
		}

		Material::~Material()
		{
			std::for_each(m_pTextureArray.begin(), m_pTextureArray.end(), [](ITexture* pTexture)
			{
				if (pTexture != nullptr)
				{
					pTexture->DecreaseReference();
				}
			});
			m_pTextureArray.fill(nullptr);
		}

		std::unique_ptr<Material> Material::Create(const MaterialInfo* pInfo)
		{
			std::unique_ptr<Material> pMaterial = std::make_unique<Material>();
			pMaterial->SetState(IResource::eReady);
			if (pInfo == nullptr)
			{
				pMaterial->SetName(StrID::DefaultMaterial);
			}
			else
			{
				pMaterial->m_info = *pInfo;
				pMaterial->LoadTexture();
			}

			pMaterial->SetState(IResource::eComplete);
			pMaterial->SetAlive(true);
			return pMaterial;
		}

		std::unique_ptr<Material> Material::Create(const char* strFileName, const char* strFilePath)
		{
			std::string strFullPath(strFilePath);
			strFullPath.append(strFileName);

			file::Stream file;
			if (file.Open(strFullPath.c_str(), file::eRead | file::eBinary) == false)
			{
				LOG_WARNING("아 실패함");
				return nullptr;
			}

			MaterialInfo materialInfo;

			std::string strBuf;
			file >> strBuf;

			materialInfo.strName = strBuf.c_str();
			materialInfo.strPath = strFilePath;

			file.Read(&materialInfo.colorAlbedo.r, 4);
			file.Read(&materialInfo.colorEmissive.r, 4);

			file.Read(&materialInfo.f4PaddingRoughMetEmi.x, 4);
			file.Read(&materialInfo.f4SurSpecTintAniso.x, 4);
			file.Read(&materialInfo.f4SheenTintClearcoatGloss.x, 4);

			file >> materialInfo.isVisible;
			file >> materialInfo.fStippleTransparencyFactor;
			file >> materialInfo.fTessellationFactor;
			file >> materialInfo.isAlbedoAlphaChannelMaskMap;

			for (int i = 0; i < EmMaterial::TypeCount; ++i)
			{
				file >> strBuf;

				if (strBuf != "None")
				{
					materialInfo.strTextureNameArray[i] = strBuf.c_str();
				}
			}

			int nValue = 0;
			file >> nValue;
			materialInfo.emSamplerState = static_cast<EmSamplerState::Type>(nValue);

			file >> nValue;
			materialInfo.emBlendState = static_cast<EmBlendState::Type>(nValue);

			file >> nValue;
			materialInfo.emRasterizerState = static_cast<EmRasterizerState::Type>(nValue);

			file >> nValue;
			materialInfo.emDepthStencilState = static_cast<EmDepthStencilState::Type>(nValue);

			file.Close();

			return Material::Create(&materialInfo);
		}

		std::unique_ptr<Material> Material::Clone(const Material* pSource)
		{
			if (pSource == nullptr)
				return nullptr;

			std::unique_ptr<Material> pMaterial = std::make_unique<Material>();
			pMaterial->SetState(IResource::eReady);
			pMaterial->m_info = pSource->m_info;

			pMaterial->LoadTexture();

			pMaterial->SetState(IResource::eComplete);
			pMaterial->SetAlive(true);
			return pMaterial;
		}

		bool Material::SaveToFile(const char* strFilePath) const
		{
			std::string strFullPath(strFilePath);
			strFullPath.append(GetName().c_str());
			strFullPath.append(".emtl");

			file::Stream file;
			if (file.Open(strFullPath.c_str(), file::eWrite | file::eBinary) == false)
			{
				LOG_WARNING("아 실패함");
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

			for (int i = 0; i < EmMaterial::TypeCount; ++i)
			{
				const String::StringID& strName = GetTextureName(static_cast<EmMaterial::Type>(i));
				if (strName.empty() == true)
				{
					file << "None";
				}
				else
				{
					file << strName.c_str();
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
			for (uint32_t i = 0; i < EmMaterial::TypeCount; ++i)
			{
				EmMaterial::Type emType = static_cast<EmMaterial::Type>(i);

				if (GetTextureName(emType).empty() == true)
				{
					SetTexture(emType, nullptr);
				}
				else
				{
					std::string strTexPath = GetPath();
					strTexPath.append("Texture\\");
					strTexPath.append(GetTextureName(emType).c_str());

					ITexture* pTexture = nullptr;
					if (m_info.isAsyncTextureLoad == true)
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

		void Material::SetTexture(EmMaterial::Type emType, ITexture* pTexture)
		{
			if (m_pTextureArray[emType] != nullptr)
			{
				m_pTextureArray[emType]->DecreaseReference();
			}

			m_pTextureArray[emType] = pTexture;

			if (m_pTextureArray[emType] != nullptr)
			{
				m_pTextureArray[emType]->IncreaseReference();
			}
		}

		bool Material::IsLoadComplete() const
		{
			for (auto& pTexture : m_pTextureArray)
			{
				if (pTexture != nullptr && pTexture->GetState() != IResource::eComplete)
					return false;
			}

			return true;
		}
	}
}