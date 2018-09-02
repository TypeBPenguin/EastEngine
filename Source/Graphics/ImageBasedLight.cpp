#include "stdafx.h"
#include "ImageBasedLight.h"

#include "Graphics.h"

namespace eastengine
{
	namespace graphics
	{
		ImageBasedLight::ImageBasedLight()
		{
		}

		ImageBasedLight::~ImageBasedLight()
		{
			ReleaseResource(&m_pEnvironmentHDR);
			ReleaseResource(&m_pDiffuseHDR);
			ReleaseResource(&m_pSpecularHDR);
			ReleaseResource(&m_pSpecularBRDF);
			ReleaseResource(&m_pEnvironmentSphereVB);
			ReleaseResource(&m_pEnvironmentSphereIB);
		}

		void ImageBasedLight::SetTexture(ITexture** ppDest, ITexture* pSource)
		{
			ReleaseResource(ppDest);

			*ppDest = pSource;

			if (*ppDest != nullptr)
			{
				(*ppDest)->IncreaseReference();
			}
		}

		void ImageBasedLight::SetVertexBuffer(IVertexBuffer** ppDest, IVertexBuffer* pSource)
		{
			ReleaseResource(ppDest);

			*ppDest = pSource;

			if (*ppDest != nullptr)
			{
				(*ppDest)->IncreaseReference();
			}
		}

		void ImageBasedLight::SetIndexBuffer(IIndexBuffer** ppDest, IIndexBuffer* pSource)
		{
			ReleaseResource(ppDest);

			*ppDest = pSource;

			if (*ppDest != nullptr)
			{
				(*ppDest)->IncreaseReference();
			}
		}
	}
}