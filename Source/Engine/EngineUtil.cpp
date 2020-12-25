#include "stdafx.h"
#include "EngineUtil.h"

#include "CommonLib/FileUtil.h"

#include "Graphics/Implement/Graphics.h"
#include "Graphics/Model/GeometryModel.h"

namespace est
{
	namespace graphics
	{
		void SetDefaultImageBaseLight()
		{
			const std::wstring iblTexturePath = string::Format(L"%sTexture\\IBL\\%s\\%s", file::GetEngineDataPath(), L"IceLake", L"IceLake");

			IImageBasedLight* pImageBasedLight = GetImageBasedLight();

			std::wstring strDiffuseHDR = iblTexturePath;
			strDiffuseHDR.append(L"DiffuseHDR.dds");
			TexturePtr pDiffuseHDR = CreateTextureAsync(strDiffuseHDR.c_str());
			pImageBasedLight->SetDiffuseHDR(pDiffuseHDR);

			std::wstring strSpecularHDR = iblTexturePath;
			strSpecularHDR.append(L"SpecularHDR.dds");
			TexturePtr pSpecularHDR = CreateTextureAsync(strSpecularHDR.c_str());
			pImageBasedLight->SetSpecularHDR(pSpecularHDR);

			std::wstring strSpecularBRDF = iblTexturePath;
			strSpecularBRDF.append(L"Brdf.dds");
			TexturePtr pSpecularBRDF = CreateTextureAsync(strSpecularBRDF.c_str());
			pImageBasedLight->SetSpecularBRDF(pSpecularBRDF);

			std::wstring strEnvIBLPath = iblTexturePath;
			strEnvIBLPath.append(L"EnvHDR.dds");
			TexturePtr pEnvironmentHDR = CreateTextureAsync(strEnvIBLPath.c_str());
			pImageBasedLight->SetEnvironmentHDR(pEnvironmentHDR);

			std::vector<VertexPosTexNor> vertices;
			std::vector<uint32_t> indices;

			geometry::CreateSphere(vertices, indices, 1.f, 32u);

			VertexBufferPtr pVertexBuffer = CreateVertexBuffer(reinterpret_cast<const uint8_t*>(vertices.data()), static_cast<uint32_t>(vertices.size()), sizeof(VertexPosTexNor), false);
			IndexBufferPtr pIndexBuffer = CreateIndexBuffer(reinterpret_cast<const uint8_t*>(indices.data()), static_cast<uint32_t>(indices.size()), sizeof(uint32_t), false);

			pImageBasedLight->SetEnvironmentSphere(pVertexBuffer, pIndexBuffer);
		}
	}
}