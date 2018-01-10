#include "stdafx.h"
#include "Material.h"

#include "CommonLib/FileStream.h"

#include "D3DInterface.h"

namespace StrID
{
	RegisterStringID(DefaultMaterial);
}

namespace EastEngine
{
	namespace Graphics
	{
		Material::Material()
			: m_nReferenceCount(0)
		{
			m_pTextureArray.fill(nullptr);
		}

		Material::~Material()
		{
			m_pTextureArray.fill(nullptr);
		}

		bool Material::Init(const MaterialInfo* pInfo)
		{
			if (pInfo == nullptr)
			{
				SetName(StrID::DefaultMaterial);
				return true;
			}

			SetName(pInfo->strName);
			SetPath(pInfo->strPath);

			SetAlbedoColor(pInfo->colorAlbedo);
			SetEmissiveColor(pInfo->colorEmissive);

			for (uint32_t i = 0; i < EmMaterial::TypeCount; ++i)
			{
				EmMaterial::Type emType = static_cast<EmMaterial::Type>(i);
				SetTextureName(emType, pInfo->strTextureNameArray[i]);
			}

			SetDisRoughMetEmi(pInfo->f4DisRoughMetEmi);
			SetSurSpecTintAniso(pInfo->f4SurSpecTintAniso);
			SetSheenTintClearcoatGloss(pInfo->f4SheenTintClearcoatGloss);

			SetSamplerState(pInfo->emSamplerState);
			SetBlendState(pInfo->emBlendState);
			SetRasterizerState(pInfo->emRasterizerState);
			SetDepthStencilState(pInfo->emDepthStencilState);

			LoadTexture();

			return true;
		}

		bool Material::Init(const String::StringID& strName)
		{
			MaterialInfo defaultInfo;
			defaultInfo.strName = strName;

			return Init(&defaultInfo);
		}

		bool Material::Init(const IMaterial* pSource)
		{
			if (pSource == nullptr)
				return false;

			SetName(pSource->GetName());
			SetPath(pSource->GetPath());

			SetAlbedoColor(pSource->GetAlbedoColor());
			SetEmissiveColor(pSource->GetEmissiveColor());

			for (uint32_t i = 0; i < EmMaterial::TypeCount; ++i)
			{
				EmMaterial::Type emType = static_cast<EmMaterial::Type>(i);
				SetTextureName(emType, pSource->GetTextureName(emType));
			}

			SetDisRoughMetEmi(pSource->GetDisRoughMetEmi());
			SetSurSpecTintAniso(pSource->GetSurSpecTintAniso());
			SetSheenTintClearcoatGloss(pSource->GetSheenTintClearcoatGloss());

			SetSamplerState(pSource->GetSamplerState());
			SetBlendState(pSource->GetBlendState());
			SetRasterizerState(pSource->GetRasterizerState());
			SetDepthStencilState(pSource->GetDepthStencilState());

			LoadTexture();

			return true;
		}

		void Material::LoadTexture()
		{
			for (uint32_t i = 0; i < EmMaterial::TypeCount; ++i)
			{
				EmMaterial::Type emType = static_cast<EmMaterial::Type>(i);

				std::string strTexPath = GetPath();
				strTexPath.append(GetTextureName(emType).c_str());

				if (GetTextureName(emType).empty() == true)
				{
					this->SetTexture(emType, nullptr);
				}
				else
				{
					std::shared_ptr<ITexture> pTexture = ITexture::Create(GetTextureName(emType), strTexPath);

					this->SetTexture(emType, pTexture);
				}
			}
		}

		bool Material::IsLoadComplete() const
		{
			for (auto& pTexture : m_pTextureArray)
			{
				if (pTexture != nullptr && pTexture->GetLoadState() != EmLoadState::eComplete)
					return false;
			}

			return true;
		}
	}
}