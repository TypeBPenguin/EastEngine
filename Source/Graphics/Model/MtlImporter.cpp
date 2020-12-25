#include "stdafx.h"
#include "MtlImporter.h"

#include "CommonLib/FileStream.h"
#include "CommonLib/FileUtil.h"

namespace est
{
	namespace graphics
	{
		MtlImporter::MtlImporter()
		{
		}

		MtlImporter::~MtlImporter()
		{
			Release();
		}

		bool MtlImporter::Init(const wchar_t* fileName, const wchar_t* path)
		{
			std::wstring filePath = path;
			filePath.append(fileName);

			file::Stream file;
			if (file.Open(filePath.c_str()) == false)
				return false;

			const std::wstring texturePath = file::GetFilePath(file.GetFilePath());

			MaterialPtr pMaterial = nullptr;

			std::string temp;
			while (file.Eof() == false)
			{
				file >> temp;

				if (temp == "#")
				{
					file.ReadLine(temp);
					continue;
				}

				if (temp == "newmtl")
				{
					std::string newMtlName;
					file.ReadLine(newMtlName);

					if (newMtlName.empty())
						continue;

					newMtlName = newMtlName.substr(1, newMtlName.length());

					IMaterial::Data data;
					data.name = newMtlName.c_str();
					auto iter = m_umapNewMtrl.emplace(string::StringID(newMtlName.c_str()), CreateMaterial(&data));
					pMaterial = iter.first->second;
				}
				else if (temp == "Ka")
				{
					assert(pMaterial);

					math::Color colorAmbient = pMaterial->GetAlbedoColor();
					file.Read(&colorAmbient.r, 3);

					pMaterial->SetAlbedoColor(colorAmbient);
				}
				else if (temp == "Kd")
				{
					assert(pMaterial);

					math::Color colorDiffuse = pMaterial->GetAlbedoColor();
					file.Read(&colorDiffuse.r, 3);

					pMaterial->SetAlbedoColor(colorDiffuse);
				}
				else if (temp == "Ks")
				{
					assert(pMaterial);

					math::Color colorSpecular;
					file.Read(&colorSpecular.r, 3);
				}
				else if (temp == "Ns")
				{
					assert(pMaterial);

					float fSpecularPower;
					file >> fSpecularPower;

					pMaterial->SetSpecular(fSpecularPower);
				}
				else if (temp == "d")
				{
					assert(pMaterial);

					math::Color colorDiffuse = pMaterial->GetAlbedoColor();
					file >> colorDiffuse.a;
					pMaterial->SetAlbedoColor(colorDiffuse);
				}
				else if (temp == "Tr")
				{
					assert(pMaterial);

					math::Color colorDiffuse = pMaterial->GetAlbedoColor();
					file >> colorDiffuse.a;
					colorDiffuse.a = (1.f - colorDiffuse.a);
					pMaterial->SetAlbedoColor(colorDiffuse);
				}
				else if (temp == "Tf")
				{
					assert(pMaterial);

					// 필터링?
				}
				else if (temp == "illum")
				{
					assert(pMaterial);

					int n = 0;
					file >> n;

					// http://paulbourke.net/dataformats/mtl/
					// 여기 제일 마지막 줄에 나와있는 대로 셰이더 코드 작성 하면 됨

					switch (n)
					{
					case 0:
						// Color on and Ambient off
						break;
					case 1:
						// Color on and Ambient on
						break;
					case 2:
						// Highlight on
						break;
					case 3:
						// Reflection on and Ray trace on
						break;
					case 4:
						// Transparency : Glass on
						// Reflection : Ray trace on
						break;
					case 5:
						// Reflection : Fresnel on and Ray trace on
						break;
					case 6:
						// Transparency : Refraction on
						// Reflection : Fresnel off and Ray trace on
						break;
					case 7:
						// Transparency : Refraction on
						// Reflection : Fresnel on and Ray trace on
						break;
					case 8:
						// Reflection on and Ray trace off
						break;
					case 9:
						// Transparency : Glass on
						// Reflection : Ray trace off
						break;
					case 10:
						// Casts shadows onto invisible surfaces
						break;
					}
				}
				else if (temp == "sharpness")
				{
					assert(pMaterial);

					// Range 0 ~ 1000
					// Default 60
				}
				else if (temp == "Ni")
				{
					assert(pMaterial);

					// 굴절률
				}
				else if (temp == "map_Ka")
				{
					assert(pMaterial);

					// ambient Texture
				}
				else if (temp == "map_Kd")
				{
					assert(pMaterial);

					// diffuse Texture
					std::string strTextureName;
					file.ReadLine(strTextureName);

					if (strTextureName.empty())
						continue;

					pMaterial->SetTextureName(IMaterial::eAlbedo, strTextureName.substr(1, strTextureName.length()).c_str());
				}
				else if (temp == "map_Ks")
				{
					assert(pMaterial);

					// specular Texture
					std::string strTextureName;
					file.ReadLine(strTextureName);

					if (strTextureName.empty())
						continue;
				}
				else if (temp == "map_Ns")
				{
					assert(pMaterial);

					// specular Highlight
					std::string strTextureName;
					file.ReadLine(strTextureName);

					if (strTextureName.empty())
						continue;

					pMaterial->SetTextureName(IMaterial::eSpecular, strTextureName.substr(1, strTextureName.length()).c_str());
				}
				else if (temp == "map_d")
				{
					assert(pMaterial);

					// the alpha texture map
					// mask map
					std::string strTextureName;
					file.ReadLine(strTextureName);

					if (strTextureName.empty())
						continue;

					pMaterial->SetTextureName(IMaterial::eMask, strTextureName.substr(1, strTextureName.length()).c_str());
				}
				else if (temp == "map_bump" || temp == "bump")
				{
					assert(pMaterial);

					// some implementations use 'map_bump' instead of 'bump' below
					std::string strTextureName;
					file.ReadLine(strTextureName);

					if (strTextureName.empty())
						continue;

					pMaterial->SetTextureName(IMaterial::eNormal, strTextureName.substr(1, strTextureName.length()).c_str());
				}
				else if (temp == "disp")
				{
					assert(pMaterial);

					// displacement map
					std::string strTextureName;
					file.ReadLine(strTextureName);

					if (strTextureName.empty())
						continue;
				}
				else if (temp == "decal")
				{
					assert(pMaterial);

					// stencil decal texture (defaults to 'matte' channel of the image)
				}
				else if (temp == "refl")
				{
					assert(pMaterial);

					// spherical reflection map
					// 환경 맵
					// 근데 이건 메테리얼 마다 하는게 아니라 환경 맵 셋팅 하는 객체가 따로 있음
				}
				else if (temp == "Pr")
				{
					assert(pMaterial);

					// roughness
					float fRoughness = 0.f;
					file >> fRoughness;

					pMaterial->SetRoughness(fRoughness);
				}
				else if (temp == "map_Pr")
				{
					assert(pMaterial);

					// roughness map
					std::string strTextureName;
					file.ReadLine(strTextureName);

					if (strTextureName.empty())
						continue;

					pMaterial->SetTextureName(IMaterial::eRoughness, strTextureName.substr(1, strTextureName.length()).c_str());
				}
				else if (temp == "Pm")
				{
					assert(pMaterial);

					// metallic
					float fMetallic = 0.f;
					file >> fMetallic;

					pMaterial->SetMetallic(fMetallic);
				}
				else if (temp == "map_Pm")
				{
					assert(pMaterial);

					// metallic map
					std::string strTextureName;
					file.ReadLine(strTextureName);

					if (strTextureName.empty())
						continue;

					pMaterial->SetTextureName(IMaterial::eMetallic, strTextureName.substr(1, strTextureName.length()).c_str());
				}
				else if (temp == "Ps")
				{
					assert(pMaterial);

					// sheen
					float fSheen = 0.f;
					file >> fSheen;

					pMaterial->SetSheen(fSheen);
				}
				else if (temp == "map_Ps")
				{
					assert(pMaterial);

					// sheen map
					std::string strTextureName;
					file.ReadLine(strTextureName);

					if (strTextureName.empty())
						continue;

					pMaterial->SetTextureName(IMaterial::eSheen, strTextureName.substr(1, strTextureName.length()).c_str());
				}
				else if (temp == "Pc")
				{
					assert(pMaterial);

					// clearcoat thickness
					float fClearcoatThickness = 0.f;
					file >> fClearcoatThickness;

					pMaterial->SetClearcoat(fClearcoatThickness);
				}
				else if (temp == "Pcr")
				{
					assert(pMaterial);

					// clearcoat roughness
					float fClearcoatRoughness = 0.f;
					file >> fClearcoatRoughness;
					fClearcoatRoughness = 1.f - fClearcoatRoughness;

					pMaterial->SetClearcoatGloss(fClearcoatRoughness);
				}
				else if (temp == "Ke")
				{
					assert(pMaterial);

					// emissive
					float fEmissive = 0.f;
					file >> fEmissive;

					pMaterial->SetEmissive(fEmissive);
				}
				else if (temp == "map_Ke")
				{
					assert(pMaterial);

					// emissive map
					std::string strTextureName;
					file.ReadLine(strTextureName);

					if (strTextureName.empty())
						continue;

					pMaterial->SetTextureName(IMaterial::eEmissive, strTextureName.substr(1, strTextureName.length()).c_str());
				}
				else if (temp == "aniso")
				{
					assert(pMaterial);

					// anisotropy
					float fAnisotropy = 0.f;
					file >> fAnisotropy;

					pMaterial->SetAnisotropic(fAnisotropy);
				}
				else if (temp == "anisor")
				{
					assert(pMaterial);

					// anisotropy rotation
				}
				else if (temp == "norm")
				{
					assert(pMaterial);

					// normal map, same format as "bump" parameter
				}
			}

			file.Close();

			for (auto& iter : m_umapNewMtrl)
			{
				iter.second->SetSurSpecTintAniso(math::float4(0.f, 0.f, 0.f, 0.f));
				iter.second->SetSheenTintClearcoatGloss(math::float4(0.f, 0.f, 0.f, 0.f));

				iter.second->LoadTexture();
			}

			return true;
		}

		void MtlImporter::Release()
		{
			m_umapNewMtrl.clear();
		}
	}
}