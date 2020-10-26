#include "stdafx.h"
#include "ImageBasedLight.h"

#include "Graphics.h"

namespace est
{
	namespace graphics
	{
		ImageBasedLight::ImageBasedLight()
		{
		}

		ImageBasedLight::~ImageBasedLight()
		{
			ReleaseResource(m_pEnvironmentHDR);
			ReleaseResource(m_pDiffuseHDR);
			ReleaseResource(m_pSpecularHDR);
			ReleaseResource(m_pSpecularBRDF);
			ReleaseResource(m_pEnvironmentSphereVB);
			ReleaseResource(m_pEnvironmentSphereIB);
		}

		void ImageBasedLight::SetTexture(TexturePtr* ppDest, const TexturePtr& pSource)
		{
			ReleaseResource(*ppDest);
			*ppDest = pSource;
		}

		void ImageBasedLight::SetVertexBuffer(VertexBufferPtr* ppDest, const VertexBufferPtr& pSource)
		{
			ReleaseResource(*ppDest);
			*ppDest = pSource;
		}

		void ImageBasedLight::SetIndexBuffer(IndexBufferPtr* ppDest, const IndexBufferPtr& pSource)
		{
			ReleaseResource(*ppDest);
			*ppDest = pSource;
		}
	}
}