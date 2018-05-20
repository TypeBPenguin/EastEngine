#include "stdafx.h"
#include "GraphicsInterface.h"

#include "CommonLib/FileStream.h"

namespace StrID
{
	RegisterStringID(DefaultMaterial);
}

namespace eastengine
{
	namespace graphics
	{
		ImageBasedLight::~ImageBasedLight()
		{
			auto ReleaseTexture = [](ITexture** ppTexture)
			{
				if ((*ppTexture) != nullptr)
				{
					(*ppTexture)->DecreaseReference();
					*ppTexture = nullptr;
				}
			};

			ReleaseTexture(&m_pEnvHDR);
			ReleaseTexture(&m_pDiffuseHDR);
			ReleaseTexture(&m_pSpecularHDR);
			ReleaseTexture(&m_pSpecularBRDF);
		}
	}
}