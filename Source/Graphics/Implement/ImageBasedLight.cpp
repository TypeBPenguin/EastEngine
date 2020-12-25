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
		}

		void ImageBasedLight::SetTexture(TexturePtr* ppDest, const TexturePtr& pSource)
		{
			*ppDest = pSource;
		}

		void ImageBasedLight::SetVertexBuffer(VertexBufferPtr* ppDest, const VertexBufferPtr& pSource)
		{
			*ppDest = pSource;
		}

		void ImageBasedLight::SetIndexBuffer(IndexBufferPtr* ppDest, const IndexBufferPtr& pSource)
		{
			*ppDest = pSource;
		}
	}
}